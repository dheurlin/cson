#ifndef LEXER_H
#define LEXER_H

#define MAXBUFLEN 1000000

#define TOKEN_MAX_LENGTH 256
#define MAX_NUM_TOKENS 10000

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

typedef struct {
  TokenType tokenType;
  union Data {
    double number;
    char str[TOKEN_MAX_LENGTH];
  } contents;
} Token;

typedef struct {
  char *input;
  long input_length;
  int input_pos;
  Token tokens[MAX_NUM_TOKENS];
  int token_pos;
} LexerState;

void lex(LexerState *state);

void printToken(Token *token);
#define printTokenLn(token) do { printToken(token); printf("\n"); } while(0);

#endif
