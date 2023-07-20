#ifndef DECODERS_H
#define DECODERS_H

#include <stddef.h>
#include "parser.h"

typedef struct JSONPath {
  enum { JSON_FIELD, JSON_INDEX } tag;
  union {
    struct JSON_FIELD { char *fieldName; } JSON_FIELD;
    struct JSON_INDEX { int index;       } JSON_INDEX;
  } data;
} JSONPath;

typedef struct DecoderError {
  JSONPath *path;
  int depth;
  int pathCapacity;
  char *errorMsg;
} DecoderError;

// TODO This takes up a lot of space, could we make it more compact?
typedef struct DecoderState {
  JSONNode *currentNode;
  DecoderError error;
} DecoderState;

typedef bool(*decodeFun)(DecoderState*, void*);

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

typedef struct DecodeResult {
  bool success;
  DecoderError error;
  int depth;
} DecodeResult;

bool decodeInt(DecoderState *state, void *dest);
bool decodeFloat(DecoderState *state, void *dest);
bool decodeString(DecoderState *state, void *dest);
FieldDef makeField(char *name, void *dest, decodeFun decoder);
FieldDef makeListField(char *name, void *dest, int *lengthDest, size_t size, decodeFun decoder);
bool decodeFields(DecoderState *state, int count, ...);
bool decodeList(DecoderState *state, void *dest, int *length, size_t size, decodeFun decoder);
void printDecoderError(DecoderError err);
char *buildDecoderError(DecoderError err);
void DecodeError_free(DecoderError err);

// Does not take ownerhip of the `input`, caller must deallocate. On success, deallocates the error
// field. On failure, ownership of the `DecodeError` is transferred to the caller who must
// deallocate it using `DecodeError_free`.
DecodeResult decode(char *input, void *dest, decodeFun decoder);

#endif
