#pragma once

char *join_paths(const char *dir, const char *file);

int   dir_create(const char *path);
int   dir_create_p(const char *dir);
char *dirname(const char *file_path);

char *read_file(const char *path);
