#include "decoders.h"
#include "stdio.h"

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

char *numbersStr = "[1, 2, 3, 4, 5]";

typedef struct NumberList {
  int *numbers;
  int length;
} NumberList;

void printNumberList(NumberList list) {
  for (int i = 0; i < list.length; i++) {
    printf("%d\n", list.numbers[i]);
  }
}

char *pointListStr = "[{ \"x\": 19, \"y\": 95 }, { \"x\": 4, \"y\": 20 }, { \"x\": 18, \"y\": 99 }]";

typedef struct PointList {
  Point *points;
  int len;
} PointList;

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

void decodeNumberList(DecoderState *state, void *dest) {
  NumberList *numberList = (NumberList*)dest;
  decodeList(state, &numberList->numbers, &numberList->length, sizeof(int), decodeInt);
}

void decodePointList(DecoderState *state, void *dest) {
  PointList *pointList = (PointList*)dest;
  decodeList(state, &pointList->points, &pointList->len, sizeof(Point), decodePoint);
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

  // --------------
  NumberList numberList;
  decode(numbersStr, &numberList, decodeNumberList);

  printf("Decoded list: \n");
  printNumberList(numberList);
  printf("\n");

  // --------------
  PointList pointList;
  decode(pointListStr, &pointList, decodePointList);

  printf("Decoded list of points: \n");
  for (int i = 0; i < pointList.len; i++) {
    printPoint(pointList.points[i]);
  }

  return 1;
}
