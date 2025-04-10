#include "ctx.h"
#include "util/macro.h"

#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static void ctx_tg_brkpt_fix_regs(struct user_regs_struct *regs_p) {
  regs_p->rip -= 1;
}

// NOTE: can optimize place calls
static ctx_target_state ctx_tg_brkpt_fix(ctx *self) {
  struct user_regs_struct regs;
  ctx_tg_getregs(self, &regs);
  ctx_tg_brkpt_fix_regs(&regs);
  ctx_tg_setregs(self, &regs);
  return ctx_tg_get_state(self);
}

int ctx_tg_is_running(ctx *self) {
  assert(self->s_target != TARGET_ERROR);
  switch (self->s_target) {
    case TARGET_BRKPT:
    case TARGET_TRAPPED:
      return 1;
    default:
      return 0;
  }
}

ctx_target_state ctx_tg_get_state(ctx *self) {
  // debug("target state: %s", state_target_enum_str(self->s_target));
  assert(self->s_target != TARGET_ERROR);
  return self->s_target;
}

ctx_target_state ctx_tg_load(ctx *self, uint64_t abfd_asymbols_count,
                             asymbol **abfd_asymbols, bfd *abfd, char *fname) {
  if (ctx_tg_is_running(self)) {
    ctx_tg_kill(self, SIGKILL);
  }

  self->abfd_asymbols_count = abfd_asymbols_count;
  if (self->abfd_asymbols) {
    free(self->abfd_asymbols);
  }
  self->abfd_asymbols = abfd_asymbols;
  if (self->abfd) {
    bfd_close(self->abfd);
  }
  self->abfd = abfd;
  if (self->fname) {
    free(self->fname);
  }
  self->fname    = fname;
  self->s_target = TARGET_NOT_RUNNING;

  ctx_dbg_deinit(self);
  ctx_dbg_init(self);

  return ctx_tg_get_state(self);
}

ctx_target_state ctx_tg_wait(ctx *self) {
  int status;
  int wres = waitpid(self->pid, &status, 0);
  if (wres == -1) {
    return TARGET_ERROR;
  } else if (WIFEXITED(status) || WIFSIGNALED(status)) {
    self->pid      = 0;
    self->s_target = TARGET_NOT_RUNNING;
  } else if (WIFSTOPPED(status) && WSTOPSIG(status) == SIGTRAP) {
    siginfo_t siginfo;
    if (ptrace(PTRACE_GETSIGINFO, self->pid, NULL, &siginfo) == 0) {
      if (siginfo.si_code == SI_KERNEL) {
        self->s_target = TARGET_BRKPT;
        ctx_tg_brkpt_fix(self);
      } else {
        self->s_target = TARGET_TRAPPED;
      }
    }
  }
  return ctx_tg_get_state(self);
}

ctx_target_state ctx_tg_kill(ctx *self, int signal) {
  if (kill(self->pid, signal) == -1) {
    return TARGET_ERROR;
  } else {
    return ctx_tg_wait(self);
  }
}

ctx_target_state ctx_tg_continue(ctx *self) {
  // to handle breakpoints
  ctx_tg_stepi(self);

  if (ptrace(PTRACE_CONT, self->pid, NULL, NULL) == -1) {
    return TARGET_ERROR;
  }
  return ctx_tg_wait(self);
}

// already handles breakpoints
ctx_target_state ctx_tg_stepi(ctx *self) {
  struct user_regs_struct regs;
  ctx_target_state        state_before;
  ctx_target_state        state;
  ctx_breakpoint         *b = NULL;

  if ((state_before = ctx_tg_get_state(self)) == TARGET_BRKPT) {
    ctx_tg_getregs(self, &regs);

    hashset_ctx_breakpoint_it it = hashset_ctx_breakpoint_find(
        self->breaks, &(ctx_breakpoint){.address = regs.rip});

    // in case of implicit breakpoints (handled not by ctx->breaks)
    if (!END(it)) {
      b = GET(it);
      ctx_bp_remove(self, b);
    }
  }

  if (ptrace(PTRACE_SINGLESTEP, self->pid, NULL, NULL) == -1) {
    return TARGET_ERROR;
  }
  state = ctx_tg_wait(self);

  if (b) {
    ctx_bp_place(self, b);
  }

  return state;
}

// NOTE: assuming address is aligned to instructions (to check breakpoints)
ctx_target_state ctx_tg_peekdata(ctx *self, uint64_t address,
                                 uint64_t *data_p) {
  uint64_t data;

  data  = ptrace(PTRACE_PEEKDATA, self->pid, address, NULL);
  errno = 0;
  if (errno) {
    return TARGET_ERROR;
  }

  hashset_ctx_breakpoint_it it = hashset_ctx_breakpoint_find(
      self->breaks, &(ctx_breakpoint){.address = address});

  if (!END(it)) {
    ctx_breakpoint *b = GET(it);
    if (b->is_placed) {
      data = CTX_WITH_STORED(data, b->data);
    }
  }

  *data_p = data;
  return ctx_tg_get_state(self);
}

// NOTE: assuming address is aligned to instructions (to check breakpoints)
ctx_target_state ctx_tg_pokedata(ctx *self, uint64_t address, uint64_t data) {
  hashset_ctx_breakpoint_it b_it = hashset_ctx_breakpoint_find(
      self->breaks, &(ctx_breakpoint){.address = address});

  if (!END(b_it)) {
    ctx_breakpoint *b = GET(b_it);
    if (b->is_placed) {
      b->data = CTX_STORE(data);
      data    = CTX_WITH_INT3(data);
    }
  }

  if (ptrace(PTRACE_POKEDATA, self->pid, address, data) == -1) {
    return TARGET_ERROR;
  }

  return ctx_tg_get_state(self);
}

ctx_target_state ctx_tg_run(ctx *self, char **argv) {
  pid_t pid = fork();
  if (pid == -1) {
    return TARGET_ERROR;
  } else if (pid == 0) {
    if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) == -1) {
      perror("ptrace traceme");
      exit(1);
    }

    execvp(self->fname, argv);
    // on error
    perror("execvp");
    exit(1);
  } else {
    self->pid = pid;
    return ctx_tg_wait(self);
  }
  return ctx_tg_get_state(self);
}

ctx_target_state ctx_tg_getregs(ctx *self, struct user_regs_struct *regs) {
  if (ptrace(PTRACE_GETREGS, self->pid, NULL, regs)) {
    return TARGET_ERROR;
  }
  return ctx_tg_get_state(self);
}

ctx_target_state ctx_tg_setregs(ctx *self, struct user_regs_struct *regs) {
  if (ptrace(PTRACE_SETREGS, self->pid, NULL, regs)) {
    return TARGET_ERROR;
  }
  return ctx_tg_get_state(self);
}
