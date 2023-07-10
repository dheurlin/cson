#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "nodelist.h"

void _parse(ParserState *state);
void parseNull(ParserState *state);
void parseBool(ParserState *state);
void parseNumber(ParserState *state);
void parseString(ParserState *state);
void parseList(ParserState *state);
void parseObject(ParserState *state);

JSONNode *parse(Token *tokens, int length) {
  JSONNode *root = malloc(sizeof(JSONNode));
  ParserState state = {
    .current_node = root,
    .current_token = tokens,
    .tokens_end = tokens + length
  };

  _parse(&state);

  return root;
}

void _parse(ParserState *state) {
  Token next = *state->current_token;
  if (next.tokenType == TOKEN_NULL_LITERAL) {
    parseNull(state);
  } else if (next.tokenType == TOKEN_NUMBER_LITERAL) {
    parseNumber(state);
  } else if (next.tokenType == TOKEN_STRING_LITERAL) {
    parseString(state);
  } else if (next.tokenType == TOKEN_BOOL_LITERAL) {
    parseBool(state);
  } else if (next.tokenType == TOKEN_OPEN_SQUARE) {
    parseList(state);
  } else if (next.tokenType == TOKEN_OPEN_CURLY) {
    parseObject(state);
  } else {
    printf("Unhandled token: "); printTokenType(next.tokenType); printf("\n");
    DIE("Aborting\n");
  }
}

void parseNull(ParserState *state) {
  JSONNode *node = state->current_node;
  node->tag = JSON_NULL;
  state->current_token++;
}

void parseNumber(ParserState *state) {
  JSONNode *node = state->current_node;
  node->tag = JSON_NUMBER;
  node->data.JSON_NUMBER.number = state->current_token->contents.number;
  state->current_token++;
}


void parseString(ParserState *state) {
  JSONNode *node = state->current_node;
  node->tag = JSON_STRING;

  char *sourceStr = state->current_token->contents.str;
  char *heapStr = strdup(sourceStr);
  node->data.JSON_STRING.string = heapStr;

  state->current_token++;
}

void parseBool(ParserState *state) {
  JSONNode *node = state->current_node;
  node->tag = JSON_BOOL;
  node->data.JSON_BOOL.boolean = strcmp(state->current_token->contents.str, "true") == 0;
  state->current_token++;
}

bool eof(ParserState *state) {
  return state->current_token >= state->tokens_end;
}

#define nextIsClosingBrace(state) (state->current_token->tokenType == TOKEN_CLOSE_SQUARE)

TokenType peekTokenType(ParserState *state) {
  return state->current_token->tokenType;
}

Token *nextToken(ParserState *state) {
  Token *next = state->current_token++;
  return next;
}

void expect(ParserState *state, TokenType type) {
  if (eof(state) || peekTokenType(state) != type) {
    printf("Expecting "); printTokenType(type); printf("\n");
    DIE("aborting\n");
  }
}

void consume(ParserState *state, TokenType type) {
  expect(state, type);
  nextToken(state);
}

void parseList(ParserState *state) {
  consume(state, TOKEN_OPEN_SQUARE);

  JSONNode *node = state->current_node;
  NodeList *nodeList = NodeList_new();
  node->tag = JSON_LIST;
  node->data.JSON_LIST.nodes = nodeList;

  while (!eof(state) && !nextIsClosingBrace(state)) {
    JSONNode *elem = NodeList_insertNew(nodeList);
    state->current_node = elem;
    _parse(state);

    if (peekTokenType(state) == TOKEN_CLOSE_SQUARE || eof(state)) {
      break;
    }
    // This allows a trailing comma, could be fixed but why not keep it?
    consume(state, TOKEN_COMMA);
  }

  consume(state, TOKEN_CLOSE_SQUARE);
  state->current_node = node;
}

void parseObject(ParserState *state) {
  consume(state, TOKEN_OPEN_CURLY);

  JSONNode *node = state->current_node;
  NodeList *nodeList = NodeList_new();
  node->tag = JSON_OBJECT;
  node->data.JSON_OBJECT.nodes = nodeList;

  while (!eof(state) && peekTokenType(state) != TOKEN_CLOSE_CURLY) {
    expect(state, TOKEN_STRING_LITERAL);
    char *name = nextToken(state)->contents.str;

    consume(state, TOKEN_COLON);
    JSONNode *elem = NodeList_insertNew(nodeList);
    state->current_node = elem;
    _parse(state);
    elem->fieldName = strdup(name);

    if (peekTokenType(state) == TOKEN_CLOSE_CURLY || eof(state)) {
      break;
    }
    // This allows a trailing comma, could be fixed but why not keep it?
    consume(state, TOKEN_COMMA);
  }

  consume(state, TOKEN_CLOSE_CURLY);
  state->current_node = node;
}

#define indentDepth 2 
#define printIndent(N) for (int i = 0; i < (N); i++) printf(" ");

void _printTree(int indentLevel, JSONNode *tree) {
  JSONNode node = *tree;
  switch (node.tag) {
    case JSON_STRING:
      printIndent(indentLevel);
      printf("string \"%s\"\n", node.data.JSON_STRING.string);
      break;

    case JSON_NUMBER:
      printIndent(indentLevel);
      printf("number %f\n", node.data.JSON_NUMBER.number);
      break;

    case JSON_NULL:
      printIndent(indentLevel);
      printf("null\n");
      break;

    case JSON_BOOL: {
      printIndent(indentLevel);
      printf("boolean %s\n", node.data.JSON_BOOL.boolean ? "true" : "false");
      break;
    }

    case JSON_LIST: {
      struct JSON_LIST data = node.data.JSON_LIST;
      JSONNode *items = data.nodes->items;
      printIndent(indentLevel); printf("List (%d) [\n", data.nodes->length);

      for (int i = 0; i < data.nodes->length; i++) {
        _printTree(indentLevel + indentDepth, &items[i]);
      }
      
      printIndent(indentLevel); printf("]\n");
      break;
    }
    case JSON_OBJECT: {
      struct JSON_OBJECT data = node.data.JSON_OBJECT;
      NodeList *list = data.nodes;

      printIndent(indentLevel); printf("Object {\n");

      for (int i = 0; i < list->length; i++) {
        char *name = list->items[i].fieldName;
        printIndent(indentLevel + indentDepth); printf("\"%s\":\n ", name);
        _printTree(indentLevel + (indentDepth * 2), &list->items[i]);
        if (i < list->length - 1) printf("\n");
      }

      printIndent(indentLevel); printf("}\n");
      break;
    }
  }
}

void printTree(JSONNode *tree) {
  _printTree(0, tree);
}

void JSONNode_free(JSONNode *ptr, bool inList) {
  JSONNode node = *ptr;
  switch (node.tag) {
    case JSON_NULL: {
      break;
    }
    case JSON_NUMBER: {
      struct JSON_NUMBER data = node.data.JSON_NUMBER;
      break;
    }
    case JSON_BOOL: {
      struct JSON_BOOL data = node.data.JSON_BOOL;
      break;
    }
    case JSON_STRING: {
      struct JSON_STRING data = node.data.JSON_STRING;
      char *str = data.string;
      free(str);
      break;
    }
    case JSON_OBJECT: {
      struct JSON_OBJECT data = node.data.JSON_OBJECT;
      NodeList_free(data.nodes);
      break;
    }
    case JSON_LIST: {
      struct JSON_LIST data = node.data.JSON_LIST;
      NodeList_free(data.nodes);
      break;
    }
  }

  char *name = node.fieldName;
  if (name != NULL) {
    free(name);
  }

  // If the node is stored in a list, free(ptr) would deallocate that whole list.
  if (!inList) {
    free(ptr);
  }
}

