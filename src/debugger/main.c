#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "debugger/cmd/cmd.h"
#include "debugger/cmd/handler.h"
#include "debugger/ctx/ctx.h"
#include "debugger/ctx/str.h"
#include "util/log.h"
#include "util/macro.h"

typedef struct args_struct {
  char *prog_name;
  // options
  char *init_file;
  // status
  int code;
  int help;
} args;

static args *args_new() {
  args *args = MALLOC(struct args_struct);

  args->prog_name = NULL;
  args->init_file = NULL;

  args->code = 0;
  args->help = 0;

  return args;
}

static void args_free(args *args) {
  if (args) {
    free(args);
  }
}

static void usage(args *args) {
  printf("Usage: %s [options] <file>...\n"
         "Options:\n"
         "-i <file>  - init file (current: %s)\n"
         "-h         - show help\n",
         args->prog_name, args->init_file);
}

static void parse(args *args, int argc, char *argv[]) {
  int           option_index;
  int           has_next;
  struct option long_options[] = {};

  if (argc > 0) {
    args->prog_name = argv[0];
  }

  option_index = 0;
  has_next     = 1;

  while (has_next) {
    unsigned c;
    c = getopt_long(argc, argv, "i:h", long_options, &option_index);

    switch (c) {
      case EOF:
        has_next = 0;
        break;
      case 0:
        if (long_options[option_index].flag) {
          break;
        }
        break;
      case 'i':
        args->init_file = optarg;
        break;
      case 'h':
        args->help = 1;
        has_next   = 0;
        break;
      default:
        args->code = -1;
        args->help = 1;
        has_next   = 0;
        break;
    }
  }

  argc -= optind;
  argv += optind;
}

static char *next_cmd(FILE *init_fp, FILE **fp_out) {
  char  *line = NULL;
  size_t len  = 0;

  if (init_fp && getline(&line, &len, init_fp) != -1) {
    *fp_out = init_fp;
  } else if (getline(&line, &len, stdin) != -1) {
    *fp_out = stdin;
  } else if (line) {
    free(line);
    line = NULL;
  }

  return line;
}

static void register_cmds(cmd_handler *h) {
  cmd_handler_register(h, cmd_exit("exit"));
  cmd_handler_register(h, cmd_echo("echo"));
  cmd_handler_register(h, cmd_file("file"));
  cmd_handler_register(h, cmd_file("f"));
  cmd_handler_register(h, cmd_run("run"));
  cmd_handler_register(h, cmd_run("r"));
  cmd_handler_register(h, cmd_info("info"));
  cmd_handler_register(h, cmd_info("i"));
  cmd_handler_register(h, cmd_kill("kill"));
  cmd_handler_register(h, cmd_kill("k"));
  cmd_handler_register(h, cmd_print("print"));
  cmd_handler_register(h, cmd_print("p"));
  cmd_handler_register(h, cmd_examine("examine"));
  cmd_handler_register(h, cmd_examine("x"));
  cmd_handler_register(h, cmd_disassemble("disassemble"));
  cmd_handler_register(h, cmd_disassemble("dis"));
  cmd_handler_register(h, cmd_continue("continue"));
  cmd_handler_register(h, cmd_continue("c"));
  cmd_handler_register(h, cmd_help("help"));
  cmd_handler_register(h, cmd_help("h"));
  cmd_handler_register(h, cmd_stepi("stepi"));
  cmd_handler_register(h, cmd_stepi("si"));
  cmd_handler_register(h, cmd_nexti("nexti"));
  cmd_handler_register(h, cmd_nexti("ni"));
  cmd_handler_register(h, cmd_break("break"));
  cmd_handler_register(h, cmd_break("b"));
  cmd_handler_register(h, cmd_delete("delete"));
  cmd_handler_register(h, cmd_delete("d"));
  cmd_handler_register(h, cmd_config("config"));
  cmd_handler_register(h, cmd_next("next"));
  cmd_handler_register(h, cmd_next("n"));
  cmd_handler_register(h, cmd_step("step"));
  cmd_handler_register(h, cmd_step("s"));
  cmd_handler_register(h, cmd_where("where"));
  cmd_handler_register(h, cmd_where("w"));
}

static int execute(args *args) {
  FILE *init_fp = NULL;

  if (args->init_file) {
    init_fp = fopen(args->init_file, "r");
    if (!init_fp) {
      warn("init file %s doesn't exist", args->init_file);
    }
  }

  ctx *ctx = ctx_new();

  cmd_handler *handler = cmd_handler_new();
  register_cmds(handler);

  while (ctx->s_debugger != DEBUGGER_EXITING) {
    FILE *fp = NULL;

    fprintf(stdout, "(dbg) ");

    char *cmd = next_cmd(init_fp, &fp);

    if (cmd) {
      if (fileno(fp) != STDIN_FILENO) {
        fprintf(stdout, "%s", cmd);
      }
      cmd_handler_handle(handler, ctx, cmd);
      free(cmd);
    } else {
      ctx->s_debugger = DEBUGGER_EXITING;
    }

    debug("debugger: %s, target: %s", state_debugger_enum_str(ctx->s_debugger),
          state_target_enum_str(ctx_tg_get_state(ctx)));

    if (ctx->s_debugger == DEBUGGER_ERROR) {
      fprintf(stderr, "error: %s", strbuf_data(ctx->buf_err));
      strbuf_reset(ctx->buf_err);
      ctx->s_debugger = DEBUGGER_IDLE;
    }

    if (strbuf_size(ctx->buf_out)) {
      fprintf(stdout, "%s", strbuf_data(ctx->buf_out));
      strbuf_reset(ctx->buf_out);
    }
  }

  cmd_handler_free(handler);
  ctx_free(ctx);

  if (init_fp) {
    fclose(init_fp);
  }

  return args->code;
}

int main(int argc, char *argv[]) {
  int   code;
  args *args = args_new();
  parse(args, argc, argv);

  if (args->help) {
    usage(args);
  }

  if (!args->help && args->code == 0) {
    args->code = execute(args);
  }

  code = args->code;

  args_free(args);

  return code;
}
