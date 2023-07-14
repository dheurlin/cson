#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "nodelist.h"
#include "lexer.h"

static void _parse(ParserState *state);
void parseNull(ParserState *state);
void parseBool(ParserState *state);
void parseNumber(ParserState *state);
void parseString(ParserState *state);
void parseList(ParserState *state);
void parseObject(ParserState *state);
bool eof(ParserState *state);
TokenType peekTokenType(ParserState *state);
Token *nextToken(ParserState *state);

JSONNode *parse(char *input) {
  LexResult lexed = lex(input);

  if (lexed.status < 0) {
    DIE("[LEXER] %s", lexed.errorMsg);
    // TODO free TokenList?
  }

  // for(int i = 0; i < lexed.tokenList.length; i++) {
  //   printTokenLn(&lexed.tokenList.tokens[i]);
  // }

  JSONNode *root = malloc(sizeof(JSONNode));
  ParserState state = {
    .current_node = root,
    .current_token = lexed.tokenList.tokens,
    .tokens_end = lexed.tokenList.tokens + lexed.tokenList.length,
  };

  _parse(&state);

  TokenList_free(&lexed.tokenList);
  return root;
}

static void _parse(ParserState *state) {
  Token next = *state->current_token;
  switch (next.tokenType) {
    case TOKEN_NULL_LITERAL:
      parseNull(state);
      break;

    case TOKEN_NUMBER_LITERAL:
      parseNumber(state);
      break;

    case TOKEN_STRING_LITERAL:
      parseString(state);
      break;

    case TOKEN_BOOL_LITERAL:
      parseBool(state);
      break;

    case TOKEN_OPEN_SQUARE:
      parseList(state);
      break;

    case TOKEN_OPEN_CURLY:
      parseObject(state);
      break;

    default:
      printf("Unexpected token at %d:%d: ", next.row, next.col); printTokenType(next.tokenType); printf("\n");
      DIE("Aborting\n");
  }
}

void parseNull(ParserState *state) {
  JSONNode *node = state->current_node;
  node->tag = JSON_NULL;
  nextToken(state);
}

void parseNumber(ParserState *state) {
  JSONNode *node = state->current_node;
  node->tag = JSON_NUMBER;
  node->data.JSON_NUMBER.number = nextToken(state)->data.TOKEN_NUMBER_LITERAL.number;
}

void parseString(ParserState *state) {
  JSONNode *node = state->current_node;
  node->tag = JSON_STRING;
  node->data.JSON_STRING.string = strdup(nextToken(state)->data.TOKEN_STRING_LITERAL.string);
}

void parseBool(ParserState *state) {
  JSONNode *node = state->current_node;
  node->tag = JSON_BOOL;
  bool contents = nextToken(state)->data.TOKEN_BOOL_LITERAL.boolean;
  node->data.JSON_BOOL.boolean = contents;
}

bool eof(ParserState *state) {
  return state->current_token >= state->tokens_end;
}

TokenType peekTokenType(ParserState *state) {
  return state->current_token->tokenType;
}

Token peekToken(ParserState *state) {
  return *state->current_token;
}

Token *nextToken(ParserState *state) {
  Token *next = state->current_token++;
  return next;
}

void expect(ParserState *state, TokenType type) {
  Token next = peekToken(state);
  if (eof(state) || peekTokenType(state) != type) {
    printf("Expecting "); printTokenType(type); 
    if (eof(state)) printf(" at end of string\n");
    else            printf(" at %d:%d\n", next.row, next.col);
    DIE("aborting\n");
  }
}

void consume(ParserState *state, TokenType type) {
  expect(state, type);
  nextToken(state);
}

// TODO stops parsing if we have too many ]s, thinking it's the end of the file
void parseList(ParserState *state) {
  consume(state, TOKEN_OPEN_SQUARE);

  JSONNode *node = state->current_node;
  NodeList *nodeList = NodeList_new();
  node->tag = JSON_LIST;
  node->data.JSON_LIST.nodes = nodeList;

  while (!eof(state) && peekTokenType(state) != TOKEN_CLOSE_SQUARE) {
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
    char *name = nextToken(state)->data.TOKEN_STRING_LITERAL.string;

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

void JSONNode_free(JSONNode *root) {
  _JSONNode_free(root, false);
}

void _JSONNode_free(JSONNode *ptr, bool inList) {
  JSONNode node = *ptr;
  switch (node.tag) {
    case JSON_NULL:
    case JSON_NUMBER:
    case JSON_BOOL:
      break;

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

  // If the node is stored in a list, free(ptr) on the first element would deallocate that whole list.
  // NodeList_free is responsible for deallocating the list as a whole for such nodes.
  if (!inList) {
    free(ptr);
  }
}

