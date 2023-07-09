#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"

void printToken(Token *token) {
  switch (token->tokenType) {
    case TOKEN_NUMBER_LITERAL:
      printf("numberLiteral(%f)", token->contents.number);
      break;

    case TOKEN_STRING_LITERAL:
      printf("stringLiteral(\"%s\")", token->contents.str);
      break;

    case TOKEN_BOOL_LITERAL:
      printf("boolLiteral(%s)", token->contents.str);
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

  Token *token = &state->tokens[state->token_pos++];
  token->tokenType = type;
  strcpy(token->contents.str, localStr);
  state->input_pos += len;
}

void lexSingleChar(LexerState *state, TokenType type) {
  next(state);
  Token *token = &state->tokens[state->token_pos++];
  token->tokenType = type;
}

void lexNumber(LexerState *state) {
  char *input = &state->input[state->input_pos];
  char *input_end = input;

  double res = strtod(input, &input_end);
  int length = input_end - input;

  state->input_pos += length;
  Token *token = &state->tokens[state->token_pos++];
  token->tokenType = TOKEN_NUMBER_LITERAL;
  token->contents.number = res;
}

void lexString(LexerState *state) {
  next(state); // skip initial "
 
  Token *token = &state->tokens[state->token_pos++];
  token->tokenType = TOKEN_STRING_LITERAL;

  int index = 0;
  char next_char;
  while ((next_char = next(state)) != '"' && !eof(state)) {
    token->contents.str[index++] = next_char;
  }
  token->contents.str[index++] = '\0';

  if (eof(state)) {
    DIE("Expected '\"' at position %d, got EOF\n", state->input_pos);
  }
}

void lex(LexerState *state) {
  while (!eof(state)) {
    skipWhitespace(state);
    char next = peek(state);

    if (isDigit(next) || next == '-' || next == '+') {
      lexNumber(state);
    } else if (next == '"') {
      lexString(state);
    } else if (next == 't') {
      lexWord(state, "true", TOKEN_BOOL_LITERAL);
    } else if (next == 'f') {
      lexWord(state, "false", TOKEN_BOOL_LITERAL);
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
