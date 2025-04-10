#include "dot.h"

#include "util/macro.h"

#include "util/container_util.h"
#include "util/list.h"
#include "util/string.h"

#include <antlr3.h>
#include <antlr3string.h>
#include <assert.h>

struct dot_ctx {
  const char *graph;
  const char *node_style;

  size_t id;

  list *list_nodes; // key - id, value - label
  list *list_edges; // key - id_u, value - id_v

  void (*free)(struct dot_ctx *);
};

static void dot_ctx_free(struct dot_ctx *self) {
  free((void *)self->graph);
  free((void *)self->node_style);

  list_free(self->list_nodes);
  list_free(self->list_edges);

  free(self);
}

static struct dot_ctx *dot_ctx_new(const char *graph, const char *node_style) {
  struct dot_ctx *ctx = MALLOC(struct dot_ctx);

  ctx->graph      = strdup(graph);
  ctx->node_style = strdup(node_style);

  ctx->id = 0;

  ctx->list_nodes =
      list_new(container_cmp_chars_chars, container_new_chars_chars,
               container_delete_chars_chars);
  ctx->list_edges =
      list_new(container_cmp_chars_chars, container_new_chars_chars,
               container_delete_chars_chars);

  ctx->free = dot_ctx_free;

  return ctx;
}

static size_t dot_len(struct dot_ctx *ctx) {
  size_t len = 0;

  len += strlen(ctx->graph);
  len += strlen("{\n");
  len += strlen("node []\n");
  len += strlen(ctx->node_style);

  for (list_node *cur = ctx->list_nodes->head; cur; cur = cur->next) {
    chars_chars *data = cur->data;
    len += strlen(data->key);
    len += strlen("[label=]\n");
    len += strlen(data->value);
  }

  for (list_node *cur = ctx->list_edges->head; cur; cur = cur->next) {
    chars_chars *data = cur->data;
    len += strlen(data->key);
    len += strlen(" -> \n");
    len += strlen(data->value);
  }

  len += strlen("}");

  return len;
}

static void dot_generate(struct dot_ctx *ctx, char *str) {
  str[0] = '\0';

  char *end = stpcpy(str, ctx->graph);
  end       = stpcpy(end, "{\n");
  end       = stpcpy(end, "node [");
  end       = stpcpy(end, ctx->node_style);
  end       = stpcpy(end, "]\n");

  for (list_node *cur = ctx->list_nodes->head; cur; cur = cur->next) {
    chars_chars *data = cur->data;
    end               = stpcpy(end, data->key);
    end               = stpcpy(end, "[label=");
    end               = stpcpy(end, data->value);
    end               = stpcpy(end, "]\n");
  }

  for (list_node *cur = ctx->list_edges->head; cur; cur = cur->next) {
    chars_chars *data = cur->data;
    end               = stpcpy(end, data->key);
    end               = stpcpy(end, " -> ");
    end               = stpcpy(end, data->value);
    end               = stpcpy(end, "\n");
  }

  end = stpcpy(end, "}");
}

static size_t dot_dfs(struct dot_ctx *ctx, pANTLR3_BASE_TREE root) {
  if (!root) {
    return 0;
  }

  char                 label[256];
  pANTLR3_COMMON_TOKEN root_token = root->getToken(root);
  if (root_token == NULL) {
    printf("root is nil\n");
    snprintf(label, 255,
             "< <FONT POINT-SIZE=\"16\"><b>&lt;empty&gt;</b></FONT> >");
  } else {
    char *str = (char *)root_token->getText(root_token)->chars;

    char *escaped_str = escape_html(str);

    snprintf(label, 255,
             "< <FONT POINT-SIZE=\"16\"><b>%s</b></FONT><br/><FONT "
             "POINT-SIZE=\"12\">Line: %d, Pos: %d</FONT> >",
             escaped_str, root_token->line,
             root_token->getCharPositionInLine(root_token));

    free(escaped_str);
  }

  size_t id = ++ctx->id;
  char   id_s[32];
  sprintf(id_s, "id_%zu", id);

  chars_chars data = {.key = id_s, .value = label};

  list_it it = list_find(ctx->list_nodes, &data);
  list_insert(ctx->list_nodes, it, &data);

  for (size_t i = 0; i < root->getChildCount(root); ++i) {
    pANTLR3_BASE_TREE child    = root->getChild(root, i);
    size_t            child_id = dot_dfs(ctx, child);

    if (child_id != 0) {
      char child_id_s[32];
      sprintf(child_id_s, "id_%zu", child_id);

      chars_chars data = {.key = id_s, .value = child_id_s};

      list_push_back(ctx->list_edges, &data);
    }
  }

  return id;
}

static struct dot_ctx *dot_new(ANTLR3_BASE_TREE *tree) {
  struct dot_ctx *ctx =
      dot_ctx_new("strict digraph AST", "shape=rect, style=filled");

  dot_dfs(ctx, tree);

  return ctx;
}

dot_string *dot_ast(const ast *ast) {
  pANTLR3_BASE_TREE tree = ast->tree;
  struct dot_ctx   *ctx  = dot_new(tree);

  size_t len   = dot_len(ctx);
  char  *chars = malloc(len + 1);

  dot_generate(ctx, chars);

  assert(len == strlen(chars));

  ctx->free(ctx);

  return dot_string_new(chars, len);
}
