#pragma once

#include "compiler/antlr3/rec/MainLexer.h"
#include "compiler/exception/list.h"
#include <antlr3interfaces.h>

typedef struct lexer_userp_struct {
  list_exception *exceptions;
  const char     *name_ref;
} lexer_userp;

typedef struct lexer_struct {
  ANTLR3_INPUT_STREAM *stream;
  MainLexer           *lexer;
  list_exception      *exceptions;
  lexer_userp          userp;
} lexer;

typedef struct lexer_token_stream_struct {
  pANTLR3_COMMON_TOKEN_STREAM tokens;
  const char                 *name_ref;
} lexer_token_stream;

lexer *lexer_new(const char *data_ref, size_t len, const char *name_ref);
void   lexer_free(lexer *);

lexer_token_stream *lexer_token_stream_new(lexer *self);
void                lexer_token_stream_free(lexer_token_stream *self);
