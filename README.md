# JSON Decoding in C.

Initially inspired by [Elm's JSON decoders](https://package.elm-lang.org/packages/elm/json/latest/Json.Decode), but ended up a bit different.


## TODO

* Add config to build as shared library, currently only builds example executables.
* Fix various memory issues
* Support string escape characters in lexer

# Usage

Let's say we have the following `struct`s representing a family:

```c
typedef struct {
  char *firstName;
  char *lastName;
  int age;
} Person;

typedef struct {
  Person father;
  Person mother;
  Person *children;
  int childCount;
} Family;

void printPerson(Person person) {
  printf("%s %s, %d\n", person.firstName, person.lastName, person.age);
}
```

And we want to deserialize the following JSON into an instance of that `struct`:

```json
{
    "father": {
        "firstName": "Walter",
        "lastName": "White",
        "age": 52
    },
    "mother": {
        "firstName": "Skyler",
        "lastName": "White",
        "age":40
    },
    "children": [
        {
            "firstName": "Walter Jr.",
            "lastName": "White", 
            "age": 17
        },
        {
            "firstName": "Holly",
            "lastName": "White",
            "age": 1
        }
    ]
}

```

We can define a set of decoders like this:

```c
#include "decoders.h"

// ...

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
```

And invoke them as such (given that `familyStr` is a `char*` containing our JSON):

```c
Family family;
DecodeResult res = decode(familyStr, &family, decodeFamily);

printf("Decoded family: \n");
printf("----------------------------\n");

if(res.success) {
    printf("Father: "); printPerson(family.father);
    printf("Mother: "); printPerson(family.mother);
    printf("Children: \n");
    for (int i = 0; i < family.childCount; i++) {
        printf("  "); printPerson(family.children[i]);
    }
} else {
    printDecoderError(res.error);
    DecodeError_free(res.error);
}

```

Which will yield the following output:

```
Decoded family: 
----------------------------
Father: Walter White, 52
Mother: Skyler White, 40
Children: 
  Walter Jr. White, 17
  Holly White, 1
```

## Error messages

If the JSON does not match the expected format, we get an error message describing the location of the discrepancy. For example, if we replace `"age": 1` with `"age": "hello"` in the JSON above, we would get the following output instead:

```
Decoded family: 
----------------------------
At root["children"][1]["age"]: Expecting number, got string
```

## More examples

See the [decoder example file](src/decodeTest.c).
