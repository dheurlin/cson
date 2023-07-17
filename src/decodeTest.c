#include "parser.h"
#include "nodelist.h"
#include "stdio.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define FAIL(args...) do {\
  printf(args);\
  printf("\n");\
  exit(-1);\
} while(0)

char *pointStr =
"{\n\
  \"x\": 69,\n\
  \"y\": 420,\n\
}";

typedef struct Point {
  int x;
  int y;
} Point;

void printPoint(Point point) {
  printf("Point { .x = %d, .y = %d }\n", point.x, point.y);
}

char *personStr = "{\n  \"firstName\": \"Walter\",\n  \"lastName\" : \"White\",\n  \"age\": 52.4\n}";

typedef struct Person {
  char *firstName;
  char *lastName;
  double age;
} Person;

void printPerson(Person person) {
  printf("%s %s, %f\n", person.firstName, person.lastName, person.age);
}

typedef struct DecoderState {
  JSONNode *currentNode;
} DecoderState;

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

typedef void(*decodeFun)(DecoderState*, void*);

void decodeField(DecoderState *state, void *dest, char *name, decodeFun decoder) {
  JSONNode *current = state->currentNode;
  JSONNode *field = findField(current->data.JSON_OBJECT.nodes, name);
  if (field == NULL) {
    FAIL("No field with name \"%s\" was found", name);
  }
  state->currentNode = field;
  decoder(state, dest);
  state->currentNode = current;
}

typedef struct FieldDef {
  char* name;
  void *dest;
  decodeFun decoder;
} FieldDef;

FieldDef makeField(char *name, void *dest, decodeFun decoder) {
  return (FieldDef){
    .name = name,
    .dest = dest,
    .decoder = decoder,
  };
}

void decodeFields(DecoderState *state, int count, ...) {
  if (state->currentNode->tag != JSON_OBJECT) {
    FAIL("Expecting object");
  }

  va_list ap;
  va_start(ap, count);
  for (int i = 0; i < count; i++) {
    FieldDef field = va_arg(ap, FieldDef);
    decodeField(state, field.dest, field.name, field.decoder);
  }
  va_end(ap);
}

void decodePoint(DecoderState *state, void *dest) {
  Point *point = (Point*)dest;
  decodeFields(state, 2, 
    makeField("x", &point->x, decodeInt),
    makeField("y", &point->y, decodeInt)
  );
}

void decodePerson(DecoderState *state, void *dest) {
  Person *person = (Person*)dest;
  decodeFields(state, 3,
    makeField("firstName", &person->firstName, decodeString),
    makeField("lastName", &person->lastName, decodeString),
    makeField("age", &person->age, decodeFloat)
  );
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

int main() {

  Point decodedPoint;
  decode(pointStr, &decodedPoint, decodePoint);

  printf("Decoded point: \n");
  printPoint(decodedPoint);
  printf("\n");

  // ------------------

  Person decodedPerson;
  decode(personStr, &decodedPerson, decodePerson);

  printf("Decoded person: \n");
  printPerson(decodedPerson);
  printf("\n");

  return 1;
}
