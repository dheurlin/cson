#ifndef DECODERS_H
#define DECODERS_H

#include "parser.h"
#include <stddef.h>

typedef struct DecoderState {
  JSONNode *currentNode;
} DecoderState;

typedef void(*decodeFun)(DecoderState*, void*);

typedef struct FieldDef {
  enum { NORMAL_FIELD, LIST_FIELD } type;
  union {
    struct NORMAL_FIELD {
      void *dest;
      decodeFun decoder;
    } NORMAL_FIELD;
    struct LIST_FIELD {
      void *dest;
      int *lengthDest;
      size_t size;
      decodeFun decoder;
    } LIST_FIELD;
  } data;
  char* name;
} FieldDef;

void decodeInt(DecoderState *state, void *dest);
void decodeFloat(DecoderState *state, void *dest);
void decodeString(DecoderState *state, void *dest);
FieldDef makeField(char *name, void *dest, decodeFun decoder);
FieldDef makeListField(char *name, void *dest, int *lengthDest, size_t size, decodeFun decoder);
void decodeFields(DecoderState *state, int count, ...);
void decodeList(DecoderState *state, void *dest, int *length, size_t size, decodeFun decoder);
void decode(char *input, void *dest, decodeFun decoder);

#endif
