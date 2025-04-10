#pragma once

char *escape_html(const char *);
int   string_all_char_in_symbols(const char *str, const char *symbols);

char *trim_end(char *str_m);
char *trim_start(char *str_m);
char *trim(char *str_m);
