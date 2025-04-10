#include "ctx.h"
#include "util/log.h"
#include <errno.h>
#include <sys/ptrace.h>

int ctx_bp_place(ctx *self, ctx_breakpoint *bp) {
  if (bp->is_enabled && !bp->is_placed) {
    errno         = 0;
    uint64_t data = ptrace(PTRACE_PEEKDATA, self->pid, bp->address, NULL);
    if (errno) {
      error("errno!");
      return -1;
    }
    bp->data = CTX_STORE(data);
    data     = CTX_WITH_INT3(data);
    if (ptrace(PTRACE_POKEDATA, self->pid, bp->address, data) == -1) {
      error("ptrace!");
      return -1;
    }
    bp->is_placed = 1;
  }
  return 0;
}

int ctx_bp_remove(ctx *self, ctx_breakpoint *bp) {
  if (bp->is_enabled && bp->is_placed) {
    errno         = 0;
    uint64_t data = ptrace(PTRACE_PEEKDATA, self->pid, bp->address, NULL);
    if (errno) {
      error("errno!");
      return -1;
    }
    data = CTX_WITH_STORED(data, bp->data);
    if (ptrace(PTRACE_POKEDATA, self->pid, bp->address, data) == -1) {
      error("ptrace!");
      return -1;
    }
    bp->is_placed = 0;
  }
  return 0;
}
