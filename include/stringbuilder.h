#pragma once

#include <stddef.h>

typedef struct {
  char *contents;
  size_t capacity;
  size_t length;
} StringBuilder;

StringBuilder StringBuilder_new();
void StringBuilder_append(StringBuilder *builder, const char *format, ...);
char *StringBuilder_getString(StringBuilder *builder);

