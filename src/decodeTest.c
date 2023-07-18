#include "decoders.h"
#include "stdio.h"
#include <stdlib.h>

char *pointStr =
"{\n\
  \"x\": 69,\n\
  \"y\": 420,\n\
}";
char *personStr = "{\n  \"firstName\": \"Walter\",\n  \"lastName\" : \"White\",\n  \"age\": 52\n}";
char *numbersStr = "[1, 2, 3, 4, 5]";
char *pointListStr = "[{ \"x\": 19, \"y\": 95 }, { \"x\": 4, \"y\": 20 }, { \"x\": 18, \"y\": 99 }]";
char *familyStr = "{\"father\":{\"firstName\":\"Walter\",\"lastName\":\"White\",\"age\":52},\"mother\":{\"firstName\":\"Skyler\",\"lastName\":\"White\",\"age\":40},\"children\":[{\"firstName\":\"Walter Jr.\",\"lastName\":\"White\",\"age\":17},{\"firstName\":\"Holly\",\"lastName\":\"White\",\"age\":1}]}";
char *familyStrWrong = "{\"father\":{\"firstName\":\"Walter\",\"lastName\":\"White\",\"age\":52},\"mother\":{\"firstName\":\"Skyler\",\"lastName\":\"White\",\"age\":40},\"children\":[{\"firstName\":\"Walter Jr.\",\"lastName\":\"White\",\"age\":17},{\"firstName\":\"Holly\",\"lastName\":\"White\",\"age\": \"hello\"}]}";

typedef struct Point {
  int x;
  int y;
} Point;

void printPoint(Point point) {
  printf("Point { .x = %d, .y = %d }\n", point.x, point.y);
}

bool decodePoint(DecoderState *state, void *dest) {
  Point *point = (Point*)dest;
  return decodeFields(state, 2, 
    makeField("x", &point->x, decodeInt),
    makeField("y", &point->y, decodeInt)
  );
}

typedef struct NumberList {
  int *numbers;
  int length;
} NumberList;

bool decodeNumberList(DecoderState *state, void *dest) {
  NumberList *numberList = (NumberList*)dest;
  return decodeList(state, &numberList->numbers, &numberList->length, sizeof(int), decodeInt);
}

void printNumberList(NumberList list) {
  for (int i = 0; i < list.length; i++) {
    printf("%d\n", list.numbers[i]);
  }
}

typedef struct PointList {
  Point *points;
  int len;
} PointList;

bool decodePointList(DecoderState *state, void *dest) {
  PointList *pointList = (PointList*)dest;
  return decodeList(state, &pointList->points, &pointList->len, sizeof(Point), decodePoint);
}

typedef struct Person {
  char *firstName;
  char *lastName;
  int age;
} Person;

void printPerson(Person person) {
  printf("%s %s, %d\n", person.firstName, person.lastName, person.age);
}

typedef struct Family {
  Person father;
  Person mother;
  Person *children;
  int childCount;
} Family;

bool decodePerson(DecoderState *state, void *dest) {
  Person *person = (Person*)dest;
  return decodeFields(state, 3,
    makeField("firstName", &person->firstName, decodeString),
    makeField("lastName", &person->lastName, decodeString),
    makeField("age", &person->age, decodeInt)
  );
}

bool decodeFamily(DecoderState *state, void *dest) {
  Family *family = (Family*)dest;
  return decodeFields(state, 3,
    makeField("father", &family->father, decodePerson),
    makeField("mother", &family->mother, decodePerson),
    makeListField("children", &family->children, &family->childCount, sizeof(Person), decodePerson)
  );
}

int main() {
  Point decodedPoint;
  decode(pointStr, &decodedPoint, decodePoint);

  printf("Decoded point: \n");
  printf("----------------------------\n");
  printPoint(decodedPoint);
  printf("\n");

  // ------------------

  Person decodedPerson;
  decode(personStr, &decodedPerson, decodePerson);

  printf("Decoded person: \n");
  printf("----------------------------\n");
  printPerson(decodedPerson);
  printf("\n");

  // --------------
  NumberList numberList;
  decode(numbersStr, &numberList, decodeNumberList);

  printf("Decoded list: \n");
  printf("----------------------------\n");
  printNumberList(numberList);
  printf("\n");

  // --------------
  PointList pointList;
  decode(pointListStr, &pointList, decodePointList);

  printf("Decoded list of points: \n");
  printf("----------------------------\n");
  for (int i = 0; i < pointList.len; i++) {
    printPoint(pointList.points[i]);
  }
  printf("\n");

  // --------------

  Family family;
  DecodeResult res = decode(familyStr, &family, decodeFamily);

  if(!res.success) {
    printDecoderError(res.error);
    exit(-1);
  }

  printf("Decoded family: \n");
  printf("----------------------------\n");
  printf("Father: "); printPerson(family.father);
  printf("Mother: "); printPerson(family.mother);
  printf("Children: \n");
  for (int i = 0; i < family.childCount; i++) {
    printf("  "); printPerson(family.children[i]);
  }
  printf("\n");

  // --------------

  printf("Error message example: \n");
  printf("----------------------------\n");

  Family wrongFam;
  DecodeResult wrongFamRes = decode(familyStrWrong, &wrongFam, decodeFamily);

  if (!wrongFamRes.success) {
    printDecoderError(wrongFamRes.error);
    exit(-1);
  }

  return 1;
}
