#include "ctx.h"

#include "util/macro.h"
#include "x86_64_core/debug.h"
#include <stdio.h>

void ctx_dbg_init(ctx *self) {
  if (!self->abfd) {
    return;
  }

  ctx_debug_data *dbg = &self->debug;

  for (asection *section = self->abfd->sections; section;
       section           = section->next) {
    const char *name = bfd_section_name(section);

    // .u_debug_info
    if (!strcmp(name, ".u_debug_info")) {
      bfd_size_type size  = bfd_section_size(section);
      uint8_t      *bytes = malloc(size);
      uint8_t      *cur   = bytes;
      bfd_get_section_contents(self->abfd, section, bytes, 0, size);

      dbg->subroutines_cnt = ((x86_64_debug_info *)cur)->subroutines_cnt;
      dbg->files_cnt       = ((x86_64_debug_info *)cur)->files_cnt;
      cur += sizeof(x86_64_debug_info);

      dbg->subroutines = MALLOCN(ctx_debug_sub *, dbg->subroutines_cnt);
      for (uint64_t i = 0; i < dbg->subroutines_cnt; ++i) {
        dbg->subroutines[i] = MALLOC(ctx_debug_sub);
        ctx_debug_sub *sub  = dbg->subroutines[i];

        sub->address_start = ((x86_64_debug_info_sub *)cur)->address_start;
        sub->address_end   = ((x86_64_debug_info_sub *)cur)->address_end;

        sub->debug_str_id = ((x86_64_debug_info_sub *)cur)->debug_str_id;
        sub->params_cnt   = ((x86_64_debug_info_sub *)cur)->params_cnt;
        sub->vars_cnt     = ((x86_64_debug_info_sub *)cur)->vars_cnt;
        cur += sizeof(x86_64_debug_info_sub);

        sub->params = MALLOCN(ctx_debug_param, sub->params_cnt);
        for (uint64_t j = 0; j < sub->params_cnt; ++j) {
          ctx_debug_param *arg = sub->params + j;
          arg->rbp_offset      = ((x86_64_debug_info_param *)cur)->rbp_offset;
          arg->debug_str_id    = ((x86_64_debug_info_param *)cur)->debug_str_id;
          cur += sizeof(x86_64_debug_info_param);
        }

        sub->vars = MALLOCN(ctx_debug_var, sub->vars_cnt);
        for (uint64_t j = 0; j < sub->vars_cnt; ++j) {
          ctx_debug_var *var = sub->vars + j;
          var->rbp_offset    = ((x86_64_debug_info_var *)cur)->rbp_offset;
          var->debug_str_id  = ((x86_64_debug_info_var *)cur)->debug_str_id;
          cur += sizeof(x86_64_debug_info_var);
        }
      }

      dbg->files = MALLOCN(ctx_debug_file *, dbg->files_cnt);
      for (uint64_t i = 0; i < dbg->files_cnt; ++i) {
        dbg->files[i]        = MALLOC(ctx_debug_file);
        ctx_debug_file *file = dbg->files[i];

        file->debug_str_id = ((x86_64_debug_info_file *)cur)->debug_str_id;
        file->lines_cnt    = 0;
        file->lines        = NULL;

        cur += sizeof(x86_64_debug_info_file);
      }

      free(bytes);
      // .u_debug_line
    } else if (!strcmp(name, ".u_debug_line")) {
      bfd_size_type size  = bfd_section_size(section);
      uint8_t      *bytes = malloc(size);
      uint8_t      *cur   = bytes;
      bfd_get_section_contents(self->abfd, section, bytes, 0, size);

      dbg->lines_cnt = ((x86_64_debug_line *)cur)->cnt;
      cur += sizeof(x86_64_debug_line);

      dbg->lines = MALLOCN(ctx_debug_line, dbg->lines_cnt);
      for (uint64_t i = 0; i < dbg->lines_cnt; ++i) {
        ctx_debug_line *line = dbg->lines + i;
        line->line           = ((x86_64_debug_line_entry *)cur)->line;
        line->file_id        = ((x86_64_debug_line_entry *)cur)->file_id;
        line->address        = ((x86_64_debug_line_entry *)cur)->address;
        cur += sizeof(x86_64_debug_line_entry);
      }

      free(bytes);
      // .u_debug_str
    } else if (!strcmp(name, ".u_debug_str")) {
      bfd_size_type size  = bfd_section_size(section);
      uint8_t      *bytes = malloc(size);
      uint8_t      *cur   = bytes;
      bfd_get_section_contents(self->abfd, section, bytes, 0, size);

      dbg->strs_cnt = ((x86_64_debug_str *)cur)->cnt;
      cur += sizeof(x86_64_debug_str);

      dbg->strs = MALLOCN(char *, dbg->strs_cnt);
      for (uint64_t i = 0; i < dbg->strs_cnt; ++i) {
        uint64_t l   = strlen((const char *)cur) + 1;
        dbg->strs[i] = strdup((const char *)cur);
        cur += sizeof(char) * l;
      }
      free(bytes);
    }
  }

  // post init files lines
  for (uint64_t i = 0; i < self->debug.files_cnt; ++i) {
    ctx_debug_file *file    = self->debug.files[i];
    FILE           *file_fp = NULL;
    file_fp                 = fopen(dbg->strs[file->debug_str_id], "r");
    if (file_fp) {
      char  *line = NULL;
      size_t len  = 0;
      while (getline(&line, &len, file_fp) != -1) {
        file->lines_cnt++;
        free(line);
        line = NULL;
      }
      free(line);
      fclose(file_fp);
    }
    file_fp = fopen(dbg->strs[file->debug_str_id], "r");
    if (file_fp) {
      // increase by one to set lines from 1
      file->lines = MALLOCN(char *, ++file->lines_cnt);

      char  *line = NULL;
      size_t len  = 0;

      uint64_t li       = 0;
      file->lines[li++] = NULL;
      while (li < file->lines_cnt && getline(&line, &len, file_fp) != -1) {
        file->lines[li++] = line;
        line              = NULL;
      }
      if (line) {
        free(line);
      }
      fclose(file_fp);
    }
  }
}

void ctx_dbg_deinit(ctx *self) {
  ctx_debug_data *dbg = &self->debug;

  for (uint64_t i = 0; i < dbg->subroutines_cnt; ++i) {
    ctx_debug_sub *sub = dbg->subroutines[i];
    free(sub->params);
    free(sub->vars);
    free(sub);
  }
  free(dbg->subroutines);
  dbg->subroutines_cnt = 0;
  dbg->subroutines     = NULL;

  for (uint64_t i = 0; i < dbg->files_cnt; ++i) {
    ctx_debug_file *file = dbg->files[i];
    if (file->lines) {
      for (uint64_t j = 0; j < file->lines_cnt; ++j) {
        if (file->lines[j]) {
          free(file->lines[j]);
        }
      }
      free(file->lines);
    }
    free(file);
  }
  free(dbg->files);
  dbg->files_cnt = 0;
  dbg->files     = NULL;

  free(dbg->lines);
  dbg->lines_cnt = 0;
  dbg->lines     = NULL;

  for (uint64_t i = 0; i < dbg->strs_cnt; ++i) {
    free(dbg->strs[i]);
  }
  free(dbg->strs);
  dbg->strs_cnt = 0;
  dbg->strs     = NULL;
}

int ctx_dbg_line_by_source(ctx *self, ctx_debug_line *result) {
  for (uint64_t i = 0; i < self->debug.lines_cnt; ++i) {
    ctx_debug_line debug = self->debug.lines[i];
    if (debug.file_id == result->file_id && debug.line == result->line) {
      result->address = debug.address;
      return 0;
    }
  }
  return -1;
}

// TODO: rewrite to binary search
int ctx_dbg_line_by_address(ctx *self, ctx_debug_line *result) {
  for (uint64_t i = 0; i < self->debug.lines_cnt; ++i) {
    ctx_debug_line debug = self->debug.lines[i];
    if (debug.address == result->address) {
      result->file_id = debug.file_id;
      result->line    = debug.line;
      return 0;
    }
  }
  return -1;
}

const char *ctx_dbg_str_by_id(ctx *self, uint64_t id) {
  return self->debug.strs[id];
}

const char *ctx_dbg_file_by_id(ctx *self, uint64_t file) {
  return ctx_dbg_str_by_id(self, self->debug.files[file]->debug_str_id);
}

// TODO: rewrite to binary search
ctx_debug_sub *ctx_dbg_sub_by_address(ctx *self, uint64_t address) {
  for (uint64_t i = 0; i < self->debug.subroutines_cnt; ++i) {
    ctx_debug_sub *sub = self->debug.subroutines[i];
    if (sub->address_start <= address && sub->address_end > address) {
      return sub;
    }
  }
  return NULL;
}

void ctx_dbg_line_reset(ctx *self) {
  self->line.address = UINT64_MAX;
  self->line.line    = UINT64_MAX;
  self->line.file_id = UINT64_MAX;
}

void ctx_dbg_line_set(ctx *self, ctx_debug_line line) {
  if (line.line != UINT64_MAX && line.file_id != UINT64_MAX) {
    self->line = line;
  }
}

int ctx_dbg_line_changed(ctx *self, ctx_debug_line line) {
  if ((line.line != UINT64_MAX) && (line.file_id != UINT64_MAX)) {
    if ((line.file_id != self->line.file_id) ||
        (line.line != self->line.line)) {
      return 1;
    }
  }
  return 0;
}
