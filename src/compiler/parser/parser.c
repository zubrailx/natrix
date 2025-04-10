#include "parser.h"
#include "compiler/exception/exception.h"
#include "compiler/exception/list.h"
#include "util/macro.h"
#include <stdlib.h>

static void parser_display_error(pANTLR3_BASE_RECOGNIZER recognizer,
                                 pANTLR3_UINT8          *token) {
  UNUSED(token);

  pANTLR3_EXCEPTION    ex;
  exception           *exc;
  exception_helper     exh;
  pANTLR3_COMMON_TOKEN theToken;
  pANTLR3_STRING       ttext;
  parser_userp        *userp;

  if (recognizer->type != ANTLR3_TYPE_PARSER) {
    return;
  }

  ex         = recognizer->state->exception;
  exh.level  = EXCEPTION_LEVEL_ERROR;
  exh.type   = EXCEPTION_PARSER;
  exh.line   = ex->line;
  exh.name   = ex->message;
  exh.offset = recognizer->state->exception->charPositionInLine;

  theToken = (pANTLR3_COMMON_TOKEN)(recognizer->state->exception->token);

  userp      = recognizer->state->userp;
  exh.stream = (char *)userp->name_ref;

  ttext = theToken->toString(theToken);

  if (theToken->type == ANTLR3_TOKEN_EOF) {
    exh.subtype = EXCEPTION_PARSER_EOF;
    exc         = exception_new_from_helper(&exh, "at <EOF>");
  } else {
    exh.subtype = EXCEPTION_PARSER_GENERIC;
    exc         = exception_new_from_helper(&exh, "near %s",
                                    ttext == NULL ? (pANTLR3_UINT8) "<empty>"
                                                          : ttext->chars);
  }

  list_exception_push_back(userp->exceptions, exc);
}

parser *parser_new(const lexer_token_stream *token_stream_ref) {
  parser *self           = MALLOC(parser);
  self->token_stream_ref = token_stream_ref;
  self->parser           = MainParserNew(token_stream_ref->tokens);
  self->exceptions       = list_exception_new();
  self->userp            = (parser_userp){.exceptions = self->exceptions,
                                          .name_ref   = token_stream_ref->name_ref};

  self->parser->pParser->rec->state->userp            = &self->userp;
  self->parser->pParser->rec->displayRecognitionError = parser_display_error;
  return self;
}

void parser_free(parser *self) {
  if (self) {
    list_exception_free(self->exceptions);
    self->exceptions = NULL;
    self->parser->free(self->parser);
    self->parser = NULL;
    free(self);
  }
}

ANTLR3_BASE_TREE *parser_parse(parser *self) {
  MainParser_source_return result = self->parser->source(self->parser);
  return result.tree;
}
