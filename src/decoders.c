#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "decoders.h"
#include "parser.h"
#include "nodelist.h"
#include "stringbuilder.h"

void setDecoderPath(DecoderError *error, int depth, JSONPath jPath);

#define allocsprintf(ptr, args...) do {\
  size_t nbytes = snprintf(NULL, 0, args) + 1;\
  char *str = malloc(nbytes);\
  snprintf(str, nbytes, args);\
  ptr = str;\
} while(0);

#define FAIL(state, args...) do {\
  allocsprintf(state->error.errorMsg, args);\
  return false;\
} while(0)

bool decodeInt(DecoderState *state, void *dest) {
  if (state->currentNode->tag != JSON_NUMBER) {
    FAIL(state, "Expecting number, got %s", nodeTagToString(state->currentNode->tag));
  }
  double num = state->currentNode->data.JSON_NUMBER.number;
  if(ceil(num) != num) {
    FAIL(state, "Expected integer, got float");
  }
  int *numDest = (int*)dest;
  *numDest = (int)num;
  return true;
}

bool decodeFloat(DecoderState *state, void *dest) {
  if (state->currentNode->tag != JSON_NUMBER) {
    FAIL(state, "Expecting number, got %s", nodeTagToString(state->currentNode->tag));
  }
  double num = state->currentNode->data.JSON_NUMBER.number;
  double *doubleDest = (double*)dest;
  *doubleDest = num;
  return true;
}

bool decodeString(DecoderState *state, void *dest) {
  if (state->currentNode->tag != JSON_STRING) {
    FAIL(state, "Expecting string, got %s", nodeTagToString(state->currentNode->tag));
  }
  char *str = state->currentNode->data.JSON_STRING.string;
  char **strDest = (char**)dest;
  *strDest = strdup(str);
  return true;
}

JSONNode *findField(NodeList *list, char *name) {
  for (int i = 0; i < list->length; i++) {
    JSONNode *node = &list->items[i];
    if (strcmp(node->fieldName, name) == 0) {
      return node;
    }
  }
  return NULL;
}

bool decodeField(DecoderState *state, FieldDef field) {
  JSONNode *current = state->currentNode;
  JSONNode *node = findField(current->data.JSON_OBJECT.nodes, field.name);

  setDecoderPath(&state->error, state->error.depth++, (JSONPath) {
    .tag = JSON_FIELD,
    .data = { .JSON_FIELD = { .fieldName = field.name } }
  });

  if (node == NULL) {
    FAIL(state, "No field with name \"%s\" was found", field.name);
  }

  state->currentNode = node;

  switch (field.type) {
    case NORMAL_FIELD: {
      struct NORMAL_FIELD data = field.data.NORMAL_FIELD;
      bool result = data.decoder(state, data.dest);
      if (!result) {
        return false;
      }
      break;
    }

    case LIST_FIELD: {
      struct LIST_FIELD data = field.data.LIST_FIELD;
      bool result = decodeList(state, data.dest, data.lengthDest, data.size, data.decoder);
      if (!result) {
        return false;
      }
      break;
    }
  }

  state->currentNode = current;
  state->error.depth--;
  return true;
}

FieldDef makeField(char *name, void *dest, decodeFun decoder) {
  return (FieldDef) {
    .type = NORMAL_FIELD,
    .name = name,
    .data = { .NORMAL_FIELD = { 
      .dest = dest,
      .decoder = decoder,
    } }
  };
}

FieldDef makeListField(char *name, void *dest, int *lengthDest, size_t size, decodeFun decoder) {
  return (FieldDef) {
    .type = LIST_FIELD,
    .name = name,
    .data = { .LIST_FIELD = {
      .dest = dest,
      .lengthDest = lengthDest, 
      .size = size,
      .decoder = decoder,
    } }
  };
}

bool decodeFields(DecoderState *state, int count, ...) {
  if (state->currentNode->tag != JSON_OBJECT) {
    FAIL(state, "Expecting object, got %s", nodeTagToString(state->currentNode->tag));
  }

  va_list ap;
  va_start(ap, count);
  for (int i = 0; i < count; i++) {
    FieldDef field = va_arg(ap, FieldDef);
    bool result = decodeField(state, field);
    if (!result) {
      return false;
    }
  }
  va_end(ap);
  return true;
}

bool decodeList(DecoderState *state, void *dest, int *length, size_t size, decodeFun decoder) {
  if (state->currentNode->tag != JSON_LIST) {
    FAIL(state, "Expecting list, got %s", nodeTagToString(state->currentNode->tag));
  }

  JSONNode *currentNode = state->currentNode;
  int currentDepth = state->error.depth;
  NodeList *nodeList = currentNode->data.JSON_LIST.nodes;

  void **listDest = (void**)dest;
  *listDest = malloc(nodeList->length * size),

  *length = nodeList->length;
  
  for (int i = 0; i < *length; i++) {
    JSONNode *item = &nodeList->items[i];
    state->currentNode = item;

    state->error.depth = currentDepth + 1;
    setDecoderPath(&state->error, currentDepth, (JSONPath) {
      .tag = JSON_INDEX,
      .data = { .JSON_INDEX = { .index = i } }
    });

    bool result = decoder(state, *listDest + (size * i));
    if (!result) {
      return false;
    }
  }
  state->currentNode = currentNode;
  state->error.depth = currentDepth;
  return true;
}

char *buildDecoderError(DecoderError err) {
  StringBuilder builder = StringBuilder_new();

  StringBuilder_append(&builder, "At root");
  for (int i = 0; i < err.depth; i++) {
    JSONPath path = err.path[i];
    switch (path.tag) {
      case JSON_FIELD:
        StringBuilder_append(&builder, "[\"%s\"]", path.data.JSON_FIELD.fieldName);
        break;

      case JSON_INDEX:
        StringBuilder_append(&builder, "[%d]", path.data.JSON_INDEX.index);
        break;
    }
  }
  StringBuilder_append(&builder, ": %s", err.errorMsg);
  
  return StringBuilder_getString(&builder);
}

void printDecoderError(DecoderError err) {
  char *errorMsg = buildDecoderError(err);
  printf("%s\n", errorMsg);
  free(errorMsg);
}

DecodeResult decode(char *input, void *dest, decodeFun decoder) {
  DecodeResult result;

  ParserResult parseResult = parse(input);

  if (parseResult.status != PARSER_SUCCESS) {
    char *errorMsg;
    allocsprintf(errorMsg, "Parsing failed: %s", parseResult.result.PARSER_ERROR.errorMsg);

    result = (DecodeResult) {
      .error = (DecoderError) {
        .path = NULL,
        .depth = 0,
        .errorMsg = errorMsg,
      },
      .success = false,
    };
    return result;
  }

  #define DECODER_ERROR_START_CAPACITY 5

  JSONNode *node = parseResult.result.PARSER_SUCCESS.tree;
  DecoderState state = {
    .currentNode = node,
    .error = (DecoderError) {
      .errorMsg = NULL,
      .pathCapacity = DECODER_ERROR_START_CAPACITY,
      .path = calloc(DECODER_ERROR_START_CAPACITY, sizeof(JSONPath))
    }
  };

  bool success = decoder(&state, dest);

  if (success) {
    DecodeError_free(state.error);
  }

  JSONNode_free(node);

  result.error = state.error;
  result.success = success;
  return result;
}

void setDecoderPath(DecoderError *error, int depth, JSONPath jPath) {
  if (depth > error->pathCapacity) {
    int newCapacity = error->pathCapacity * 2;
    JSONPath *newPath = reallocarray(error->path, newCapacity, sizeof(JSONPath)) ;
    error->path = newPath;
    error->pathCapacity = newCapacity;
  }

  JSONPath *ptr = &error->path[depth];
  *ptr = jPath;
}

void DecodeError_free(DecoderError error) {
  if (error.path != NULL) {
    free(error.path);
  }
  if (error.errorMsg != NULL) {
    free(error.errorMsg);
  }
}

