#pragma once

#define ANTLR3_USERP() (ctx->pTreeParser->rec->state->userp)

#define ANTLR3_CHARS(id)                                                       \
  ((const char *)id->getText(id)->to8(id->getText(id))->chars)

#define ANTLR3_SIZE(id)                                                        \
  ((size_t)id->children ? id->children->size(id->children) : 0)
