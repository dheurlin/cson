#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>

#include "lexer.h"

struct NodeList;
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
    struct JSON_NUMBER { double number;          } JSON_NUMBER;
    struct JSON_STRING { char *string;           } JSON_STRING;
    struct JSON_BOOL   { bool boolean;           } JSON_BOOL;
    struct JSON_NULL   {                         } JSON_NULL;
    struct JSON_OBJECT { struct NodeList *nodes; } JSON_OBJECT;
    struct JSON_LIST   { struct NodeList *nodes; } JSON_LIST;
  } data;
  char *fieldName;
};

typedef struct {
  Token *current_token;
  Token *tokens_end;
  JSONNode *current_node;
} ParserState;

void printTree(JSONNode *root);
JSONNode *parse(Token *tokens, int length);
void JSONNode_free(JSONNode *node, bool inList);

#endif
