#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "decoders.h"
#include "parser.h"
#include "nodelist.h"

#define FAIL(state, args...) do {\
    sprintf(state->error.errorMsg, args);\
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

  if (state->error.depth == DECODER_MAX_DEPTH) {
    FAIL(state, "Max depth %d exceded", DECODER_MAX_DEPTH);
  }

  state->error.path[state->error.depth++] = (JSONPath) {
    .tag = JSON_FIELD,
    .data = { .JSON_FIELD = { .fieldName = field.name } }
  };

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
  FieldDef field = { .type = NORMAL_FIELD };
  field.data.NORMAL_FIELD.decoder = decoder;
  field.data.NORMAL_FIELD.dest = dest;
  field.name = name;
  return field;
}

FieldDef makeListField(char *name, void *dest, int *lengthDest, size_t size, decodeFun decoder) {
  FieldDef field = { .type = LIST_FIELD };
  field.data.LIST_FIELD.decoder = decoder;
  field.data.LIST_FIELD.dest = dest;
  field.name = name;
  field.data.LIST_FIELD.lengthDest = lengthDest;
  field.data.LIST_FIELD.size = size;
  return field;
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

  if (state->error.depth + 1 == DECODER_MAX_DEPTH) {
    FAIL(state, "Max depth %d exceded", DECODER_MAX_DEPTH);
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
    state->error.path[currentDepth] = (JSONPath) {
      .tag = JSON_INDEX,
      .data = { .JSON_INDEX = { .index = i } }
    };

    bool result = decoder(state, *listDest + (size * i));
    if (!result) {
      return false;
    }
  }
  state->currentNode = currentNode;
  state->error.depth = currentDepth;
  return true;
}

void printDecoderError(DecoderError err) {
  printf("At root");
  for (int i = 0; i < err.depth; i++) {
    JSONPath path = err.path[i];
    switch (path.tag) {
      case JSON_FIELD:
        printf("[\"%s\"]", path.data.JSON_FIELD.fieldName);
        break;

      case JSON_INDEX:
        printf("[%d]", path.data.JSON_INDEX.index);
        break;
    }
  }
  printf(": %s\n", err.errorMsg);
}

DecodeResult decode(char *input, void *dest, decodeFun decoder) {
  DecodeResult result;

  ParserResult parseResult = parse(input);

  if (parseResult.status != PARSER_SUCCESS) {
    sprintf(result.error.errorMsg, "Parsing failed: %s\n", parseResult.result.PARSER_ERROR.errorMsg);
    result.success = false;
    result.depth = 0;
    return result;
  }

  JSONNode *node = parseResult.result.PARSER_SUCCESS.tree;
  DecoderState state = {
    .currentNode = node,
    .error = (DecoderError) {
      .errorMsg = "",
    }
  };

  bool success = decoder(&state, dest);

  JSONNode_free(node);

  result.error = state.error;
  result.success = success;
  return result;
}

