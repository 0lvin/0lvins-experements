#ifndef UTILS
#define UTILS

#include <stdio.h>
#include <sys/stat.h>
#include <sys/mount.h>

void mkdir_if_not_exists(const char *target, mode_t mode);
#endif
