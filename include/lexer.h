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
} LexerState;

typedef struct {
  enum {
    LEXER_SUCCESS,
    LEXER_FAIL,
  } status;
  union {
    struct LEXER_SUCCESS { TokenList tokenList;         } LEXER_SUCCESS;
    struct LEXER_FAIL    { char errorMsg[MAX_ERR_SIZE]; } LEXER_FAIL;
  } result;
} LexResult;

// Does not take ownership of the input, caller must deallocate. On failure, deallocates its partial `TokenList`.
// On success, ownership of the `TokenList` transfers to the caller, who must deallocate it using `TokenList_free`
LexResult lex(char *input);

void printToken(Token *token);
void printTokenType(TokenType type);
// void sprintTokenType(char *dest, TokenType type);
char *tokenTypeToString(TokenType type);
#define printTokenLn(token) do { printToken(token); printf("\n"); } while(0);

void TokenList_free(TokenList *list);
struct Token *TokenList_insertNew(TokenList *list);
void TokenList_free(TokenList *list);

#endif
