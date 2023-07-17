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

typedef struct PersonList {
  Person *persons;
  int count;
} PersonList;

typedef struct Family {
  Person father;
  Person mother;
  PersonList children;
} Family;

char *familyStr = "{\"father\":{\"firstName\":\"Walter\",\"lastName\":\"White\",\"age\":52},\"mother\":{\"firstName\":\"Skyler\",\"lastName\":\"White\",\"age\":36},\"children\":[{\"firstName\":\"Walter Jr.\",\"lastName\":\"White\",\"age\":16},{\"firstName\":\"Holly\",\"lastName\":\"White\",\"age\":0}]}";

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

void decodePersonList(DecoderState *state, void *dest) {
  PersonList *personList = (PersonList*)dest;
  decodeList(state, &personList->persons, &personList->count, sizeof(Person), decodePerson);
}

void decodeFamily(DecoderState *state, void *dest) {
  Family *family = (Family*)dest;
  decodeFields(state, 3,
    makeField("father", &family->father, decodePerson),
    makeField("mother", &family->mother, decodePerson),
    makeField("children", &family->children, decodePersonList)
  );
}

// TODO Decodelist doesn't compose nicely with fields, maybe a special makeField for lists
// that would allow us to define the list type in-line?
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
  printf("\n");

  // --------------
  Family family;
  decode(familyStr, &family, decodeFamily);

  printf("Decoded family: \n");
  printf("Father: "); printPerson(family.father);
  printf("Mother: "); printPerson(family.mother);
  printf("Children: \n");
  for (int i = 0; i < family.children.count; i++) {
    printf("  "); printPerson(family.children.persons[i]);
  }

  return 1;
}
