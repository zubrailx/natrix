#pragma once

#include "util/list.h"

#define STRTABLE_SPACE " "
#define STRTABLE_TOPLEFT "┌"
#define STRTABLE_TOPRIGHT "┐"
#define STRTABLE_BOTLEFT "└"
#define STRTABLE_BOTRIGHT "┘"
#define STRTABLE_HORIZ "─"
#define STRTABLE_VERT "│"
#define STRTABLE_CROSS_MID "┼"
#define STRTABLE_CROSS_LEFT "├"
#define STRTABLE_CROSS_RIGHT "┤"
#define STRTABLE_CROSS_TOP "┬"
#define STRTABLE_CROSS_BOT "┴"

// uses generic list to not depend of specific list implementation (move/delete)
// * title - NULL or string. If NULL - won't add title
// * header - list of char*
// * rows - list of list char*
char *strtable_build(const char *title, const list *header, const list *rows);
