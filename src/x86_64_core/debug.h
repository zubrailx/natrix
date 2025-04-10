#pragma once

#include <stdint.h>

/*
.u_debug_info
header:
- uint64_t subroutines_cnt
- uint64_t files_cnt
content:
- subroutines:
  - subroutine:
    - uint64_t address_start
    - uint64_t address_end
    - uint64_t debug_str_id
    - uint64_t params_cnt
    - uint64_t vars_cnt
    - params:
      - param:
        - uint64_t rbp_offset
        - uint64_t debug_str_id
    - vars:
      - var:
        - uint64_t rbp_offset
        - uint64_t debug_str_id
- files:
  - file:
    - uint64_t debug_str_id
  - file:
    - uint64_t debug_str_id
  - file:
    - uint64_t debug_str_id

.u_debug_line
header:
- uint64_t cnt
content:
- [entries: uint64_t address, uint64_t file_id, uint64_t line]
- [entries: uint64_t address, uint64_t file_id, uint64_t line]
- [entries: uint64_t address, uint64_t file_id, uint64_t line]

.u_debug_str
header:
- uint64_t cnt
content:
  - <string1>
  - <string2>
  - <string3>
 */

// debug_info
typedef struct __attribute__((packed)) x86_64_debug_info_struct {
  uint64_t subroutines_cnt;
  uint64_t files_cnt;
  uint8_t  data[0];
} x86_64_debug_info;

typedef struct __attribute__((packed)) x86_64_debug_info_sub_struct {
  uint64_t address_start;
  uint64_t address_end;
  uint64_t debug_str_id;
  uint64_t params_cnt;
  uint64_t vars_cnt;
  uint8_t  data[0];
} x86_64_debug_info_sub;

typedef struct __attribute__((packed)) x86_64_debug_info_param_struct {
  uint64_t rbp_offset;
  uint64_t debug_str_id;
} x86_64_debug_info_param;

typedef struct __attribute__((packed)) x86_64_debug_info_var_struct {
  uint64_t rbp_offset;
  uint64_t debug_str_id;
} x86_64_debug_info_var;

typedef struct __attribute__((packed)) x86_64_debug_info_file_struct {
  uint64_t debug_str_id;
} x86_64_debug_info_file;

// debug_str
typedef struct __attribute__((packed)) x86_64_debug_str_struct {
  uint64_t cnt;
  uint64_t data[0];
} x86_64_debug_str;

typedef struct __attribute__((packed)) x86_64_debug_str_str_struct {
  uint8_t data[0];
} x86_64_debug_str_str;

// debug_line
typedef struct __attribute__((packed)) x86_64_debug_line_struct {
  uint64_t cnt;
  uint8_t  data[0];
} x86_64_debug_line;

typedef struct __attribute__((packed)) x86_64_debug_line_entry_struct {
  uint64_t address;
  uint64_t file_id;
  uint64_t line;
} x86_64_debug_line_entry;
