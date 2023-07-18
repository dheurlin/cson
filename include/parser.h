#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>

#include "lexer.h"

struct NodeList;
typedef struct JSONNode JSONNode;

struct JSONNode {
  enum JSONNode_Tag {
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

#define PARSER_ERROR_MAX_SIZE 256

typedef struct {
  Token *current_token;
  Token *tokens_end;
  JSONNode *current_node;
  int depth;
  char errorMsg[PARSER_ERROR_MAX_SIZE];
} ParserState;

typedef struct {
  enum {
    PARSER_SUCCESS,
    PARSER_FAIL,
  } status;
  union {
    struct PARSER_SUCCESS {
      JSONNode *tree;
    } PARSER_SUCCESS;
    struct PARSER_FAIL {
      char errorMsg[PARSER_ERROR_MAX_SIZE];
    } PARSER_ERROR;
  } result;
} ParserResult;

// Does not take ownership of the input, caller must deallocate. On failure, will free its partial `JSONNode`.
// On success, ownership of the returned `JSONNode` is transferred to the caller who must free it using `JSONNode_free`.
ParserResult parse(char *input);
void printTree(JSONNode *root);
void JSONNode_free(JSONNode *node);
void _JSONNode_free(JSONNode *node, bool inList);
char *nodeTagToString(enum JSONNode_Tag tag);

#endif
