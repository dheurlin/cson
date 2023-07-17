#ifndef DECODERS_H
#define DECODERS_H

#include "parser.h"
#include <stddef.h>

typedef struct DecoderState {
  JSONNode *currentNode;
} DecoderState;

typedef void(*decodeFun)(DecoderState*, void*);

typedef struct FieldDef {
  char* name;
  void *dest;
  decodeFun decoder;
} FieldDef;

void decodeInt(DecoderState *state, void *dest);
void decodeFloat(DecoderState *state, void *dest);
void decodeString(DecoderState *state, void *dest);
FieldDef makeField(char *name, void *dest, decodeFun decoder);
void decodeFields(DecoderState *state, int count, ...);
void decodeList(DecoderState *state, size_t size, void *dest, int *length, decodeFun decoder);
void decode(char *input, void *dest, decodeFun decoder);

#endif
