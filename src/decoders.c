#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "decoders.h"
#include "parser.h"
#include "nodelist.h"

#define FAIL(args...) do {\
  printf(args);\
  printf("\n");\
  exit(-1);\
} while(0)

void decodeInt(DecoderState *state, void *dest) {
  if (state->currentNode->tag != JSON_NUMBER) {
    FAIL("Expecting number");
  }
  double num = state->currentNode->data.JSON_NUMBER.number;
  if(ceil(num) != num) {
    FAIL("Expected integer, got float");
  }
  int *numDest = (int*)dest;
  *numDest = (int)num;
}

void decodeFloat(DecoderState *state, void *dest) {
  if (state->currentNode->tag != JSON_NUMBER) {
    FAIL("Expecting number");
  }
  double num = state->currentNode->data.JSON_NUMBER.number;
  double *doubleDest = (double*)dest;
  *doubleDest = num;
}

void decodeString(DecoderState *state, void *dest) {
  if (state->currentNode->tag != JSON_STRING) {
    FAIL("Expecting string");
  }
  char *str = state->currentNode->data.JSON_STRING.string;
  char **strDest = (char**)dest;
  *strDest = strdup(str);
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

void decodeField(DecoderState *state, FieldDef field) {
  JSONNode *current = state->currentNode;
  JSONNode *node = findField(current->data.JSON_OBJECT.nodes, field.name);

  if (node == NULL) {
    FAIL("No field with name \"%s\" was found", field.name);
  }

  state->currentNode = node;

  switch (field.type) {
    case NORMAL_FIELD: {
      struct NORMAL_FIELD data = field.data.NORMAL_FIELD;
      data.decoder(state, data.dest);
      break;
    }

    case LIST_FIELD: {
      struct LIST_FIELD data = field.data.LIST_FIELD;
      decodeList(state, data.dest, data.lengthDest, data.size, data.decoder);
      break;
    }
  }

  state->currentNode = current;
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

void decodeFields(DecoderState *state, int count, ...) {
  if (state->currentNode->tag != JSON_OBJECT) {
    FAIL("Expecting object");
  }

  va_list ap;
  va_start(ap, count);
  for (int i = 0; i < count; i++) {
    FieldDef field = va_arg(ap, FieldDef);
    decodeField(state, field);
  }
  va_end(ap);
}

void decodeList(DecoderState *state, void *dest, int *length, size_t size, decodeFun decoder) {
  if (state->currentNode->tag != JSON_LIST) {
    FAIL("Expecting list");
  }
  JSONNode *currentNode = state->currentNode;
  NodeList *nodeList = currentNode->data.JSON_LIST.nodes;

  void **listDest = (void**)dest;
  *listDest = malloc(nodeList->length * size),

  *length = nodeList->length;

  for (int i = 0; i < *length; i++) {
    JSONNode *item = &nodeList->items[i];
    state->currentNode = item;
    decoder(state, *listDest + (size * i));
  }
  state->currentNode = currentNode;
}

void decode(char *input, void *dest, decodeFun decoder) {
  ParserResult parseResult = parse(input);
  if (parseResult.status != PARSER_SUCCESS) {
    FAIL("Parsing failed: %s\n", parseResult.result.PARSER_ERROR.errorMsg);
  }

  JSONNode *node = parseResult.result.PARSER_SUCCESS.tree;
  DecoderState state = {
    .currentNode = node,
  };

  decoder(&state, dest);

  JSONNode_free(node);
}

