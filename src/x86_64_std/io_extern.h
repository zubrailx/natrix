#pragma once

#include <stdint.h>

uint8_t  std_read_bool();
uint8_t  std_read_byte();
uint8_t  std_read_char();
int32_t  std_read_int();
uint32_t std_read_uint();
int64_t  std_read_long();
uint64_t std_read_ulong();
uint8_t *std_read_string();

void std_write_bool(uint8_t);
void std_write_byte(uint8_t);
void std_write_char(uint8_t);
void std_write_int(int32_t);
void std_write_uint(uint32_t);
void std_write_long(int64_t);
void std_write_ulong(uint64_t);
void std_write_string(const uint8_t *);
