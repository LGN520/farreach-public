/*
 * Copyright (c) 2009, 2010, 2011, 2012, 2014, 2016 Nicira, Inc.
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
#undef NDEBUG
#include "reconnect.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "command-line.h"
#include "compiler.h"
#include "ovstest.h"
#include "svec.h"
#include "util.h"
#include "openvswitch/vlog.h"

static struct reconnect *reconnect;
static int now;

static void diff_stats(const struct reconnect_stats *old,
                       const struct reconnect_stats *new,
                       int delta);
static const struct ovs_cmdl_command *get_all_commands(void);

static void
test_reconnect_main(int argc OVS_UNUSED, char *argv[] OVS_UNUSED)
{
    struct reconnect_stats prev;
    unsigned int old_max_tries;
    int old_time;
    char line[128];

    vlog_set_levels_from_string_assert("reconnect:off");

    now = 1000;
    reconnect = reconnect_create(now);
    reconnect_set_name(reconnect, "remote");
    reconnect_get_stats(reconnect, now, &prev);
    printf("### t=%d ###\n", now);
    old_time = now;
    old_max_tries = reconnect_get_max_tries(reconnect);
    while (fgets(line, sizeof line, stdin)) {
        struct reconnect_stats cur;
        struct svec args;

        fputs(line, stdout);
        if (line[0] == '#') {
            continue;
        }

        svec_init(&args);
        svec_parse_words(&args, line);
        svec_terminate(&args);
        if (!svec_is_empty(&args)) {
            struct ovs_cmdl_context ctx = {
                .argc = args.n,
                .argv = args.names,
            };
            ovs_cmdl_run_command(&ctx, get_all_commands());
        }
        svec_destroy(&args);

        if (old_time != now) {
            printf("\n### t=%d ###\n", now);
        }

        reconnect_get_stats(reconnect, now, &cur);
        diff_stats(&prev, &cur, now - old_time);
        prev = cur;
        if (reconnect_get_max_tries(reconnect) != old_max_tries) {
            old_max_tries = reconnect_get_max_tries(reconnect);
            printf("  %u tries left\n", old_max_tries);
        }

        old_time = now;
    }
}

static void
do_enable(struct ovs_cmdl_context *ctx OVS_UNUSED)
{
    reconnect_enable(reconnect, now);
}

static void
do_disable(struct ovs_cmdl_context *ctx OVS_UNUSED)
{
    reconnect_disable(reconnect, now);
}

static void
do_force_reconnect(struct ovs_cmdl_context *ctx OVS_UNUSED)
{
    reconnect_force_reconnect(reconnect, now);
}

static int
error_from_string(const char *s)
{
    if (!s) {
        return 0;
    } else if (!strcmp(s, "ECONNREFUSED")) {
        return ECONNREFUSED;
    } else if (!strcmp(s, "EOF")) {
        return EOF;
    } else {
        ovs_fatal(0, "unknown error '%s'", s);
    }
}

static void
do_disconnected(struct ovs_cmdl_context *ctx)
{
    reconnect_disconnected(reconnect, now, error_from_string(ctx->argv[1]));
}

static void
do_connecting(struct ovs_cmdl_context *ctx OVS_UNUSED)
{
    reconnect_connecting(reconnect, now);
}

static void
do_connect_failed(struct ovs_cmdl_context *ctx)
{
    reconnect_connect_failed(reconnect, now, error_from_string(ctx->argv[1]));
}

static void
do_connected(struct ovs_cmdl_context *ctx OVS_UNUSED)
{
    reconnect_connected(reconnect, now);
}

static void
do_activity(struct ovs_cmdl_context *ctx OVS_UNUSED)
{
    reconnect_activity(reconnect, now);
}

static void
do_run(struct ovs_cmdl_context *ctx)
{
    enum reconnect_action action;

    if (ctx->argc > 1) {
        now += atoi(ctx->argv[1]);
    }

    action = reconnect_run(reconnect, now);
    switch (action) {
    default:
        if (action != 0) {
            OVS_NOT_REACHED();
        }
        break;

    case RECONNECT_CONNECT:
        printf("  should connect\n");
        break;

    case RECONNECT_DISCONNECT:
        printf("  should disconnect\n");
        break;

    case RECONNECT_PROBE:
        printf("  should send probe\n");
        break;
    }
}

static void
do_advance(struct ovs_cmdl_context *ctx)
{
    now += atoi(ctx->argv[1]);
}

static void
do_timeout(struct ovs_cmdl_context *ctx OVS_UNUSED)
{
    int timeout = reconnect_timeout(reconnect, now);
    if (timeout >= 0) {
        printf("  advance %d ms\n", timeout);
        now += timeout;
    } else {
        printf("  no timeout\n");
    }
}

static void
do_set_max_tries(struct ovs_cmdl_context *ctx)
{
    reconnect_set_max_tries(reconnect, atoi(ctx->argv[1]));
}

static void
do_set_backoff_free_tries(struct ovs_cmdl_context *ctx)
{
    reconnect_set_backoff_free_tries(reconnect, atoi(ctx->argv[1]));
}

static void
diff_stats(const struct reconnect_stats *old,
           const struct reconnect_stats *new,
           int delta)
{
    if (old->state != new->state
        || old->state_elapsed != new->state_elapsed
        || old->backoff != new->backoff) {
        printf("  in %s for %u ms (%d ms backoff)\n",
               new->state, new->state_elapsed, new->backoff);
    }
    if (old->creation_time != new->creation_time
        || old->last_activity != new->last_activity
        || old->last_connected != new->last_connected) {
        printf("  created %lld, last activity %lld, last connected %lld\n",
               new->creation_time, new->last_activity, new->last_connected);
    }
    if (old->n_successful_connections != new->n_successful_connections
        || old->n_attempted_connections != new->n_attempted_connections
        || old->seqno != new->seqno) {
        printf("  %u successful connections out of %u attempts, seqno %u\n",
               new->n_successful_connections, new->n_attempted_connections,
               new->seqno);
    }
    if (old->is_connected != new->is_connected) {
        printf("  %sconnected\n", new->is_connected ? "" : "dis");
    }
    if (old->last_connected != new->last_connected
        || (old->msec_since_connect != new->msec_since_connect - delta
            && !(old->msec_since_connect == UINT_MAX
                 && new->msec_since_connect == UINT_MAX))
        || (old->total_connected_duration != new->total_connected_duration - delta
            && !(old->total_connected_duration == 0
                 && new->total_connected_duration == 0))) {
        printf("  last connected %u ms ago, connected %u ms total\n",
               new->msec_since_connect, new->total_connected_duration);
    }
    if (old->last_disconnected != new->last_disconnected
        || (old->msec_since_disconnect != new->msec_since_disconnect - delta
            && !(old->msec_since_disconnect == UINT_MAX
                 && new->msec_since_disconnect == UINT_MAX))) {
        printf("  disconnected at %llu ms (%u ms ago)\n",
               new->last_disconnected, new->msec_since_disconnect);
    }
}

static void
do_set_passive(struct ovs_cmdl_context *ctx OVS_UNUSED)
{
    reconnect_set_passive(reconnect, true, now);
}

static void
do_listening(struct ovs_cmdl_context *ctx OVS_UNUSED)
{
    reconnect_listening(reconnect, now);
}

static void
do_listen_error(struct ovs_cmdl_context *ctx)
{
    reconnect_listen_error(reconnect, now, atoi(ctx->argv[1]));
}

static void
do_receive_attempted(struct ovs_cmdl_context *ctx OVS_UNUSED)
{
    if (!strcmp(ctx->argv[1], "now")) {
        reconnect_receive_attempted(reconnect, now);
    } else if (!strcmp(ctx->argv[1], "LLONG_MAX")) {
        reconnect_receive_attempted(reconnect, LLONG_MAX);
    } else {
        ovs_fatal(0, "%s: bad argument %s", ctx->argv[0], ctx->argv[1]);
    }
}

static const struct ovs_cmdl_command all_commands[] = {
    { "enable", NULL, 0, 0, do_enable, OVS_RO },
    { "disable", NULL, 0, 0, do_disable, OVS_RO },
    { "force-reconnect", NULL, 0, 0, do_force_reconnect, OVS_RO },
    { "disconnected", NULL, 0, 1, do_disconnected, OVS_RO },
    { "connecting", NULL, 0, 0, do_connecting, OVS_RO },
    { "connect-failed", NULL, 0, 1, do_connect_failed, OVS_RO },
    { "connected", NULL, 0, 0, do_connected, OVS_RO },
    { "activity", NULL, 0, 0, do_activity, OVS_RO },
    { "run", NULL, 0, 1, do_run, OVS_RO },
    { "advance", NULL, 1, 1, do_advance, OVS_RO },
    { "timeout", NULL, 0, 0, do_timeout, OVS_RO },
    { "set-max-tries", NULL, 1, 1, do_set_max_tries, OVS_RO },
    { "set-backoff-free-tries", NULL, 1, 1, do_set_backoff_free_tries,
      OVS_RO },
    { "passive", NULL, 0, 0, do_set_passive, OVS_RO },
    { "listening", NULL, 0, 0, do_listening, OVS_RO },
    { "listen-error", NULL, 1, 1, do_listen_error, OVS_RO },
    { "receive-attempted", NULL, 1, 1, do_receive_attempted, OVS_RO },
    { NULL, NULL, 0, 0, NULL, OVS_RO },
};

static const struct ovs_cmdl_command *
get_all_commands(void)
{
    return all_commands;
}

OVSTEST_REGISTER("test-reconnect", test_reconnect_main);
