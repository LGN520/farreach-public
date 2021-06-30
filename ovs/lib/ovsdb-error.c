/* Copyright (c) 2009, 2010, 2011, 2012, 2016, 2017 Nicira, Inc.
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

#include "ovsdb-error.h"

#include <inttypes.h>

#include "backtrace.h"
#include "openvswitch/dynamic-string.h"
#include "openvswitch/json.h"
#include "util.h"
#include "openvswitch/vlog.h"

VLOG_DEFINE_THIS_MODULE(ovsdb_error);

struct ovsdb_error {
    const char *tag;            /* String for "error" member. */
    char *details;              /* String for "details" member. */
    char *syntax;               /* String for "syntax" member. */
    int errno_;                 /* Unix errno value, 0 if none. */
};

static struct ovsdb_error *
ovsdb_error_valist(const char *tag, const char *details, va_list args)
{
    struct ovsdb_error *error = xmalloc(sizeof *error);
    error->tag = tag ? tag : "ovsdb error";
    error->details = details ? xvasprintf(details, args) : NULL;
    error->syntax = NULL;
    error->errno_ = 0;
    return error;
}

struct ovsdb_error *
ovsdb_error(const char *tag, const char *details, ...)
{
    struct ovsdb_error *error;
    va_list args;

    va_start(args, details);
    error = ovsdb_error_valist(tag, details, args);
    va_end(args);

    return error;
}

struct ovsdb_error *
ovsdb_io_error(int errno_, const char *details, ...)
{
    struct ovsdb_error *error;
    va_list args;

    va_start(args, details);
    error = ovsdb_error_valist("I/O error", details, args);
    va_end(args);

    error->errno_ = errno_;

    return error;
}

struct ovsdb_error *
ovsdb_syntax_error(const struct json *json, const char *tag,
                   const char *details, ...)
{
    struct ovsdb_error *error;
    va_list args;

    va_start(args, details);
    error = ovsdb_error_valist(tag ? tag : "syntax error", details, args);
    va_end(args);

    if (json) {
        /* XXX this is much too much information in some cases */
        error->syntax = json_to_string(json, JSSF_SORT);
    }

    return error;
}

struct ovsdb_error *
ovsdb_wrap_error(struct ovsdb_error *error, const char *details, ...)
{
    va_list args;
    char *msg;

    va_start(args, details);
    msg = xvasprintf(details, args);
    va_end(args);

    if (error->details) {
        char *new = xasprintf("%s: %s", msg, error->details);
        free(error->details);
        error->details = new;
        free(msg);
    } else {
        error->details = msg;
    }

    return error;
}

/* Returns an ovsdb_error that represents an internal error for file name
 * 'file' and line number 'line', with 'details' (formatted as with printf())
 * as the associated message.  The caller is responsible for freeing the
 * returned error.
 *
 * If 'inner_error' is nonnull then the returned error is wrapped around
 * 'inner_error'.  Takes ownership of 'inner_error'.  */
struct ovsdb_error *
ovsdb_internal_error(struct ovsdb_error *inner_error,
                     const char *file, int line, const char *details, ...)
{
    struct ds ds = DS_EMPTY_INITIALIZER;
    struct backtrace backtrace;
    struct ovsdb_error *error;
    va_list args;

    ds_put_format(&ds, "%s:%d:", file, line);

    if (details) {
        ds_put_char(&ds, ' ');
        va_start(args, details);
        ds_put_format_valist(&ds, details, args);
        va_end(args);
    }

    backtrace_capture(&backtrace);
    if (backtrace.n_frames) {
        int i;

        ds_put_cstr(&ds, " (backtrace:");
        for (i = 0; i < backtrace.n_frames; i++) {
            ds_put_format(&ds, " 0x%08"PRIxPTR, backtrace.frames[i]);
        }
        ds_put_char(&ds, ')');
    }

    ds_put_format(&ds, " (%s %s)", program_name, VERSION);

    if (inner_error) {
        char *s = ovsdb_error_to_string_free(inner_error);
        ds_put_format(&ds, " (generated from: %s)", s);
        free(s);
    }

    error = ovsdb_error("internal error", "%s", ds_cstr(&ds));

    ds_destroy(&ds);

    return error;
}

struct ovsdb_error *
ovsdb_perm_error(const char *details, ...)
{
    struct ovsdb_error *error;
    va_list args;

    va_start(args, details);
    error = ovsdb_error_valist("permission error", details, args);
    va_end(args);

    return error;
}

void
ovsdb_error_destroy(struct ovsdb_error *error)
{
    if (error) {
        free(error->details);
        free(error->syntax);
        free(error);
    }
}

struct ovsdb_error *
ovsdb_error_clone(const struct ovsdb_error *old)
{
    if (old) {
        struct ovsdb_error *new = xmalloc(sizeof *new);
        new->tag = old->tag;
        new->details = nullable_xstrdup(old->details);
        new->syntax = nullable_xstrdup(old->syntax);
        new->errno_ = old->errno_;
        return new;
    } else {
        return NULL;
    }
}

/* Returns 'error' converted to the <error> JSON object format described in RFC
 * 7047.  The caller must free the returned json (with json_destroy()). */
struct json *
ovsdb_error_to_json(const struct ovsdb_error *error)
{
    struct json *json = json_object_create();
    json_object_put_string(json, "error", error->tag);
    if (error->details) {
        json_object_put_string(json, "details", error->details);
    }

    /* These are RFC 7047-compliant extensions. */
    if (error->syntax) {
        json_object_put_string(json, "syntax", error->syntax);
    }
    if (error->errno_) {
        json_object_put_string(json, "io-error",
                               ovs_retval_to_string(error->errno_));
    }

    return json;
}

/* Returns 'error' converted to the <error> JSON object format described in RFC
 * 7047.  The caller must free the returned json (with json_destroy()).
 *
 * Also, frees 'error'. */
struct json *
ovsdb_error_to_json_free(struct ovsdb_error *error)
{
    struct json *json = ovsdb_error_to_json(error);
    ovsdb_error_destroy(error);
    return json;
}

/* Returns 'error' converted to a string suitable for use as an error message.
 * The caller must free the returned string (with free()). */
char *
ovsdb_error_to_string(const struct ovsdb_error *error)
{
    struct ds ds = DS_EMPTY_INITIALIZER;
    if (error->syntax) {
        ds_put_format(&ds, "syntax \"%s\": ", error->syntax);
    }
    ds_put_cstr(&ds, error->tag);
    if (error->details) {
        ds_put_format(&ds, ": %s", error->details);
    }
    if (error->errno_) {
        ds_put_format(&ds, " (%s)", ovs_retval_to_string(error->errno_));
    }
    return ds_steal_cstr(&ds);
}

/* Returns 'error' converted to a string suitable for use as an error message.
 * The caller must free the returned string (with free()).
 *
 * If 'error' is NULL, returns NULL.
 *
 * Also, frees 'error'. */
char *
ovsdb_error_to_string_free(struct ovsdb_error *error)
{
    if (error) {
        char *s = ovsdb_error_to_string(error);
        ovsdb_error_destroy(error);
        return s;
    } else {
        return NULL;
    }
}

const char *
ovsdb_error_get_tag(const struct ovsdb_error *error)
{
    return error->tag;
}

/* If 'error' is nonnull, logs it as an error and frees it.  To be used in
 * situations where an error should never occur, but an 'ovsdb_error *' gets
 * passed back anyhow. */
void
ovsdb_error_assert(struct ovsdb_error *error)
{
    if (error) {
        static struct vlog_rate_limit rl = VLOG_RATE_LIMIT_INIT(1, 1);
        char *s = ovsdb_error_to_string_free(error);
        VLOG_ERR_RL(&rl, "unexpected ovsdb error: %s", s);
        free(s);
    }
}
