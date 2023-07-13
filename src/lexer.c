#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "tokenlist.h"

void _lex(LexerState *state);
static char eof(LexerState *state);
void skipWhitespace(LexerState *state);
char isDigit(char n);
char next(LexerState *state);
char peek(LexerState *state);
void lexNumber(LexerState *state);
void lexSingleChar(LexerState *state, TokenType type);
void lexString(LexerState *state);
void lexFalse(LexerState *state);
void lexTrue(LexerState *state);
void lexWord(LexerState *state, char *word, TokenType type);

TokenList lex(char *input) {
  TokenList list = {
    .length = 0,
    .capacity = TOKEN_START_CAPACITY,
    .tokens = malloc(TOKEN_START_CAPACITY * sizeof(Token))
  };

  LexerState state = {
    .input = input,
    .tokenList = &list,
    .col = 1,
    .row = 1,
  };

  _lex(&state);
  return list;
}

void _lex(LexerState *state) {
  while (!eof(state)) {
    skipWhitespace(state);
    char next = peek(state);

    if (isDigit(next) || next == '-' || next == '+') {
      lexNumber(state);
    } else if (next == '"') {
      lexString(state);
    } else if (next == 't') {
      lexTrue(state);
    } else if (next == 'f') {
      lexFalse(state);
    } else if (next == 'n') {
      lexWord(state, "null", TOKEN_NULL_LITERAL);
    } else if (next == '{') {
      lexSingleChar(state, TOKEN_OPEN_CURLY);
    } else if (next == '}') {
      lexSingleChar(state, TOKEN_CLOSE_CURLY);
    } else if (next == '[') {
      lexSingleChar(state, TOKEN_OPEN_SQUARE);
    } else if (next == ']') {
      lexSingleChar(state, TOKEN_CLOSE_SQUARE);
    } else if (next == ',') {
      lexSingleChar(state, TOKEN_COMMA);
    } else if (next == ':') {
      lexSingleChar(state, TOKEN_COLON);
    } else if (eof(state)) {
      return;
    } else {
      DIE("Unkown character '%c' at %d:%d\n", next, state->row, state->col);
    }
  }
}

char isDigit(char n) {
  return n >= '0' && n <= '9';
}

char isWhitespace(char n) {
  return n == ' ' || n == '\n';
}

char peek(LexerState *state) {
  return state->input[0];
}

static char eof(LexerState *state) {
  char next_char = peek(state);
  return next_char == '\0';
}

char next(LexerState *state) {
  char nextChar = peek(state);
  if (nextChar == '\n') {
    state->row++;
    state->col = 1;
  } else {
    state->col++;
  }
  return *state->input++;
}

void skipWhitespace(LexerState *state) {
  while(!eof(state)) {
    char next_char = peek(state);
    if (!isWhitespace(next_char)) {
      break;
    }
    next(state);
  }
}

void lexWord(LexerState *state, char *word, TokenType type) {
  int startRow = state->row;
  int startCol = state->col;

  long len = strlen(word);
  char localStr[len + 1];
  char nextChar;
  for (int i = 0; i < len; i++) {
    nextChar = next(state);
    if (nextChar == '\0') {
      DIE("Expected \"%s\" at %d:%d\n", word, startRow, startCol);
    }
    localStr[i] = nextChar;
  }
  localStr[len] = '\0';

  if (strcmp(localStr, word) != 0) {
    DIE("Expected \"%s\" at %d:%d\n", word, startRow, startCol);
  }

  Token *token = TokenList_insertNew(state->tokenList);
  token->tokenType = type;
}

void lexTrue(LexerState *state) {
  lexWord(state, "true", TOKEN_BOOL_LITERAL);
  Token *currToken = &state->tokenList->tokens[state->tokenList->length - 1];
  currToken->data.TOKEN_BOOL_LITERAL.boolean = true;
}

void lexFalse(LexerState *state) {
  lexWord(state, "false", TOKEN_BOOL_LITERAL);
  Token *currToken = &state->tokenList->tokens[state->tokenList->length - 1];
  currToken->data.TOKEN_BOOL_LITERAL.boolean = false;
}

void lexSingleChar(LexerState *state, TokenType type) {
  next(state);
  Token *token = TokenList_insertNew(state->tokenList);
  token->tokenType = type;
}

void lexNumber(LexerState *state) {
  char *input = state->input;
  char *input_end = input;

  double res = strtod(input, &input_end);
  int length = input_end - input;

  for (int i = 0; i < length; i++) {
    next(state);
  }
  Token *token = TokenList_insertNew(state->tokenList);
  token->tokenType = TOKEN_NUMBER_LITERAL;
  token->data.TOKEN_NUMBER_LITERAL.number = res;
}

// TODO This does not handle escape characters (\n, \" etc)
void lexString(LexerState *state) {
  int startRow = state->row;
  int startCol = state->col;

  next(state); // skip initial "
 
  Token *token = TokenList_insertNew(state->tokenList);
  token->tokenType = TOKEN_STRING_LITERAL;

  char *strStart = state->input;
  int strLen = 0;
  char next_char;
  while ((next_char = next(state)) != '"' && !eof(state) && next_char != '\n') {
    strLen++;
  }
  char *copiedStr = malloc(strLen + 1);
  memcpy(copiedStr, strStart, strLen);
  copiedStr[strLen] = '\0';

  token->data.TOKEN_STRING_LITERAL.string = copiedStr;

  if (eof(state) || next_char == '\n') {
    DIE("Unterminated string literal at %d:%d\n", startRow, startCol);
  }
}

void printTokenType(TokenType type) {
  switch (type) {
    case TOKEN_NUMBER_LITERAL:
      printf("Number literal");
      break;

    case TOKEN_NULL_LITERAL:
      printf("Null literal");
      break;

    case TOKEN_BOOL_LITERAL:
      printf("Boolean literal");
      break;

    case TOKEN_STRING_LITERAL:
      printf("String literal");
      break;

    case TOKEN_COLON:
      printf(":");
      break;

    case TOKEN_COMMA:
      printf(",");
      break;

    case TOKEN_OPEN_SQUARE:
      printf("[");
      break;

    case TOKEN_CLOSE_SQUARE:
      printf("]");
      break;

    case TOKEN_OPEN_CURLY:
      printf("{");
      break;

    case TOKEN_CLOSE_CURLY:
      printf("}");
      break;
  }
}

void printToken(Token *token) {
  switch (token->tokenType) {
    case TOKEN_NUMBER_LITERAL:
      printf("numberLiteral(%f)", token->data.TOKEN_NUMBER_LITERAL.number);
      break;

    case TOKEN_STRING_LITERAL:
      printf("stringLiteral(\"%s\")", token->data.TOKEN_STRING_LITERAL.string);
      break;

    case TOKEN_BOOL_LITERAL:
      printf("boolLiteral(%s)", (token->data.TOKEN_BOOL_LITERAL.boolean ? "true" : "false"));
      break;

    case TOKEN_NULL_LITERAL:
      printf("nullLiteral(null)");
      break;

    case TOKEN_OPEN_CURLY:
      printf("openCurly( { )");
      break;

    case TOKEN_CLOSE_CURLY:
      printf("closeCurly( } )");
      break;

    case TOKEN_OPEN_SQUARE:
      printf("openSquare( [ )");
      break;

    case TOKEN_CLOSE_SQUARE:
      printf("closeSquare( ] )");
      break;

    case TOKEN_COMMA:
      printf("comma( , )");
      break;

    case TOKEN_COLON:
      printf("colon( : )");
      break;
  }
}


