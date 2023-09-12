/* Copyright (c) 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2017, 2019 Nicira, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <config.h>

#include "transaction.h"

#include "bitmap.h"
#include "openvswitch/dynamic-string.h"
#include "file.h"
#include "hash.h"
#include "monitor.h"
#include "openvswitch/hmap.h"
#include "openvswitch/json.h"
#include "openvswitch/list.h"
#include "openvswitch/poll-loop.h"
#include "openvswitch/vlog.h"
#include "ovsdb-error.h"
#include "ovsdb.h"
#include "row.h"
#include "storage.h"
#include "table.h"
#include "uuid.h"

VLOG_DEFINE_THIS_MODULE(transaction);

struct ovsdb_txn {
    struct ovsdb *db;
    struct ovs_list txn_tables; /* Contains "struct ovsdb_txn_table"s. */
    struct ds comment;
    struct uuid txnid; /* For clustered mode only. It is the eid. */
};

/* A table modified by a transaction. */
struct ovsdb_txn_table {
    struct ovs_list node;       /* Element in ovsdb_txn's txn_tables list. */
    struct ovsdb_table *table;
    struct hmap txn_rows;       /* Contains "struct ovsdb_txn_row"s. */

    /* This has the same form as the 'indexes' member of struct ovsdb_table,
     * but it is only used or updated at transaction commit time, from
     * check_index_uniqueness(). */
    struct hmap *txn_indexes;

    /* Used by for_each_txn_row(). */
    unsigned int serial;        /* Serial number of in-progress iteration. */
    unsigned int n_processed;   /* Number of rows processed. */
};

/* A row modified by the transaction:
 *
 *      - A row added by a transaction will have null 'old' and non-null 'new'.
 *
 *      - A row deleted by a transaction will have non-null 'old' and null
 *        'new'.
 *
 *      - A row modified by a transaction will have non-null 'old' and 'new'.
 *
 *      - 'old' and 'new' both null indicates that a row was added then deleted
 *        within a single transaction.  Most of the time we instead delete the
 *        ovsdb_txn_row entirely, but inside a for_each_txn_row() callback
 *        there are restrictions that sometimes mean we have to leave the
 *        ovsdb_txn_row in place.
 */
struct ovsdb_txn_row {
    struct hmap_node hmap_node; /* In ovsdb_txn_table's txn_rows hmap. */
    struct ovsdb_row *old;      /* The old row. */
    struct ovsdb_row *new;      /* The new row. */
    size_t n_refs;              /* Number of remaining references. */

    /* These members are the same as the corresponding members of 'old' or
     * 'new'.  They are present here for convenience and because occasionally
     * there can be an ovsdb_txn_row where both 'old' and 'new' are NULL. */
    struct uuid uuid;
    struct ovsdb_table *table;

    /* Used by for_each_txn_row(). */
    unsigned int serial;        /* Serial number of in-progress commit. */

    unsigned long changed[];    /* Bits set to 1 for columns that changed. */
};

static struct ovsdb_error * OVS_WARN_UNUSED_RESULT
delete_garbage_row(struct ovsdb_txn *txn, struct ovsdb_txn_row *r);
static void ovsdb_txn_row_prefree(struct ovsdb_txn_row *);
static struct ovsdb_error * OVS_WARN_UNUSED_RESULT
for_each_txn_row(struct ovsdb_txn *txn,
                      struct ovsdb_error *(*)(struct ovsdb_txn *,
                                              struct ovsdb_txn_row *));

/* Used by for_each_txn_row() to track tables and rows that have been
 * processed.  */
static unsigned int serial;

struct ovsdb_txn *
ovsdb_txn_create(struct ovsdb *db)
{
    struct ovsdb_txn *txn = xzalloc(sizeof *txn);
    txn->db = db;
    ovs_list_init(&txn->txn_tables);
    ds_init(&txn->comment);
    return txn;
}

void
ovsdb_txn_set_txnid(const struct uuid *txnid, struct ovsdb_txn *txn)
{
    txn->txnid = *txnid;
}

const struct uuid *
ovsdb_txn_get_txnid(const struct ovsdb_txn *txn)
{
    return &txn->txnid;
}

static void
ovsdb_txn_free(struct ovsdb_txn *txn)
{
    ovs_assert(ovs_list_is_empty(&txn->txn_tables));
    ds_destroy(&txn->comment);
    free(txn);
}

static struct ovsdb_error *
ovsdb_txn_row_abort(struct ovsdb_txn *txn OVS_UNUSED,
                    struct ovsdb_txn_row *txn_row)
{
    struct ovsdb_row *old = txn_row->old;
    struct ovsdb_row *new = txn_row->new;

    ovsdb_txn_row_prefree(txn_row);
    if (!old) {
        if (new) {
            hmap_remove(&new->table->rows, &new->hmap_node);
        }
    } else if (!new) {
        hmap_insert(&old->table->rows, &old->hmap_node, ovsdb_row_hash(old));
    } else {
        hmap_replace(&new->table->rows, &new->hmap_node, &old->hmap_node);
    }
    ovsdb_row_destroy(new);
    free(txn_row);

    return NULL;
}

/* Returns the offset in bytes from the start of an ovsdb_row for 'table' to
 * the hmap_node for the index numbered 'i'. */
static size_t
ovsdb_row_index_offset__(const struct ovsdb_table *table, size_t i)
{
    size_t n_fields = shash_count(&table->schema->columns);
    return (offsetof(struct ovsdb_row, fields)
            + n_fields * sizeof(struct ovsdb_datum)
            + i * sizeof(struct hmap_node));
}

/* Returns the hmap_node in 'row' for the index numbered 'i'. */
static struct hmap_node *
ovsdb_row_get_index_node(struct ovsdb_row *row, size_t i)
{
    return (void *) ((char *) row + ovsdb_row_index_offset__(row->table, i));
}

/* Returns the ovsdb_row given 'index_node', which is a pointer to that row's
 * hmap_node for the index numbered 'i' within 'table'. */
static struct ovsdb_row *
ovsdb_row_from_index_node(struct hmap_node *index_node,
                          const struct ovsdb_table *table, size_t i)
{
    return (void *) ((char *) index_node - ovsdb_row_index_offset__(table, i));
}

void
ovsdb_txn_abort(struct ovsdb_txn *txn)
{
    ovsdb_error_assert(for_each_txn_row(txn, ovsdb_txn_row_abort));
    ovsdb_txn_free(txn);
}

static struct ovsdb_txn_row *
find_txn_row(const struct ovsdb_table *table, const struct uuid *uuid)
{
    struct ovsdb_txn_row *txn_row;

    if (!table->txn_table) {
        return NULL;
    }

    HMAP_FOR_EACH_WITH_HASH (txn_row, hmap_node,
                             uuid_hash(uuid), &table->txn_table->txn_rows) {
        if (uuid_equals(uuid, &txn_row->uuid)) {
            return txn_row;
        }
    }

    return NULL;
}

static struct ovsdb_txn_row *
find_or_make_txn_row(struct ovsdb_txn *txn, const struct ovsdb_table *table,
                     const struct uuid *uuid)
{
    struct ovsdb_txn_row *txn_row = find_txn_row(table, uuid);
    if (!txn_row) {
        const struct ovsdb_row *row = ovsdb_table_get_row(table, uuid);
        if (row) {
            txn_row = ovsdb_txn_row_modify(txn, row)->txn_row;
        }
    }
    return txn_row;
}

static struct ovsdb_error * OVS_WARN_UNUSED_RESULT
ovsdb_txn_adjust_atom_refs(struct ovsdb_txn *txn, const struct ovsdb_row *r,
                           const struct ovsdb_column *c,
                           const struct ovsdb_base_type *base,
                           const union ovsdb_atom *atoms, unsigned int n,
                           int delta)
{
    const struct ovsdb_table *table;
    unsigned int i;

    if (!ovsdb_base_type_is_strong_ref(base)) {
        return NULL;
    }

    table = base->uuid.refTable;
    for (i = 0; i < n; i++) {
        const struct uuid *uuid = &atoms[i].uuid;
        struct ovsdb_txn_row *txn_row;

        if (uuid_equals(uuid, ovsdb_row_get_uuid(r))) {
            /* Self-references don't count. */
            continue;
        }

        txn_row = find_or_make_txn_row(txn, table, uuid);
        if (!txn_row) {
            return ovsdb_error("referential integrity violation",
                               "Table %s column %s row "UUID_FMT" "
                               "references nonexistent row "UUID_FMT" in "
                               "table %s.",
                               r->table->schema->name, c->name,
                               UUID_ARGS(ovsdb_row_get_uuid(r)),
                               UUID_ARGS(uuid), table->schema->name);
        }
        txn_row->n_refs += delta;
    }

    return NULL;
}

static struct ovsdb_error * OVS_WARN_UNUSED_RESULT
ovsdb_txn_adjust_row_refs(struct ovsdb_txn *txn, const struct ovsdb_row *r,
                          const struct ovsdb_column *column, int delta)
{
    const struct ovsdb_datum *field = &r->fields[column->index];
    struct ovsdb_error *error;

    error = ovsdb_txn_adjust_atom_refs(txn, r, column, &column->type.key,
                                       field->keys, field->n, delta);
    if (!error) {
        error = ovsdb_txn_adjust_atom_refs(txn, r, column, &column->type.value,
                                           field->values, field->n, delta);
    }
    return error;
}

static struct ovsdb_error * OVS_WARN_UNUSED_RESULT
update_row_ref_count(struct ovsdb_txn *txn, struct ovsdb_txn_row *r)
{
    struct ovsdb_table *table = r->table;
    struct shash_node *node;

    SHASH_FOR_EACH (node, &table->schema->columns) {
        const struct ovsdb_column *column = node->data;
        struct ovsdb_error *error;

        if (bitmap_is_set(r->changed, column->index)) {
            if (r->old) {
                error = ovsdb_txn_adjust_row_refs(txn, r->old, column, -1);
                if (error) {
                    return OVSDB_WRAP_BUG("error decreasing refcount", error);
                }
            }
            if (r->new) {
                error = ovsdb_txn_adjust_row_refs(txn, r->new, column, 1);
                if (error) {
                    return error;
                }
            }
        }
    }

    return NULL;
}

static struct ovsdb_error * OVS_WARN_UNUSED_RESULT
check_ref_count(struct ovsdb_txn *txn OVS_UNUSED, struct ovsdb_txn_row *r)
{
    if (r->new || !r->n_refs) {
        return NULL;
    } else {
        return ovsdb_error("referential integrity violation",
                           "cannot delete %s row "UUID_FMT" because "
                           "of %"PRIuSIZE" remaining reference(s)",
                           r->table->schema->name, UUID_ARGS(&r->uuid),
                           r->n_refs);
    }
}

static struct ovsdb_error * OVS_WARN_UNUSED_RESULT
delete_row_refs(struct ovsdb_txn *txn, const struct ovsdb_row *row,
                const struct ovsdb_base_type *base,
                const union ovsdb_atom *atoms, unsigned int n)
{
    const struct ovsdb_table *table;
    unsigned int i;

    if (!ovsdb_base_type_is_strong_ref(base)) {
        return NULL;
    }

    table = base->uuid.refTable;
    for (i = 0; i < n; i++) {
        const struct uuid *uuid = &atoms[i].uuid;
        struct ovsdb_txn_row *txn_row;

        if (uuid_equals(uuid, ovsdb_row_get_uuid(row))) {
            /* Self-references don't count. */
            continue;
        }

        txn_row = find_or_make_txn_row(txn, table, uuid);
        if (!txn_row) {
            return OVSDB_BUG("strong ref target missing");
        } else if (!txn_row->n_refs) {
            return OVSDB_BUG("strong ref target has zero n_refs");
        } else if (!txn_row->new) {
            return OVSDB_BUG("deleted strong ref target");
        }

        if (--txn_row->n_refs == 0) {
            struct ovsdb_error *error = delete_garbage_row(txn, txn_row);
            if (error) {
                return error;
            }
        }
    }

    return NULL;
}

static struct ovsdb_error * OVS_WARN_UNUSED_RESULT
delete_garbage_row(struct ovsdb_txn *txn, struct ovsdb_txn_row *txn_row)
{
    struct shash_node *node;
    struct ovsdb_row *row;

    if (txn_row->table->schema->is_root) {
        return NULL;
    }

    row = txn_row->new;
    txn_row->new = NULL;
    hmap_remove(&txn_row->table->rows, &row->hmap_node);
    SHASH_FOR_EACH (node, &txn_row->table->schema->columns) {
        const struct ovsdb_column *column = node->data;
        const struct ovsdb_datum *field = &row->fields[column->index];
        struct ovsdb_error *error;

        error = delete_row_refs(txn, row,
                                &column->type.key, field->keys, field->n);
        if (error) {
            return error;
        }

        error = delete_row_refs(txn, row,
                                &column->type.value, field->values, field->n);
        if (error) {
            return error;
        }
    }
    ovsdb_row_destroy(row);

    return NULL;
}

static struct ovsdb_error * OVS_WARN_UNUSED_RESULT
collect_garbage(struct ovsdb_txn *txn, struct ovsdb_txn_row *txn_row)
{
    if (txn_row->new && !txn_row->n_refs) {
        return delete_garbage_row(txn, txn_row);
    }
    return NULL;
}

static struct ovsdb_error * OVS_WARN_UNUSED_RESULT
update_ref_counts(struct ovsdb_txn *txn)
{
    struct ovsdb_error *error;

    error = for_each_txn_row(txn, update_row_ref_count);
    if (error) {
        return error;
    }

    return for_each_txn_row(txn, check_ref_count);
}

static struct ovsdb_error *
ovsdb_txn_row_commit(struct ovsdb_txn *txn OVS_UNUSED,
                     struct ovsdb_txn_row *txn_row)
{
    size_t n_indexes = txn_row->table->schema->n_indexes;

    if (txn_row->old) {
        size_t i;

        for (i = 0; i < n_indexes; i++) {
            struct hmap_node *node = ovsdb_row_get_index_node(txn_row->old, i);
            hmap_remove(&txn_row->table->indexes[i], node);
        }
    }
    if (txn_row->new) {
        size_t i;

        for (i = 0; i < n_indexes; i++) {
            struct hmap_node *node = ovsdb_row_get_index_node(txn_row->new, i);
            hmap_insert(&txn_row->table->indexes[i], node, node->hash);
        }
    }

    ovsdb_txn_row_prefree(txn_row);
    if (txn_row->new) {
        txn_row->new->n_refs = txn_row->n_refs;
    }
    ovsdb_row_destroy(txn_row->old);
    free(txn_row);

    return NULL;
}

static struct ovsdb_error *
ovsdb_txn_update_weak_refs(struct ovsdb_txn *txn OVS_UNUSED,
                           struct ovsdb_txn_row *txn_row)
{
    struct ovsdb_weak_ref *weak, *next;

    /* Remove the weak references originating in the old version of the row. */
    if (txn_row->old) {
        LIST_FOR_EACH_SAFE (weak, next, src_node, &txn_row->old->src_refs) {
            ovs_list_remove(&weak->src_node);
            ovs_list_remove(&weak->dst_node);
            free(weak);
        }
    }

    /* Although the originating rows have the responsibility of updating the
     * weak references in the dst, it is possible that some source rows aren't
     * part of the transaction.  In that situation this row needs to move the
     * list of incoming weak references from the old row into the new one.
     */
    if (txn_row->old && txn_row->new) {
        /* Move the incoming weak references from old to new. */
        ovs_list_push_back_all(&txn_row->new->dst_refs,
                               &txn_row->old->dst_refs);
    }

    /* Insert the weak references originating in the new version of the row. */
    struct ovsdb_row *dst_row;
    if (txn_row->new) {
        LIST_FOR_EACH (weak, src_node, &txn_row->new->src_refs) {
            /* dst_row MUST exist. */
            dst_row = CONST_CAST(struct ovsdb_row *,
                    ovsdb_table_get_row(weak->dst_table, &weak->dst));
            ovs_list_insert(&dst_row->dst_refs, &weak->dst_node);
        }
    }

    return NULL;
}

static void
add_weak_ref(const struct ovsdb_row *src_, const struct ovsdb_row *dst_)
{
    struct ovsdb_row *src = CONST_CAST(struct ovsdb_row *, src_);
    struct ovsdb_row *dst = CONST_CAST(struct ovsdb_row *, dst_);
    struct ovsdb_weak_ref *weak;

    if (src == dst) {
        return;
    }

    if (!ovs_list_is_empty(&dst->dst_refs)) {
        /* Omit duplicates. */
        weak = CONTAINER_OF(ovs_list_back(&dst->dst_refs),
                            struct ovsdb_weak_ref, dst_node);
        if (weak->src == src) {
            return;
        }
    }

    weak = xmalloc(sizeof *weak);
    weak->src = src;
    weak->dst_table = dst->table;
    weak->dst = *ovsdb_row_get_uuid(dst);
    /* The dst_refs list is updated at commit time. */
    ovs_list_init(&weak->dst_node);
    ovs_list_push_back(&src->src_refs, &weak->src_node);
}

static struct ovsdb_error * OVS_WARN_UNUSED_RESULT
assess_weak_refs(struct ovsdb_txn *txn, struct ovsdb_txn_row *txn_row)
{
    struct ovsdb_table *table;
    struct shash_node *node;

    if (txn_row->old && !txn_row->new) {
        /* Mark rows that have weak references to 'txn_row' as modified, so
         * that their weak references will get reassessed. */
        struct ovsdb_weak_ref *weak, *next;

        LIST_FOR_EACH_SAFE (weak, next, dst_node, &txn_row->old->dst_refs) {
            if (!weak->src->txn_row) {
                ovsdb_txn_row_modify(txn, weak->src);
            }
        }
    }

    if (!txn_row->new) {
        /* We don't have to do anything about references that originate at
         * 'txn_row', because ovsdb_row_destroy() will remove those weak
         * references. */
        return NULL;
    }

    table = txn_row->table;
    SHASH_FOR_EACH (node, &table->schema->columns) {
        const struct ovsdb_column *column = node->data;
        struct ovsdb_datum *datum = &txn_row->new->fields[column->index];
        unsigned int orig_n, i;
        bool zero = false;

        orig_n = datum->n;

        if (ovsdb_base_type_is_weak_ref(&column->type.key)) {
            for (i = 0; i < datum->n; ) {
                const struct ovsdb_row *row;

                row = ovsdb_table_get_row(column->type.key.uuid.refTable,
                                          &datum->keys[i].uuid);
                if (row) {
                    add_weak_ref(txn_row->new, row);
                    i++;
                } else {
                    if (uuid_is_zero(&datum->keys[i].uuid)) {
                        zero = true;
                    }
                    ovsdb_datum_remove_unsafe(datum, i, &column->type);
                }
            }
        }

        if (ovsdb_base_type_is_weak_ref(&column->type.value)) {
            for (i = 0; i < datum->n; ) {
                const struct ovsdb_row *row;

                row = ovsdb_table_get_row(column->type.value.uuid.refTable,
                                          &datum->values[i].uuid);
                if (row) {
                    add_weak_ref(txn_row->new, row);
                    i++;
                } else {
                    if (uuid_is_zero(&datum->values[i].uuid)) {
                        zero = true;
                    }
                    ovsdb_datum_remove_unsafe(datum, i, &column->type);
                }
            }
        }

        if (datum->n != orig_n) {
            bitmap_set1(txn_row->changed, column->index);
            ovsdb_datum_sort_assert(datum, column->type.key.type);
            if (datum->n < column->type.n_min) {
                const struct uuid *row_uuid = ovsdb_row_get_uuid(txn_row->new);
                if (zero && !txn_row->old) {
                    return ovsdb_error(
                        "constraint violation",
                        "Weak reference column \"%s\" in \"%s\" row "UUID_FMT
                        " (inserted within this transaction) contained "
                        "all-zeros UUID (probably as the default value for "
                        "this column) but deleting this value caused a "
                        "constraint volation because this column is not "
                        "allowed to be empty.", column->name,
                        table->schema->name, UUID_ARGS(row_uuid));
                } else {
                    return ovsdb_error(
                        "constraint violation",
                        "Deletion of %u weak reference(s) to deleted (or "
                        "never-existing) rows from column \"%s\" in \"%s\" "
                        "row "UUID_FMT" %scaused this column to become empty, "
                        "but constraints on this column disallow an "
                        "empty column.",
                        orig_n - datum->n, column->name, table->schema->name,
                        UUID_ARGS(row_uuid),
                        (txn_row->old
                         ? ""
                         : "(inserted within this transaction) "));
                }
            }
        }
    }

    return NULL;
}

static struct ovsdb_error * OVS_WARN_UNUSED_RESULT
determine_changes(struct ovsdb_txn *txn, struct ovsdb_txn_row *txn_row)
{
    struct ovsdb_table *table = txn_row->table;

    if (txn_row->old && txn_row->new) {
        struct shash_node *node;
        bool changed = false;

        SHASH_FOR_EACH (node, &table->schema->columns) {
            const struct ovsdb_column *column = node->data;
            const struct ovsdb_type *type = &column->type;
            unsigned int idx = column->index;

            if (!ovsdb_datum_equals(&txn_row->old->fields[idx],
                                    &txn_row->new->fields[idx],
                                    type)) {
                bitmap_set1(txn_row->changed, idx);
                changed = true;
            }
        }

        if (!changed) {
            /* Nothing actually changed in this row, so drop it. */
            ovsdb_txn_row_abort(txn, txn_row);
        }
    } else {
        bitmap_set_multiple(txn_row->changed, 0,
                            shash_count(&table->schema->columns), 1);
    }

    return NULL;
}

static struct ovsdb_error * OVS_WARN_UNUSED_RESULT
check_max_rows(struct ovsdb_txn *txn)
{
    struct ovsdb_txn_table *t;

    LIST_FOR_EACH (t, node, &txn->txn_tables) {
        size_t n_rows = hmap_count(&t->table->rows);
        unsigned int max_rows = t->table->schema->max_rows;

        if (n_rows > max_rows) {
            return ovsdb_error("constraint violation",
                               "transaction causes \"%s\" table to contain "
                               "%"PRIuSIZE" rows, greater than the schema-defined "
                               "limit of %u row(s)",
                               t->table->schema->name, n_rows, max_rows);
        }
    }

    return NULL;
}

static struct ovsdb_row *
ovsdb_index_search(struct hmap *index, struct ovsdb_row *row, size_t i,
                   uint32_t hash)
{
    const struct ovsdb_table *table = row->table;
    const struct ovsdb_column_set *columns = &table->schema->indexes[i];
    struct hmap_node *node;

    for (node = hmap_first_with_hash(index, hash); node;
         node = hmap_next_with_hash(node)) {
        struct ovsdb_row *irow = ovsdb_row_from_index_node(node, table, i);
        if (ovsdb_row_equal_columns(row, irow, columns)) {
            return irow;
        }
    }

    return NULL;
}

static void
duplicate_index_row__(const struct ovsdb_column_set *index,
                      const struct ovsdb_row *row,
                      const char *title,
                      struct ds *out)
{
    size_t n_columns = shash_count(&row->table->schema->columns);

    ds_put_format(out, "%s row, with UUID "UUID_FMT", ",
                  title, UUID_ARGS(ovsdb_row_get_uuid(row)));
    if (!row->txn_row
        || bitmap_scan(row->txn_row->changed, 1, 0, n_columns) == n_columns) {
        ds_put_cstr(out, "existed in the database before this "
                    "transaction and was not modified by the transaction.");
    } else if (!row->txn_row->old) {
        ds_put_cstr(out, "was inserted by this transaction.");
    } else if (ovsdb_row_equal_columns(row->txn_row->old,
                                       row->txn_row->new, index)) {
        ds_put_cstr(out, "existed in the database before this "
                    "transaction, which modified some of the row's columns "
                    "but not any columns in this index.");
    } else {
        ds_put_cstr(out, "had the following index values before the "
                    "transaction: ");
        ovsdb_row_columns_to_string(row->txn_row->old, index, out);
        ds_put_char(out, '.');
    }
}

static struct ovsdb_error * OVS_WARN_UNUSED_RESULT
duplicate_index_row(const struct ovsdb_column_set *index,
                    const struct ovsdb_row *a,
                    const struct ovsdb_row *b)
{
    struct ovsdb_column_set all_columns;
    struct ovsdb_error *error;
    char *index_s;
    struct ds s;

    /* Put 'a' and 'b' in a predictable order to make error messages
     * reproducible for testing. */
    ovsdb_column_set_init(&all_columns);
    ovsdb_column_set_add_all(&all_columns, a->table);
    if (ovsdb_row_compare_columns_3way(a, b, &all_columns) < 0) {
        const struct ovsdb_row *tmp = a;
        a = b;
        b = tmp;
    }
    ovsdb_column_set_destroy(&all_columns);

    index_s = ovsdb_column_set_to_string(index);

    ds_init(&s);
    ds_put_format(&s, "Transaction causes multiple rows in \"%s\" table to "
                  "have identical values (", a->table->schema->name);
    ovsdb_row_columns_to_string(a, index, &s);
    ds_put_format(&s, ") for index on %s.  ", index_s);
    duplicate_index_row__(index, a, "First", &s);
    ds_put_cstr(&s, "  ");
    duplicate_index_row__(index, b, "Second", &s);

    free(index_s);

    error = ovsdb_error("constraint violation", "%s", ds_cstr(&s));
    ds_destroy(&s);
    return error;
}

static struct ovsdb_error * OVS_WARN_UNUSED_RESULT
check_index_uniqueness(struct ovsdb_txn *txn OVS_UNUSED,
                       struct ovsdb_txn_row *txn_row)
{
    /* Skip rows that are being deleted since they can't violate uniqueness. */
    struct ovsdb_row *row = txn_row->new;
    if (!row) {
        return NULL;
    }

    struct ovsdb_txn_table *txn_table = txn_row->table->txn_table;
    struct ovsdb_table *table = txn_row->table;
    for (size_t i = 0; i < table->schema->n_indexes; i++) {
        const struct ovsdb_column_set *index = &table->schema->indexes[i];
        uint32_t hash = ovsdb_row_hash_columns(row, index, 0);

        /* Check whether the row has a match in the temporary hash table that
         * we're building.  If we add two rows with the same index data, then
         * there's a duplicate within the rows added or modified in this
         * transaction.*/
        struct ovsdb_row *irow
            = ovsdb_index_search(&txn_table->txn_indexes[i], row, i, hash);
        if (irow) {
            return duplicate_index_row(index, irow, row);
        }

        /* Also check whether the row has a match in the table's real index
         * (which won't be updated until transaction commit is certain).  If
         * there's a match, and it's for a row that wasn't pulled into the
         * transaction, then it's a duplicate.  (If it is for a row that is
         * part of the transaction, then the first check has already handled
         * it.) */
        irow = ovsdb_index_search(&table->indexes[i], row, i, hash);
        if (irow && !irow->txn_row) {
            return duplicate_index_row(index, irow, row);
        }

        /* Add row to temporary hash table. */
        hmap_insert(&txn_table->txn_indexes[i],
                    ovsdb_row_get_index_node(row, i), hash);
    }

    return NULL;
}

static struct ovsdb_error * OVS_WARN_UNUSED_RESULT
update_version(struct ovsdb_txn *txn OVS_UNUSED, struct ovsdb_txn_row *txn_row)
{
    struct ovsdb_table *table = txn_row->table;
    size_t n_columns = shash_count(&table->schema->columns);

    if (txn_row->old && txn_row->new
        && !bitmap_is_all_zeros(txn_row->changed, n_columns)) {
        bitmap_set1(txn_row->changed, OVSDB_COL_VERSION);
        uuid_generate(ovsdb_row_get_version_rw(txn_row->new));
    }

    return NULL;
}

static bool
ovsdb_txn_is_empty(const struct ovsdb_txn *txn)
{
    return ovs_list_is_empty(&txn->txn_tables);
}

static struct ovsdb_error * OVS_WARN_UNUSED_RESULT
ovsdb_txn_precommit(struct ovsdb_txn *txn)
{
    struct ovsdb_error *error;

    /* Figure out what actually changed, and abort early if the transaction
     * was really a no-op. */
    error = for_each_txn_row(txn, determine_changes);
    if (error) {
        ovsdb_txn_abort(txn);
        return OVSDB_WRAP_BUG("can't happen", error);
    }
    if (ovs_list_is_empty(&txn->txn_tables)) {
        return NULL;
    }

    /* Update reference counts and check referential integrity. */
    error = update_ref_counts(txn);
    if (error) {
        return error;
    }

    /* Delete unreferenced, non-root rows. */
    error = for_each_txn_row(txn, collect_garbage);
    if (error) {
        return OVSDB_WRAP_BUG("can't happen", error);
    }

    /* Check maximum rows table constraints. */
    error = check_max_rows(txn);
    if (error) {
        return error;
    }

    /* Check reference counts and remove bad references for "weak" referential
     * integrity. */
    error = for_each_txn_row(txn, assess_weak_refs);
    if (error) {
        return error;
    }

    /* Verify that the indexes will still be unique post-transaction. */
    error = for_each_txn_row(txn, check_index_uniqueness);
    if (error) {
        return error;
    }

    /* Update _version for rows that changed.  */
    error = for_each_txn_row(txn, update_version);
    if (error) {
        return OVSDB_WRAP_BUG("can't happen", error);
    }

    return error;
}

static struct ovsdb_txn*
ovsdb_txn_clone(const struct ovsdb_txn *txn)
{
    struct ovsdb_txn *txn_cloned = xzalloc(sizeof *txn_cloned);
    ovs_list_init(&txn_cloned->txn_tables);
    txn_cloned->txnid = txn->txnid;

    struct ovsdb_txn_table *t;
    LIST_FOR_EACH (t, node, &txn->txn_tables) {
        struct ovsdb_txn_table *t_cloned = xmalloc(sizeof *t_cloned);
        ovs_list_push_back(&txn_cloned->txn_tables, &t_cloned->node);
        hmap_init(&t_cloned->txn_rows);

        struct ovsdb_txn_row *r;
        HMAP_FOR_EACH (r, hmap_node, &t->txn_rows) {
            size_t n_columns = shash_count(&t->table->schema->columns);
            struct ovsdb_txn_row *r_cloned =
                xzalloc(offsetof(struct ovsdb_txn_row, changed)
                        + bitmap_n_bytes(n_columns));

            r_cloned->uuid = r->uuid;
            r_cloned->table = r->table;
            r_cloned->old = r->old ? ovsdb_row_clone(r->old) : NULL;
            r_cloned->new = r->new ? ovsdb_row_clone(r->new) : NULL;
            memcpy(&r_cloned->changed, &r->changed, bitmap_n_bytes(n_columns));
            hmap_insert(&t_cloned->txn_rows, &r_cloned->hmap_node,
                        uuid_hash(&r_cloned->uuid));
        }
    }
    return txn_cloned;
}

static void
ovsdb_txn_destroy_cloned(struct ovsdb_txn *txn)
{
    ovs_assert(!txn->db);
    struct ovsdb_txn_table *t, *next_txn_table;
    LIST_FOR_EACH_SAFE (t, next_txn_table, node, &txn->txn_tables) {
        struct ovsdb_txn_row *r, *next_txn_row;
        HMAP_FOR_EACH_SAFE (r, next_txn_row, hmap_node, &t->txn_rows) {
            if (r->old) {
                ovsdb_row_destroy(r->old);
            }
            if (r->new) {
                ovsdb_row_destroy(r->new);
            }
            hmap_remove(&t->txn_rows, &r->hmap_node);
            free(r);
        }
        hmap_destroy(&t->txn_rows);
        ovs_list_remove(&t->node);
        free(t);
    }
    free(txn);
}

static void
ovsdb_txn_add_to_history(struct ovsdb_txn *txn)
{
    if (txn->db->need_txn_history) {
        struct ovsdb_txn_history_node *node = xzalloc(sizeof *node);
        node->txn = ovsdb_txn_clone(txn);
        ovs_list_push_back(&txn->db->txn_history, &node->node);
        txn->db->n_txn_history++;
    }
}

/* Finalize commit. */
void
ovsdb_txn_complete(struct ovsdb_txn *txn)
{
    if (!ovsdb_txn_is_empty(txn)) {

        txn->db->run_triggers_now = txn->db->run_triggers = true;
        ovsdb_monitors_commit(txn->db, txn);
        ovsdb_error_assert(for_each_txn_row(txn, ovsdb_txn_update_weak_refs));
        ovsdb_error_assert(for_each_txn_row(txn, ovsdb_txn_row_commit));
    }
    ovsdb_txn_free(txn);
}

/* Applies 'txn' to the internal representation of the database.  This is for
 * transactions that don't need to be written to storage; probably, they came
 * from storage.  These transactions shouldn't ordinarily fail because storage
 * should contain only consistent transactions.  (One exception is for database
 * conversion in ovsdb_convert().) */
struct ovsdb_error * OVS_WARN_UNUSED_RESULT
ovsdb_txn_replay_commit(struct ovsdb_txn *txn)
{
    struct ovsdb_error *error = ovsdb_txn_precommit(txn);
    if (error) {
        ovsdb_txn_abort(txn);
    } else {
        ovsdb_txn_add_to_history(txn);
        ovsdb_txn_complete(txn);
    }
    return error;
}

/* If 'error' is nonnull, the transaction is complete, with the given error as
 * the result.
 *
 * Otherwise, if 'write' is nonnull, then the transaction is waiting for
 * 'write' to complete.
 *
 * Otherwise, if 'commit_index' is nonzero, then the transaction is waiting for
 * 'commit_index' to be applied to the storage.
 *
 * Otherwise, the transaction is complete and successful. */
struct ovsdb_txn_progress {
    struct ovsdb_error *error;
    struct ovsdb_write *write;
    uint64_t commit_index;

    struct ovsdb_storage *storage;
};

bool
ovsdb_txn_precheck_prereq(const struct ovsdb *db)
{
    const struct uuid *eid = ovsdb_storage_peek_last_eid(db->storage);
    if (!eid) {
        return true;
    }
    return uuid_equals(&db->prereq, eid);
}

struct ovsdb_txn_progress *
ovsdb_txn_propose_schema_change(struct ovsdb *db,
                                const struct json *schema,
                                const struct json *data)
{
    struct ovsdb_txn_progress *progress = xzalloc(sizeof *progress);
    progress->storage = db->storage;

    struct uuid next;
    struct ovsdb_write *write = ovsdb_storage_write_schema_change(
        db->storage, schema, data, &db->prereq, &next);
    if (!ovsdb_write_is_complete(write)) {
        progress->write = write;
    } else {
        progress->error = ovsdb_error_clone(ovsdb_write_get_error(write));
        ovsdb_write_destroy(write);
    }
    return progress;
}

struct ovsdb_txn_progress *
ovsdb_txn_propose_commit(struct ovsdb_txn *txn, bool durable)
{
    struct ovsdb_txn_progress *progress = xzalloc(sizeof *progress);
    progress->storage = txn->db->storage;
    progress->error = ovsdb_txn_precommit(txn);
    if (progress->error) {
        return progress;
    }

    /* Turn the commit into the format used for the storage logs.. */
    struct json *txn_json = ovsdb_file_txn_to_json(txn);
    if (!txn_json) {
        /* Nothing to do, so success. */
        return progress;
    }
    txn_json = ovsdb_file_txn_annotate(txn_json, ovsdb_txn_get_comment(txn));

    struct uuid next;
    struct ovsdb_write *write = ovsdb_storage_write(
        txn->db->storage, txn_json, &txn->db->prereq, &next, durable);
    json_destroy(txn_json);
    if (!ovsdb_write_is_complete(write)) {
        progress->write = write;
    } else {
        progress->error = ovsdb_error_clone(ovsdb_write_get_error(write));
        ovsdb_write_destroy(write);
    }
    return progress;
}

/* Proposes 'txn' for commitment and then waits for the commit to succeed or
 * fail.  Returns null if successful, otherwise the error.
 *
 * **In addition**, this function also completes or aborts the transaction if
 * the transaction succeeded or failed, respectively. */
struct ovsdb_error * OVS_WARN_UNUSED_RESULT
ovsdb_txn_propose_commit_block(struct ovsdb_txn *txn, bool durable)
{
    struct ovsdb_txn_progress *p = ovsdb_txn_propose_commit(txn, durable);
    for (;;) {
        ovsdb_storage_run(p->storage);
        if (ovsdb_txn_progress_is_complete(p)) {
            struct ovsdb_error *error
                = ovsdb_error_clone(ovsdb_txn_progress_get_error(p));
            ovsdb_txn_progress_destroy(p);

            if (error) {
                ovsdb_txn_abort(txn);
            } else {
                ovsdb_txn_complete(txn);
            }

            return error;
        }
        ovsdb_storage_wait(p->storage);
        poll_block();
    }
}

static void
ovsdb_txn_progress_run(struct ovsdb_txn_progress *p)
{
    if (p->error) {
        return;
    }

    if (p->write) {
        if (!ovsdb_write_is_complete(p->write)) {
            return;
        }
        p->error = ovsdb_error_clone(ovsdb_write_get_error(p->write));
        p->commit_index = ovsdb_write_get_commit_index(p->write);
        ovsdb_write_destroy(p->write);
        p->write = NULL;

        if (p->error) {
            return;
        }
    }

    if (p->commit_index) {
        if (ovsdb_storage_get_applied_index(p->storage) >= p->commit_index) {
            p->commit_index = 0;
        }
    }
}

static bool
ovsdb_txn_progress_is_complete__(const struct ovsdb_txn_progress *p)
{
    return p->error || (!p->write && !p->commit_index);
}

bool
ovsdb_txn_progress_is_complete(const struct ovsdb_txn_progress *p)
{
    ovsdb_txn_progress_run(CONST_CAST(struct ovsdb_txn_progress *, p));
    return ovsdb_txn_progress_is_complete__(p);
}

const struct ovsdb_error *
ovsdb_txn_progress_get_error(const struct ovsdb_txn_progress *p)
{
    ovs_assert(ovsdb_txn_progress_is_complete__(p));
    return p->error;
}

void
ovsdb_txn_progress_destroy(struct ovsdb_txn_progress *p)
{
    if (p) {
        ovsdb_error_destroy(p->error);
        ovsdb_write_destroy(p->write);
        free(p);
    }
}

void
ovsdb_txn_for_each_change(const struct ovsdb_txn *txn,
                          ovsdb_txn_row_cb_func *cb, void *aux)
{
    struct ovsdb_txn_table *t;
    struct ovsdb_txn_row *r;

    LIST_FOR_EACH (t, node, &txn->txn_tables) {
        HMAP_FOR_EACH (r, hmap_node, &t->txn_rows) {
            if ((r->old || r->new) && !cb(r->old, r->new, r->changed, aux)) {
                break;
            }
        }
   }
}

static struct ovsdb_txn_table *
ovsdb_txn_create_txn_table(struct ovsdb_txn *txn, struct ovsdb_table *table)
{
    if (!table->txn_table) {
        struct ovsdb_txn_table *txn_table;
        size_t i;

        table->txn_table = txn_table = xmalloc(sizeof *table->txn_table);
        txn_table->table = table;
        hmap_init(&txn_table->txn_rows);
        txn_table->serial = serial - 1;
        txn_table->txn_indexes = xmalloc(table->schema->n_indexes
                                         * sizeof *txn_table->txn_indexes);
        for (i = 0; i < table->schema->n_indexes; i++) {
            hmap_init(&txn_table->txn_indexes[i]);
        }
        ovs_list_push_back(&txn->txn_tables, &txn_table->node);
    }
    return table->txn_table;
}

static struct ovsdb_txn_row *
ovsdb_txn_row_create(struct ovsdb_txn *txn, struct ovsdb_table *table,
                     const struct ovsdb_row *old_, struct ovsdb_row *new)
{
    const struct ovsdb_row *row = old_ ? old_ : new;
    struct ovsdb_row *old = CONST_CAST(struct ovsdb_row *, old_);
    size_t n_columns = shash_count(&table->schema->columns);
    struct ovsdb_txn_table *txn_table;
    struct ovsdb_txn_row *txn_row;

    txn_row = xzalloc(offsetof(struct ovsdb_txn_row, changed)
                      + bitmap_n_bytes(n_columns));
    txn_row->uuid = *ovsdb_row_get_uuid(row);
    txn_row->table = row->table;
    txn_row->old = old;
    txn_row->new = new;
    txn_row->n_refs = old ? old->n_refs : 0;
    txn_row->serial = serial - 1;

    if (old) {
        old->txn_row = txn_row;
    }
    if (new) {
        new->txn_row = txn_row;
    }

    txn_table = ovsdb_txn_create_txn_table(txn, table);
    hmap_insert(&txn_table->txn_rows, &txn_row->hmap_node,
                ovsdb_row_hash(old ? old : new));

    return txn_row;
}

struct ovsdb_row *
ovsdb_txn_row_modify(struct ovsdb_txn *txn, const struct ovsdb_row *ro_row_)
{
    struct ovsdb_row *ro_row = CONST_CAST(struct ovsdb_row *, ro_row_);

    if (ro_row->txn_row) {
        ovs_assert(ro_row == ro_row->txn_row->new);
        return ro_row;
    } else {
        struct ovsdb_table *table = ro_row->table;
        struct ovsdb_row *rw_row;

        rw_row = ovsdb_row_clone(ro_row);
        rw_row->n_refs = ro_row->n_refs;
        ovsdb_txn_row_create(txn, table, ro_row, rw_row);
        hmap_replace(&table->rows, &ro_row->hmap_node, &rw_row->hmap_node);

        return rw_row;
    }
}

void
ovsdb_txn_row_insert(struct ovsdb_txn *txn, struct ovsdb_row *row)
{
    uint32_t hash = ovsdb_row_hash(row);
    struct ovsdb_table *table = row->table;

    uuid_generate(ovsdb_row_get_version_rw(row));

    ovsdb_txn_row_create(txn, table, NULL, row);
    hmap_insert(&table->rows, &row->hmap_node, hash);
}

/* 'row' must be assumed destroyed upon return; the caller must not reference
 * it again. */
void
ovsdb_txn_row_delete(struct ovsdb_txn *txn, const struct ovsdb_row *row_)
{
    struct ovsdb_row *row = CONST_CAST(struct ovsdb_row *, row_);
    struct ovsdb_table *table = row->table;
    struct ovsdb_txn_row *txn_row = row->txn_row;

    hmap_remove(&table->rows, &row->hmap_node);

    if (!txn_row) {
        ovsdb_txn_row_create(txn, table, row, NULL);
    } else {
        ovs_assert(txn_row->new == row);
        if (txn_row->old) {
            txn_row->new = NULL;
        } else {
            hmap_remove(&table->txn_table->txn_rows, &txn_row->hmap_node);
            free(txn_row);
        }
        ovsdb_row_destroy(row);
    }
}

/* Returns true if 'row_uuid' may be used as the UUID for a newly created row
 * in 'table' (that is, that it is unique within 'table'), false otherwise. */
bool
ovsdb_txn_may_create_row(const struct ovsdb_table *table,
                         const struct uuid *row_uuid)
{
    /* If a row 'row_uuid' currently exists, disallow creating a duplicate. */
    if (ovsdb_table_get_row(table, row_uuid)) {
        return false;
    }

    /* If a row 'row_uuid' previously existed in this transaction, disallow
     * creating a new row with the same UUID. */
    if (find_txn_row(table, row_uuid)) {
        return false;
    }

    return true;
}

void
ovsdb_txn_add_comment(struct ovsdb_txn *txn, const char *s)
{
    if (txn->comment.length) {
        ds_put_char(&txn->comment, '\n');
    }
    ds_put_cstr(&txn->comment, s);
}

const char *
ovsdb_txn_get_comment(const struct ovsdb_txn *txn)
{
    return txn->comment.length ? ds_cstr_ro(&txn->comment) : NULL;
}

static void
ovsdb_txn_row_prefree(struct ovsdb_txn_row *txn_row)
{
    struct ovsdb_txn_table *txn_table = txn_row->table->txn_table;

    txn_table->n_processed--;
    hmap_remove(&txn_table->txn_rows, &txn_row->hmap_node);

    if (txn_row->old) {
        txn_row->old->txn_row = NULL;
    }
    if (txn_row->new) {
        txn_row->new->txn_row = NULL;
    }
}

static void
ovsdb_txn_table_destroy(struct ovsdb_txn_table *txn_table)
{
    size_t i;

    ovs_assert(hmap_is_empty(&txn_table->txn_rows));

    for (i = 0; i < txn_table->table->schema->n_indexes; i++) {
        hmap_destroy(&txn_table->txn_indexes[i]);
    }
    free(txn_table->txn_indexes);

    txn_table->table->txn_table = NULL;
    hmap_destroy(&txn_table->txn_rows);
    ovs_list_remove(&txn_table->node);
    free(txn_table);
}

/* Calls 'cb' for every txn_row within 'txn'.  If 'cb' returns nonnull, this
 * aborts the iteration and for_each_txn_row() passes the error up.  Otherwise,
 * returns a null pointer after iteration is complete.
 *
 * 'cb' may insert new txn_rows and new txn_tables into 'txn'.  It may delete
 * the txn_row that it is passed in, or txn_rows in txn_tables other than the
 * one passed to 'cb'.  It may *not* delete txn_rows other than the one passed
 * in within the same txn_table.  It may *not* delete any txn_tables.  As long
 * as these rules are followed, 'cb' will be called exactly once for each
 * txn_row in 'txn', even those added by 'cb'.
 *
 * (Even though 'cb' is not allowed to delete some txn_rows, it can still
 * delete any actual row by clearing a txn_row's 'new' member.)
 */
static struct ovsdb_error * OVS_WARN_UNUSED_RESULT
for_each_txn_row(struct ovsdb_txn *txn,
                 struct ovsdb_error *(*cb)(struct ovsdb_txn *,
                                           struct ovsdb_txn_row *))
{
    bool any_work;

    serial++;

    do {
        struct ovsdb_txn_table *t, *next_txn_table;

        any_work = false;
        LIST_FOR_EACH_SAFE (t, next_txn_table, node, &txn->txn_tables) {
            if (t->serial != serial) {
                t->serial = serial;
                t->n_processed = 0;
            }

            while (t->n_processed < hmap_count(&t->txn_rows)) {
                struct ovsdb_txn_row *r, *next_txn_row;

                HMAP_FOR_EACH_SAFE (r, next_txn_row, hmap_node, &t->txn_rows) {
                    if (r->serial != serial) {
                        struct ovsdb_error *error;

                        r->serial = serial;
                        t->n_processed++;
                        any_work = true;

                        error = cb(txn, r);
                        if (error) {
                            return error;
                        }
                    }
                }
            }
            if (hmap_is_empty(&t->txn_rows)) {
                /* Table is empty.  Drop it. */
                ovsdb_txn_table_destroy(t);
            }
        }
    } while (any_work);

    return NULL;
}

void
ovsdb_txn_history_run(struct ovsdb *db)
{
    if (!db->need_txn_history) {
        return;
    }
    /* Remove old histories to limit the size of the history */
    while (db->n_txn_history > 100) {
        struct ovsdb_txn_history_node *txn_h_node = CONTAINER_OF(
                ovs_list_pop_front(&db->txn_history),
                struct ovsdb_txn_history_node, node);

        ovsdb_txn_destroy_cloned(txn_h_node->txn);
        free(txn_h_node);
        db->n_txn_history--;
    }
}

void
ovsdb_txn_history_init(struct ovsdb *db, bool need_txn_history)
{
    db->need_txn_history = need_txn_history;
    db->n_txn_history = 0;
    ovs_list_init(&db->txn_history);
}

void
ovsdb_txn_history_destroy(struct ovsdb *db)
{

    if (!db->need_txn_history) {
        return;
    }

    struct ovsdb_txn_history_node *txn_h_node, *next;
    LIST_FOR_EACH_SAFE (txn_h_node, next, node, &db->txn_history) {
        ovs_list_remove(&txn_h_node->node);
        ovsdb_txn_destroy_cloned(txn_h_node->txn);
        free(txn_h_node);
    }
    db->n_txn_history = 0;
}
