#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"

#define FAIL(state, args...) do {\
  sprintf(state->errorMsg, args);\
  return false;\
} while(0)

#define TRY(cmd) do {\
  if (!cmd) return false;\
} while(0)

bool _lex(LexerState *state);
static char eof(LexerState *state);
void skipWhitespace(LexerState *state);
char isDigit(char n);
char next(LexerState *state);
char peek(LexerState *state);
void lexNumber(LexerState *state);
void lexSingleChar(LexerState *state, TokenType type);
bool lexString(LexerState *state);
bool lexFalse(LexerState *state);
bool lexTrue(LexerState *state);
bool lexWord(LexerState *state, char *word, TokenType type);

LexResult lex(char *input) {
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
    .errorMsg = "",
  };

  bool status = _lex(&state);

  LexResult res;
  if (status) {
    res.status = LEXER_SUCCESS;
    res.result.LEXER_SUCCESS.tokenList = list;
  } else {
    res.status = LEXER_FAIL;
    strcpy(res.result.LEXER_FAIL.errorMsg, state.errorMsg);
    TokenList_free(&list);
  }

  return res;
}

bool _lex(LexerState *state) {
  while (!eof(state)) {
    skipWhitespace(state);
    char next = peek(state);

    if (isDigit(next) || next == '-' || next == '+') {
      lexNumber(state);
    } else if (next == '"') {
      TRY(lexString(state));
    } else if (next == 't') {
      TRY(lexTrue(state));
    } else if (next == 'f') {
      TRY(lexFalse(state));
    } else if (next == 'n') {
      TRY(lexWord(state, "null", TOKEN_NULL_LITERAL));
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
      return true;
    } else {
      FAIL(state, "Unkown character '%c' at %d:%d", next, state->row, state->col);
    }
  }
  return true;
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

bool lexWord(LexerState *state, char *word, TokenType type) {
  int startRow = state->row;
  int startCol = state->col;

  long len = strlen(word);
  char localStr[len + 1];
  char nextChar;
  for (int i = 0; i < len; i++) {
    nextChar = next(state);
    if (nextChar == '\0') {
      FAIL(state, "Expected \"%s\" at %d:%d", word, startRow, startCol);
    }
    localStr[i] = nextChar;
  }
  localStr[len] = '\0';

  if (strcmp(localStr, word) != 0) {
    FAIL(state, "Expected \"%s\" at %d:%d", word, startRow, startCol);
  }

  Token *token = TokenList_insertNew(state->tokenList);
  token->tokenType = type;
  token->col = startCol;
  token->row = startRow;
  return true;
}

bool lexTrue(LexerState *state) {
  TRY(lexWord(state, "true", TOKEN_BOOL_LITERAL));
  Token *currToken = &state->tokenList->tokens[state->tokenList->length - 1];
  currToken->data.TOKEN_BOOL_LITERAL.boolean = true;
  return true;
}

bool lexFalse(LexerState *state) {
  TRY(lexWord(state, "false", TOKEN_BOOL_LITERAL));
  Token *currToken = &state->tokenList->tokens[state->tokenList->length - 1];
  currToken->data.TOKEN_BOOL_LITERAL.boolean = false;
  return true;
}

void lexSingleChar(LexerState *state, TokenType type) {
  int startRow = state->row;
  int startCol = state->col;

  next(state);
  Token *token = TokenList_insertNew(state->tokenList);
  token->tokenType = type;
  token->row = startRow;
  token->col = startCol;
}

void lexNumber(LexerState *state) {
  int startRow = state->row;
  int startCol = state->col;
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
  token->row = startRow;
  token->col = startCol;
}

// TODO This does not handle escape characters (\n, \" etc)
bool lexString(LexerState *state) {
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

  if (eof(state) || next_char == '\n') {
    FAIL(state, "Unterminated string literal at %d:%d", startRow, startCol);
  }

  char *copiedStr = malloc(strLen + 1);
  memcpy(copiedStr, strStart, strLen);
  copiedStr[strLen] = '\0';

  token->data.TOKEN_STRING_LITERAL.string = copiedStr;
  token->row = startRow;
  token->col = startCol;
  return true;
}

void sprintTokenType(char *dest, TokenType type) {
  switch (type) {
    case TOKEN_NUMBER_LITERAL:
      sprintf(dest, "%s", "Number literal");
      break;

    case TOKEN_NULL_LITERAL:
      sprintf(dest, "%s", "Null literal");
      break;

    case TOKEN_BOOL_LITERAL:
      sprintf(dest, "%s", "Boolean literal");
      break;

    case TOKEN_STRING_LITERAL:
      sprintf(dest, "%s", "String literal");
      break;

    case TOKEN_COLON:
      sprintf(dest, "%s", ":");
      break;

    case TOKEN_COMMA:
      sprintf(dest, "%s", ",");
      break;

    case TOKEN_OPEN_SQUARE:
      sprintf(dest, "%s", "[");
      break;

    case TOKEN_CLOSE_SQUARE:
      sprintf(dest, "%s", "]");
      break;

    case TOKEN_OPEN_CURLY:
      sprintf(dest, "%s", "{");
      break;

    case TOKEN_CLOSE_CURLY:
      sprintf(dest, "%s", "}");
      break;
  }
}

void printTokenType(TokenType type) {
  char dest[32];
  sprintTokenType(dest, type);
  printf("%s", dest);
}

void printToken(Token *token) {
  switch (token->tokenType) {
    case TOKEN_NUMBER_LITERAL:
      printf("%d:%d numberLiteral(%f)", token->row, token->col, token->data.TOKEN_NUMBER_LITERAL.number);
      break;

    case TOKEN_STRING_LITERAL:
      printf("%d:%d stringLiteral(\"%s\")", token->row, token->col, token->data.TOKEN_STRING_LITERAL.string);
      break;

    case TOKEN_BOOL_LITERAL:
      printf("%d:%d boolLiteral(%s)", token->row, token->col, (token->data.TOKEN_BOOL_LITERAL.boolean ? "true" : "false"));
      break;

    case TOKEN_NULL_LITERAL:
      printf("%d:%d nullLiteral(null)", token->row, token->col);
      break;

    case TOKEN_OPEN_CURLY:
      printf("%d:%d openCurly( { )", token->row, token->col);
      break;

    case TOKEN_CLOSE_CURLY:
      printf("%d:%d closeCurly( } )", token->row, token->col);
      break;

    case TOKEN_OPEN_SQUARE:
      printf("%d:%d openSquare( [ )", token->row, token->col);
      break;

    case TOKEN_CLOSE_SQUARE:
      printf("%d:%d closeSquare( ] )", token->row, token->col);
      break;

    case TOKEN_COMMA:
      printf("%d:%d comma( , )", token->row, token->col);
      break;

    case TOKEN_COLON:
      printf("%d:%d colon( : )", token->row, token->col);
      break;
  }
}


