#include "str.h"
#include "util/log.h"

const char *state_target_enum_str(ctx_target_state state) {
  switch (state) {
    case TARGET_NO_TARGET:
      return "NO_TARGET";
    case TARGET_NOT_RUNNING:
      return "NOT_RUNNING";
    case TARGET_ERROR:
      return "ERROR";
    case TARGET_BRKPT:
      return "BRKPT";
    case TARGET_TRAPPED:
      return "TRAPPED";
    case TARGET_UNKNOWN:
      break;
  }
  warn("unknown target state %d", state);
  return "UNKNOWN";
}

const char *state_debugger_enum_str(ctx_debugger_state state) {
  switch (state) {
    case DEBUGGER_IDLE:
      return "IDLE";
    case DEBUGGER_PROCESSING:
      return "PROCESSING";
    case DEBUGGER_ERROR:
      return "ERROR";
    case DEBUGGER_EXITING:
      return "EXITING";
    case DEBUGGER_UNKNOWN:
      break;
  }
  warn("unknown debugger state %d", state);
  return "UNKNOWN";
}
