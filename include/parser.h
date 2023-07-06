#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>

#include "lexer.h"

typedef struct JSONNode JSONNode;

struct JSONNode {
  enum {
    JSON_NUMBER,
    JSON_STRING,
    JSON_BOOL,
    JSON_NULL,
    JSON_OBJECT,
    JSON_LIST,
  } tag;
  union {
    struct JSON_NUMBER { double number; } JSON_NUMBER;
    struct JSON_STRING { char *string;  } JSON_STRING;
    struct JSON_BOOL   { bool boolean;  } JSON_BOOL;
    struct JSON_NULL   {                } JSON_NULL;
    struct JSON_OBJECT {
      char *name;
      JSONNode *value;
    } JSON_OBJECT;
    struct JSON_LIST {
      int length;
      JSONNode *items;
    } JSON_LIST;
  } data;
};

typedef struct {
  Token *current_token;
  int tokens_len;
  JSONNode *current_node;
} ParserState;

void printTree(JSONNode *root);
JSONNode *parse(Token *tokens, int length);
void JSONNode_free(JSONNode *node);

#endif
