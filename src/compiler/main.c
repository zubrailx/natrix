#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "compiler/ast/ast.h"
#include "compiler/codegen/x86_64/x86_64.h"
#include "compiler/codegen/x86_64_build/x86_64_build.h"
#include "compiler/codegen/x86_64_emit/x86_64_emit.h"
#include "compiler/dot/dot.h"
#include "compiler/exception/exception.h"
#include "compiler/hir/hir.h"
#include "compiler/hir/str.h"
#include "compiler/hir_build/hir_build.h"
#include "compiler/mir/mir.h"
#include "compiler/mir/str.h"
#include "compiler/mir_build/mir_build.h"
#include "compiler/symbol_table/str.h"
#include "compiler/type_table/str.h"
#include "util/container_util.h"
#include "util/file.h"
#include "util/list.h"
#include "util/macro.h"

LIST_DECLARE_STATIC_INLINE(list_chars, char, container_cmp_chars,
                           container_new_move, container_delete_chars);

typedef struct args_struct {
  char       *prog_name;
  list_chars *input_files;
  // options
  char       *output_dir;
  char       *output_file;
  int         ignore_errors;
  int         tee;
  int         ast;
  int         cfg;
  int         cfg_add_expr;
  int         cg;
  list_chars *cg_subroutines;
  int         hir_tree;
  int         hir_symbols;
  int         hir_types;
  int         mir;
  // status
  int code;
  int help;
} args;

static void print_exception(args *args, exception *exc);

static args *args_new() {
  args *args = MALLOC(struct args_struct);

  args->prog_name   = NULL;
  args->input_files = list_chars_new();

  args->output_dir    = ".";
  args->output_file   = "a.asm";
  args->ignore_errors = 0;
  args->tee           = 0;

  args->ast = 0;

  args->cfg            = 0;
  args->cfg_add_expr   = 0;
  args->cg             = 0;
  args->cg_subroutines = list_chars_new();

  args->hir_tree    = 0;
  args->hir_symbols = 0;
  args->hir_types   = 0;

  args->mir = 0;

  args->code = 0;
  args->help = 0;

  return args;
}

static void args_free(args *args) {
  list_chars_free(args->input_files);
  list_chars_free(args->cg_subroutines);
  free(args);
}

static void usage(args *args) {
  int  buf_len = 256;
  char cg_subroutines[buf_len];
  int  offset = 0;
  for (list_chars_it it = list_chars_begin(args->cg_subroutines); !END(it);
       NEXT(it)) {
    char *sub = GET(it);
    offset +=
        snprintf(cg_subroutines + offset, buf_len - offset - 1, "%s,", sub);
  }
  if (offset > 0) {
    cg_subroutines[offset - 1] = '\0';
  }

  printf("Usage: %s [options] <file>...\n"
         "Options:\n"
         "-d <directory>   - output directory (current: %s)\n"
         "-o <file>        - main output file (current: %s)\n"
         "--tee            - print to file and to stdout (current: %d)\n"
         "--ignore-errors  - continue execution on errors (current: %d)\n"
         "--ast            - add AST output (current: %d)\n"
         "--cfg            - add global subroutines control flow graph output "
         "(current: %d)\n"
         "--cfg-add-expr   - include expressions in control flow graph "
         "(current: %d)\n"
         "--cg             - add global subroutines call graph output "
         "(current: %d)\n"
         "-s <subroutine>  - set or add global subroutine for call graph "
         "generation (current: %s)\n"
         "--hir-tree       - print HIR tree (current: %d)\n"
         "--hir-symbols    - print HIR symbol table (current: %d)\n"
         "--hir-types      - print HIR type table (current: %d)\n"
         "--mir            - print MIR tree (current: %d)\n"
         "-h\n"
         "--help           - show help\n",
         args->prog_name, args->output_dir, args->output_file, args->tee,
         args->ignore_errors, args->ast, args->cfg, args->cfg_add_expr,
         args->cg, cg_subroutines, args->hir_tree, args->hir_symbols,
         args->hir_types, args->mir);
}

static void parse(args *args, int argc, char *argv[]) {
  int           option_index;
  int           has_next;
  struct option long_options[] = {
      {"tee", no_argument, &args->tee, 1},
      {"ignore-errors", no_argument, &args->ignore_errors, 1},
      {"ast", no_argument, &args->ast, 1},
      {"cfg", no_argument, &args->cfg, 1},
      {"cfg-add-expr", no_argument, &args->cfg_add_expr, 1},
      {"cg", no_argument, &args->cg, 1},
      {"hir-tree", no_argument, &args->hir_tree, 1},
      {"hir-symbols", no_argument, &args->hir_symbols, 1},
      {"hir-types", no_argument, &args->hir_types, 1},
      {"mir", no_argument, &args->mir, 1},
      {"help", no_argument, &args->help, 1},
  };

  if (argc > 0) {
    args->prog_name = argv[0];
  }

  option_index = 0;
  has_next     = 1;

  while (has_next) {
    unsigned c;

    c = getopt_long(argc, argv, "o:d:s:h", long_options, &option_index);

    switch (c) {
      case EOF:
        has_next = 0;
        break;
      case 0:
        if (long_options[option_index].flag) {
          if (args->help) {
            has_next = 0;
          }
          break;
        }
        break;
      case 'd':
        args->output_dir = optarg;
        break;
      case 'o':
        args->output_file = optarg;
        break;
      case 's':
        list_chars_push_back(args->cg_subroutines, strdup(optarg));
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

  for (; argc > 0; --argc, ++argv) {
    char *file = argv[0];
    char *path = realpath(file, NULL);

    if (!path) {
      exception *exc = exception_new_f(
          EXCEPTION_LEVEL_ERROR, EXCEPTION_UNKNOWN, 0, file, 0, 0, "not found",
          "file '%s' path can't be resolved", file);
      print_exception(args, exc);
      exception_free(exc);
      continue;
    }

    list_chars_push_back(args->input_files, path);
  }

  if (list_chars_size(args->cg_subroutines) == 0) {
    list_chars_push_back(args->cg_subroutines, strdup("main"));
  }
}

// filename is not directory
static void ensure_dir_exist(const char *filename) {
  char *dir = dirname(filename);
  dir_create_p(dir);
  free(dir);
}

// ast - ext, dot - file extension
static char *get_ast_dot_path(const args *args, const ast *ast) {
  char *cwd    = getcwd(NULL, 0);
  char *suffix = ".ast.dot";

  const char *fpath  = ast->name_ref + strlen(cwd) + 1;
  char       *prefix = join_paths(args->output_dir, fpath);

  char *path = malloc(strlen(prefix) + strlen(suffix) + 1);
  sprintf(path, "%s%s", prefix, suffix);

  free(prefix);
  free(cwd);
  return path;
}

static char *get_c_dot_path(args *args, const char *source, const char *func,
                            const char *ext) {
  char *cwd    = getcwd(NULL, 0);
  char *suffix = ".dot";

  const char *fpath  = source + strlen(cwd) + 1;
  char       *prefix = join_paths(args->output_dir, fpath);

  char *path =
      malloc(strlen(prefix) + strlen(func) + strlen(ext) + strlen(suffix) + 3);
  sprintf(path, "%s.%s.%s%s", prefix, func, ext, suffix);

  free(prefix);
  free(cwd);
  return path;
}

static char *get_cfg_dot_path(args *args, const char *source,
                              const char *func) {
  return get_c_dot_path(args, source, func, "cfg");
}

static char *get_cg_dot_path(args *args, const char *source, const char *func) {
  return get_c_dot_path(args, source, func, "cg");
}

static char *get_asm_path(args *args, const char *source) {
  char *path = join_paths(args->output_dir, source);
  return path;
}

static int has_subroutine(args *args, const char *subroutine) {
  list_chars_it it = list_chars_find(args->cg_subroutines, subroutine);
  return !END(it);
}

static void print_exception(args *args, exception *exc) {
  UNUSED(args);
  if (!exc) {
    return;
  }
  char *msg = exception_str(exc, isatty(STDERR_FILENO));
  fprintf(stderr, "%s\n", msg);
  free(msg);
}

static void print_exceptions(args *args, list_exception *exceptions) {
  UNUSED(args);
  if (!exceptions) {
    return;
  }
  for (list_exception_it it = list_exception_begin(exceptions); !END(it);
       NEXT(it)) {
    exception *exc = GET(it);
    print_exception(args, exc);
  }
}

static void write_dot(args *args, const dot_string *dot, const char *path) {
  if (args->tee) {
    printf("%s\n", dot->chars);
  }
  ensure_dir_exist(path);
  if (path) {
    FILE *path_fd = fopen(path, "wb");
    fwrite(dot->chars, dot->len, 1, path_fd);
    fclose(path_fd);
  }
}

static void write_asm(args *args, const char *data, const char *path) {
  if (args->tee) {
    printf("%s\n", data);
  }
  ensure_dir_exist(path);
  if (path) {
    FILE *path_fd = fopen(path, "wb");
    fwrite(data, strlen(data), 1, path_fd);
    fclose(path_fd);
  }
}

static int execute_ok(args *args) {
  return (!args->code || args->ignore_errors);
}

static int execute(args *args) {
  list_ast *asts = list_ast_new();

  // stage: tokenize and parse to AST
  for (list_chars_it it = list_chars_begin(args->input_files); !END(it);
       NEXT(it)) {
    const char *path   = GET(it);
    char       *source = read_file(path);

    ast *ast = ast_build(path, source);

    if (list_exception_count_by_level(ast_lexer_exceptions(ast),
                                      EXCEPTION_LEVEL_ERROR) ||
        list_exception_count_by_level(ast_parser_exceptions(ast),
                                      EXCEPTION_LEVEL_ERROR)) {
      args->code = 1;
    }
    print_exceptions(args, ast_lexer_exceptions(ast));
    print_exceptions(args, ast_parser_exceptions(ast));

    list_ast_push_back(asts, ast);
  }

  // stage: dot ast
  if (execute_ok(args) && args->ast) {
    for (list_ast_it it = list_ast_begin(asts); !END(it); NEXT(it)) {
      ast        *ast  = GET(it);
      dot_string *dot  = dot_ast(ast);
      char       *path = get_ast_dot_path(args, ast);
      write_dot(args, dot, path);
      free(path);
      dot_string_free(dot);
    }
  }

  // stage: build hir + handle exceptions
  hir          *hir              = NULL;
  symbol_table *hir_symbol_table = NULL;
  type_table   *hir_type_table   = NULL;

  if (execute_ok(args)) {
    hir_build_result result = hir_build(asts, args->ignore_errors);
    hir                     = result.hir;
    hir_symbol_table        = result.symbol_table;
    hir_type_table          = result.type_table;

    if (list_exception_count_by_level(result.exceptions,
                                      EXCEPTION_LEVEL_ERROR)) {
      args->code = 1;
    }
    print_exceptions(args, result.exceptions);
    list_exception_free(result.exceptions);
  }

  // stage: print hir tree
  if (execute_ok(args) && args->hir_tree) {
    char *tree = hir_tree_str(hir);
    fprintf(stdout, "HIR TREE:\n%s\n", tree);
    if (tree) {
      free(tree);
    }
  }

  // stage: print type table
  if (execute_ok(args) && args->hir_types) {
    char *table = type_table_str(hir_type_table, "HIR TYPE TABLE");
    fprintf(stdout, "%s", table);
    if (table) {
      free(table);
    }
  }

  // stage: print symbol table
  if (execute_ok(args) && args->hir_symbols) {
    char *table = symbol_table_str(hir_symbol_table, "HIR SYMBOL TABLE");
    fprintf(stdout, "%s", table);
    if (table) {
      free(table);
    }
  }

  mir *mir = NULL;

  if (execute_ok(args)) {
    mir_build_result result =
        mir_build(hir, hir_symbol_table, hir_type_table, args->ignore_errors);
    mir = result.mir;

    if (list_exception_count_by_level(result.exceptions,
                                      EXCEPTION_LEVEL_ERROR)) {
      args->code = 1;
    }
    print_exceptions(args, result.exceptions);
    list_exception_free(result.exceptions);
  }

  // stage: print mir
  if (execute_ok(args) && args->mir) {
    char *mir_s = mir_str(mir);
    fprintf(stdout, "MIR:\n%s\n", mir_s);
    if (mir_s) {
      free(mir_s);
    }
  }

  // stage: dot cfg
  if (args->cfg && (!args->code || args->ignore_errors)) {
    for (list_mir_subroutine_it it =
             list_mir_subroutine_begin(mir->defined_subs);
         !END(it); NEXT(it)) {
      const mir_subroutine *sub      = GET(it);
      const char           *sub_name = sub->symbol_ref->name;
      const char           *source   = sub->symbol_ref->span->source_ref;

      dot_string *dot = dot_cfg(mir, hir, hir_type_table, hir_symbol_table, sub,
                                args->cfg_add_expr);
      char       *output = get_cfg_dot_path(args, source, sub_name);
      write_dot(args, dot, output);
      free(output);
      dot_string_free(dot);
    }
  }

  // stage: dot cg
  if (args->cg && (!args->code || args->ignore_errors)) {
    for (list_mir_subroutine_it it =
             list_mir_subroutine_begin(mir->defined_subs);
         !END(it); NEXT(it)) {
      const mir_subroutine *sub      = GET(it);
      const char           *sub_name = sub->symbol_ref->name;
      const char           *source   = sub->symbol_ref->span->source_ref;

      if (has_subroutine(args, sub_name)) {
        dot_string *dot    = dot_cg(mir, hir_type_table, hir_symbol_table, sub);
        char       *output = get_cg_dot_path(args, source, sub_name);
        write_dot(args, dot, output);
        free(output);
        dot_string_free(dot);
      }
    }
  }

  cg_x86_64 *code = NULL;

  // stage: build x86_64 structs
  if (!args->code || args->ignore_errors) {
    cg_x86_64_build_result result = cg_x86_64_build(mir, args->ignore_errors);
    code                          = result.code;

    if (list_exception_count_by_level(result.exceptions,
                                      EXCEPTION_LEVEL_ERROR)) {
      args->code = 1;
    }
    print_exceptions(args, result.exceptions);
    list_exception_free(result.exceptions);
  }

  // stage: emit x86_64 assembly code
  if (!args->code || args->ignore_errors) {
    cg_x86_64_emit_result result =
        cg_x86_64_emit(code, CG_X86_64_EMIT_FORMAT_GAS);

    if (list_exception_count_by_level(result.exceptions,
                                      EXCEPTION_LEVEL_ERROR)) {
      args->code = 1;
    } else {
      char *path = get_asm_path(args, args->output_file);
      write_asm(args, result.asm_str, path);
      free(path);
      free(result.asm_str);
    }
    print_exceptions(args, result.exceptions);
    list_exception_free(result.exceptions);
  }

  cg_x86_64_free(code);

  mir_free(mir);

  hir_free(hir);
  symbol_table_free(hir_symbol_table);
  type_table_free(hir_type_table);

  list_ast_free(asts);

  return args->code && !args->ignore_errors;
}

int main(int argc, char *argv[]) {
  int code;

  args *args = args_new();
  parse(args, argc, argv);

  if (args->help) {
    usage(args);
  }

  if (!args->help && list_chars_size(args->input_files) == 0) {
    printf("%s: no input files specified\n", args->prog_name);
    args->code = -1;
  }

  if (!args->help && args->code == 0) {
    args->code = execute(args);
  }

  code = args->code;

  args_free(args);

  return code;
}
