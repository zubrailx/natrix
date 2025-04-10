#include "cmd.h"

#include "debugger/cmd/handler.h"
#include "util/macro.h"
#include "util/math.h"
#include "util/string.h"

static const char *DESC = "Help";

static const char *HELP = "Usage: <command> [<cmd>]";

static void cmd_help_all(cmd_handler *handler, ctx *ctx) {
  ctx_out_appendln(ctx, "Commands:");
  size_t first_w     = 0;
  size_t entries_cnt = 0;
  for (hashset_cmd_it it = hashset_cmd_begin(handler->cmds); !END(it);
       NEXT(it)) {
    cmd *entry = GET(it);
    first_w    = max_size_t(first_w, strlen(entry->cmd_ref));
    ++entries_cnt;
  }

  // sort elements
  cmd **entries = malloc(sizeof(cmd *) * entries_cnt);

  size_t i = 0;
  for (hashset_cmd_it it = hashset_cmd_begin(handler->cmds); !END(it);
       NEXT(it)) {
    cmd *entry   = GET(it);
    entries[i++] = entry;
  }

  for (size_t i = 0; i < entries_cnt - 1; ++i) {
    for (size_t j = 0; j < entries_cnt - i - 1; ++j) {
      if (strcmp(entries[j]->cmd_ref, entries[j + 1]->cmd_ref) > 0) {
        cmd *tmp       = entries[j];
        entries[j]     = entries[j + 1];
        entries[j + 1] = tmp;
      }
    }
  }

  for (size_t i = 0; i < entries_cnt; ++i) {
    cmd *entry = entries[i];
    ctx_out_append(ctx, entry->cmd_ref);

    for (size_t i = 0; i < first_w - strlen(entry->cmd_ref); ++i) {
      ctx_out_append(ctx, " ");
    }
    ctx_out_append(ctx, " -- ");
    ctx_out_appendln(ctx, entry->desc_ref);
  }

  free(entries);
}

static void cmd_help_one(ctx *ctx, cmd *entry) {
  ctx_out_append(ctx, entry->cmd_ref);
  ctx_out_append(ctx, " -- ");
  ctx_out_appendln(ctx, entry->desc_ref);
  ctx_out_appendln(ctx, entry->help_ref);
}

static void cmd_help_fn(cmd_handler *handler, ctx *ctx, const char *rest) {
  char *args = trim(strdup(rest));

  if (!args || !*args) {
    cmd_help_all(handler, ctx);
  } else {
    int found = 0;
    for (hashset_cmd_it it = hashset_cmd_begin(handler->cmds); !END(it);
         NEXT(it)) {
      cmd *entry = GET(it);
      if (!strcmp(args, entry->cmd_ref)) {
        cmd_help_one(ctx, entry);
        found = 1;
      }
    }
    if (!found) {
      ctx_error(ctx, "command not found");
      cmd_help_all(handler, ctx);
    }
  }

  if (args) {
    free(args);
  }
}

cmd *cmd_help(const char *cmd_ref) {
  return cmd_entry_new(cmd_help_fn, DESC, HELP, cmd_ref);
}
