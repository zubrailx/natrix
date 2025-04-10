#include "handler.h"

#include "util/log.h"
#include "util/macro.h"
#include "util/string.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

cmd_handler *cmd_handler_new() {
  cmd_handler *self   = MALLOC(cmd_handler);
  self->cmds          = hashset_cmd_new();
  self->cmd_prev      = NULL;
  self->cmd_prev_rest = NULL;
  return self;
}

void cmd_handler_free(cmd_handler *self) {
  if (self) {
    hashset_cmd_free(self->cmds);
    self->cmd_prev = NULL;
    if (self->cmd_prev_rest) {
      free(self->cmd_prev_rest);
    }
    free(self);
  }
}

void cmd_handler_register(cmd_handler *self, cmd *entry) {
  hashset_cmd_insert(self->cmds, entry);
  debug("handler %p added cmd %s", self, entry->cmd_ref);
}

void cmd_handler_handle(cmd_handler *self, ctx *ctx, const char *cmd_ref) {
  char  c_tmp;
  char *args       = strdup(cmd_ref);
  char *args_start = trim_start(args);
  char *args_rest  = args_start;

  while (*args_rest && !isspace((unsigned char)*args_rest)) {
    ++args_rest;
  }
  // break string to hash it
  c_tmp      = *args_rest;
  *args_rest = '\0';

  // repeat command if exists
  if (!*args_start) {
    if (self->cmd_prev) {
      self->cmd_prev->fn(self, ctx, self->cmd_prev_rest);
    }
    goto cleanup;
  }

  hashset_cmd_it it =
      hashset_cmd_find(self->cmds, &(cmd){.cmd_ref = (char *)args_start});

  if (END(it)) {
    ctx->s_debugger = DEBUGGER_ERROR;
    ctx_err_append(ctx, "unknown command: ");
    ctx_err_append(ctx, args_start);
    ctx_err_appendln(ctx, ", try help");
    goto cleanup;
  }

  cmd *entry = GET(it);

  *args_rest = c_tmp;
  while (isspace((unsigned char)*args_rest)) {
    ++args_rest;
  }

  ctx->s_debugger = DEBUGGER_PROCESSING;
  entry->fn(self, ctx, args_rest);

  self->cmd_prev = entry;
  if (self->cmd_prev_rest) {
    free(self->cmd_prev_rest);
  }
  self->cmd_prev_rest = strdup(args_rest);

  if (ctx->s_debugger == DEBUGGER_PROCESSING) {
    ctx->s_debugger = DEBUGGER_IDLE;
  }

cleanup:
  if (args) {
    free(args);
  }
}
