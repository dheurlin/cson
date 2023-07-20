#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "stringbuilder.h"

#define START_CAPACITY 10
#define SCALE_FACTOR 2

StringBuilder StringBuilder_new() {
  return (StringBuilder) {
    .capacity = START_CAPACITY,
    .length = 1,
    .contents = malloc(START_CAPACITY * sizeof(char)),
  };
}

void StringBuilder_resize(StringBuilder *builder) {
  size_t newCapacity = builder->capacity * SCALE_FACTOR;
  char *newContents = reallocarray(builder->contents, newCapacity, sizeof(char));
  builder->contents = newContents;
  builder->capacity = newCapacity;
}

void StringBuilder_append(StringBuilder *builder, const char *format, ...) {
  va_list args, argsCopy;
  va_start(args, format);
  va_copy(argsCopy, args); // just to be sure, since the first vnsprintf call might corrupt `args`

  size_t nbytes = vsnprintf(NULL, 0, format, args);
  size_t newLength = builder->length + nbytes;

  if (builder->capacity - builder->length < newLength) {
    StringBuilder_resize(builder);
  }
  // length - 1 to overwrite old \0, nbytes + 1 to account for new \0
  vsnprintf(&builder->contents[builder->length - 1], nbytes + 1, format, argsCopy);

  va_end(args);
  va_end(argsCopy);

  builder->length = newLength;
}

char *StringBuilder_getString(StringBuilder *builder) {
  return builder->contents;
}
