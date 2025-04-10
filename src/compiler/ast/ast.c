#include "ast.h"
#include "compiler/lexer/lexer.h"
#include "compiler/parser/parser.h"
#include "util/macro.h"

static ast *ast_new(const char *name_ref, char *source, lexer *lexer,
                    lexer_token_stream *token_stream, parser *parser,
                    ANTLR3_BASE_TREE *tree) {
  ast *self          = MALLOC(ast);
  self->name_ref     = name_ref;
  self->source       = source;
  self->lexer        = lexer;
  self->token_stream = token_stream;
  self->parser       = parser;
  self->tree         = tree;
  return self;
}

void ast_free(ast *self) {
  if (self) {
    self->tree = NULL;
    parser_free(self->parser);
    lexer_token_stream_free(self->token_stream);
    lexer_free(self->lexer);
    free(self->source);
    free(self);
  }
}

// source is not const because it is consumed by (moved to) this function.
// it is done because if source changes then ast won't be valid.
ast *ast_build(const char *name, char *source) {
  lexer              *lexer        = lexer_new(source, strlen(source), name);
  lexer_token_stream *token_stream = lexer_token_stream_new(lexer);
  parser             *parser       = parser_new(token_stream);
  ANTLR3_BASE_TREE   *tree         = parser_parse(parser);

  return ast_new(name, source, lexer, token_stream, parser, tree);
}

list_exception *ast_lexer_exceptions(ast *self) {
  return self->lexer->exceptions;
}

list_exception *ast_parser_exceptions(ast *self) {
  return self->parser->exceptions;
}
