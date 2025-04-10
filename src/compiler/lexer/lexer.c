#include "lexer.h"

#include "compiler/antlr3/rec/MainLexer.h"
#include "compiler/exception/exception.h"
#include "util/macro.h"
#include <antlr3.h>
#include <stdlib.h>

static void lexer_display_error(pANTLR3_BASE_RECOGNIZER recognizer,
                                pANTLR3_UINT8          *token) {
  UNUSED(token);

  pANTLR3_EXCEPTION ex;
  exception_helper  exh;
  exception        *exc;
  lexer_userp      *userp;

  if (recognizer->type != ANTLR3_TYPE_LEXER) {
    return;
  }

  ex = recognizer->state->exception;

  exh.level  = EXCEPTION_LEVEL_ERROR;
  exh.type   = EXCEPTION_LEXER;
  exh.line   = ex->line;
  exh.name   = ex->message;
  exh.offset = recognizer->state->exception->charPositionInLine;

  userp      = recognizer->state->userp;
  exh.stream = (char *)userp->name_ref;

  if (recognizer->state) {
    exh.subtype = EXCEPTION_LEXER_MATCHING;
    exc         = exception_new_from_helper(
        &exh, "error matching from [Line: %d, LinePos: %d]",
        recognizer->state->tokenStartLine,
        recognizer->state->tokenStartCharPositionInLine);

  } else {
    exh.subtype = EXCEPTION_LEXER_UNKNOWN;
    exc         = exception_new_from_helper(
        &exh, "error, dumb implementation, may help [Line: %d, LinePos: %d]",
        exh.line, exh.offset);
  }

  list_exception_push_back(userp->exceptions, exc);
}

lexer *lexer_new(const char *data_ref, size_t len, const char *name_ref) {
  lexer *self  = MALLOC(lexer);
  self->stream = antlr3StringStreamNew((pANTLR3_UINT8)data_ref, ANTLR3_ENC_UTF8,
                                       len, (pANTLR3_UINT8)name_ref);
  self->lexer  = MainLexerNew(self->stream);
  self->exceptions = list_exception_new();
  self->userp =
      (lexer_userp){.exceptions = self->exceptions, .name_ref = name_ref};

  self->lexer->pLexer->rec->state->userp            = &self->userp;
  self->lexer->pLexer->rec->displayRecognitionError = lexer_display_error;

  return self;
}

void lexer_free(lexer *self) {
  if (self) {
    list_exception_free(self->exceptions);
    self->exceptions = NULL;
    self->lexer->free(self->lexer);
    self->lexer = NULL;
    self->stream->free(self->stream);
    self->stream = NULL;
    free(self);
  }
}

lexer_token_stream *lexer_token_stream_new(lexer *self) {
  lexer_token_stream *stream = MALLOC(lexer_token_stream);
  stream->tokens   = antlr3CommonTokenStreamSourceNew(ANTLR3_SIZE_HINT,
                                                      TOKENSOURCE(self->lexer));
  stream->name_ref = self->userp.name_ref;
  return stream;
}

void lexer_token_stream_free(lexer_token_stream *self) {
  if (self) {
    self->tokens->free(self->tokens);
    self->tokens = NULL;
    free(self);
  }
}
