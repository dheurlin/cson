#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"

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

char isDigit(char n) {
  return n >= '0' && n <= '9';
}

char isWhitespace(char n) {
  return n == ' ' || n == '\n';
}

char peek(LexerState *state) {
  return state->input[state->input_pos];
}

static char eof(LexerState *state) {
  char next_char = peek(state);
  return state->input_pos >= state->input_length || next_char == '\0';
}

char next(LexerState *state) {
  return state->input[state->input_pos++];
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

void TokenList_resize(TokenList *list) {
  int newCapacity = list->capacity * 2;
  Token *newItems = reallocarray(list->tokens, newCapacity, sizeof(Token));
  list->tokens = newItems;
  list->capacity = newCapacity;
}

Token *TokenList_insertNew(TokenList *list) {
  int oldLength = list->length;
  int newLength = oldLength + 1;
  if (newLength > list->capacity) {
    TokenList_resize(list);
  }
  list->length = newLength;
  return &list->tokens[oldLength];
}

void lexWord(LexerState *state, char *word, TokenType type) {
  long len = strlen(word);
  char localStr[len + 1];
  char next;
  for (int i = 0; i < len; i++) {
    next = state->input[state->input_pos + i];
    if (state->input_pos >= state->input_length || next == '\0') {
      DIE("Expected \"%s\" at position %d\n", word, state->input_pos);
    }
    localStr[i] = next;
  }
  localStr[len] = '\0';

  if (strcmp(localStr, word) != 0) {
    DIE("Expected \"%s\" at position %d\n", word, state->input_pos);
  }

  Token *token = TokenList_insertNew(state->tokenList);
  token->tokenType = type;
  state->input_pos += len;
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
  char *input = &state->input[state->input_pos];
  char *input_end = input;

  double res = strtod(input, &input_end);
  int length = input_end - input;

  state->input_pos += length;
  Token *token = TokenList_insertNew(state->tokenList);
  token->tokenType = TOKEN_NUMBER_LITERAL;
  token->data.TOKEN_NUMBER_LITERAL.number = res;
}

void lexString(LexerState *state) {
  next(state); // skip initial "
 
  Token *token = TokenList_insertNew(state->tokenList);
  token->tokenType = TOKEN_STRING_LITERAL;

  char *strStart = &state->input[state->input_pos];
  int strLen = 0;
  char next_char;
  while ((next_char = next(state)) != '"' && !eof(state)) {
    strLen++;
  }
  char *copiedStr = malloc(strLen + 1);
  memcpy(copiedStr, strStart, strLen);
  copiedStr[strLen] = '\0';

  token->data.TOKEN_STRING_LITERAL.string = copiedStr;

  if (eof(state)) {
    DIE("Expected '\"' at position %d, got EOF\n", state->input_pos);
  }
}

void _lex(LexerState *state);

TokenList lex(char *input, int inputLen) {
  TokenList list = {
    .length = 0,
    .capacity = TOKEN_START_CAPACITY,
    .tokens = malloc(TOKEN_START_CAPACITY * sizeof(Token))
  };

  LexerState state = {
    .input = input,
    .input_length = inputLen,
    .input_pos = 0,
    .tokenList = &list,
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
      DIE("Unkown character at position %d: %c\n", state->input_pos, next);
    }
  }
}

void TokenList_free(TokenList *list) {
  for (int i = 0; i < list->length; i++) {
    Token token = list->tokens[i];
    switch (token.tokenType) {
      case TOKEN_STRING_LITERAL: {
        char *str = token.data.TOKEN_STRING_LITERAL.string;
        free(str);
      }

      case TOKEN_NUMBER_LITERAL:
      case TOKEN_BOOL_LITERAL:
      case TOKEN_NULL_LITERAL:
      case TOKEN_OPEN_CURLY:
      case TOKEN_CLOSE_CURLY:
      case TOKEN_OPEN_SQUARE:
      case TOKEN_CLOSE_SQUARE:
      case TOKEN_COMMA:
      case TOKEN_COLON:
        break;
    }
  }
  free(list->tokens);
}

