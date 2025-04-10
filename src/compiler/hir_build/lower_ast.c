#include "lower_ast.h"

#include "compiler/antlr3/tree/HirBuilder.h"
#include "util/macro.h"

static void hir_dispay_error(pANTLR3_BASE_RECOGNIZER recognizer,
                             pANTLR3_UINT8          *token) {
  UNUSED(token);

  hir_lower_ast_ctx *ctx = recognizer->state->userp;
  exception         *exc = NULL;

  pANTLR3_STRING    ttext;
  pANTLR3_BASE_TREE theBaseTree;

  exception_helper exh;
  exh.level   = EXCEPTION_LEVEL_ERROR;
  exh.stream  = (char *)ctx->ast_cur_ref->name_ref;
  exh.type    = EXCEPTION_TREE;
  exh.subtype = EXCEPTION_TREE_UNEXPECTED_NODE;
  exh.line    = 0;
  exh.offset  = 0;
  exh.name    = recognizer->state->exception->message;

  switch (recognizer->type) {
    case ANTLR3_TYPE_TREE_PARSER:
      theBaseTree = (pANTLR3_BASE_TREE)(recognizer->state->exception->token);
      ttext       = theBaseTree->toStringTree(theBaseTree);

      if (theBaseTree != NULL) {
        exc = exception_new_from_helper(
            &exh, "at offset %d near %s",
            theBaseTree->getCharPositionInLine(theBaseTree), ttext->chars);
      }
      break;
  }

  if (!exc) {
    return;
  }

  list_exception_push_back(ctx->exceptions, exc);
}

hir_lower_ast_result hir_lower_ast(const list_ast *asts) {
  hir_lower_ast_result result = {
      .hir        = hir_new(),
      .exceptions = list_exception_new(),
  };

  hir_lower_ast_ctx ctx = {
      .hir_ref    = result.hir,
      .exceptions = result.exceptions,
  };

  for (list_ast_it it = list_ast_begin(asts); !END(it); NEXT(it)) {
    const ast *ast  = GET(it);
    ctx.ast_cur_ref = ast;

    ANTLR3_COMMON_TREE_NODE_STREAM *nodes =
        antlr3CommonTreeNodeStreamNewTree(ast->tree, ANTLR3_SIZE_HINT);
    HirBuilder *builder = HirBuilderNew(nodes);

    builder->pTreeParser->rec->state->userp            = &ctx;
    builder->pTreeParser->rec->displayRecognitionError = hir_dispay_error;

    builder->source(builder);

    builder->free(builder);
    nodes->free(nodes);
  }

  return result;
}
