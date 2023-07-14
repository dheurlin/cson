#ifndef LEXER_H
#define LEXER_H

#include "stdbool.h"

#define TOKEN_START_CAPACITY 10

#define DIE(msg...) do { fprintf(stderr, msg); exit(1); } while(0);

typedef enum {
  TOKEN_STRING_LITERAL,
  TOKEN_NUMBER_LITERAL,
  TOKEN_BOOL_LITERAL,
  TOKEN_NULL_LITERAL,
  TOKEN_OPEN_CURLY,
  TOKEN_CLOSE_CURLY,
  TOKEN_OPEN_SQUARE,
  TOKEN_CLOSE_SQUARE,
  TOKEN_COMMA,
  TOKEN_COLON,
} TokenType;

typedef struct Token {
  TokenType tokenType;
  union {
    struct TOKEN_STRING_LITERAL { char *string;  } TOKEN_STRING_LITERAL;
    struct TOKEN_NUMBER_LITERAL { double number; } TOKEN_NUMBER_LITERAL;
    struct TOKEN_BOOL_LITERAL   { bool boolean;  } TOKEN_BOOL_LITERAL;
  } data;
  int row;
  int col;
} Token;

typedef struct TokenList {
  struct Token *tokens;
  int length;
  int capacity;
} TokenList;

#define MAX_ERR_SIZE 256

typedef struct {
  char *input;
  struct TokenList *tokenList;
  int row;
  int col;
  char errorMsg[MAX_ERR_SIZE];
  int status;
} LexerState;

typedef struct {
  int status;
  char errorMsg[MAX_ERR_SIZE];
  struct TokenList tokenList;
} LexResult;

// Does not take ownership of the input, caller must deallocate
LexResult lex(char *input);

void printToken(Token *token);
void printTokenType(TokenType type);
#define printTokenLn(token) do { printToken(token); printf("\n"); } while(0);

void TokenList_free(TokenList *list);
struct Token *TokenList_insertNew(TokenList *list);
void TokenList_free(TokenList *list);

#endif
