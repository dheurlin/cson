#include <stdio.h>
#include <stdlib.h>

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

void printToken(Token *token) {
  switch (token->tokenType) {
    case TOKEN_NUMBER_LITERAL:
      printf("numberLiteral(%f)\n", token->contents.number);
      break;

    case TOKEN_STRING_LITERAL:
      printf("stringLiteral(\"%s\")\n", token->contents.str);
      break;

    case TOKEN_BOOL_LITERAL:
      printf("boolLiteral(%s)\n", token->contents.str);
      break;

    case TOKEN_NULL_LITERAL:
      printf("nullLiteral(null)\n");
      break;

    case TOKEN_OPEN_CURLY:
      printf("openCurly( { )\n");
      break;

    case TOKEN_CLOSE_CURLY:
      printf("closeCurly( } )\n");
      break;

    case TOKEN_OPEN_SQUARE:
      printf("openSquare( [ )\n");
      break;

    case TOKEN_CLOSE_SQUARE:
      printf("closeSquare( ] )\n");
      break;

    case TOKEN_COMMA:
      printf("comma( , )\n");
      break;

    case TOKEN_COLON:
      printf("colon( : )\n");
      break;
  }
}

typedef struct {
  char *input;
  long input_length;
  int input_pos;
  Token tokens[MAX_NUM_TOKENS];
  int token_pos;
} LexerState;

char isDigit(char n) {
  return n >= '0' && n <= '9';
}

char isWhitespace(char n) {
  return n == ' ' || n == '\n';
}

char peek(LexerState *state) {
  return state->input[state->input_pos];
}

char eof(LexerState *state) {
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
    printf("Peeked: %c\n", next);

    if (isDigit(next)) {
      lexNumber(state);
    } else if (next == '"') {
      lexString(state);
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

size_t read_whole_file(FILE* fp, char *filename, char *dest) {
  if (fp == NULL) {
    DIE("File %s not found\n", filename);
  }

  size_t new_len = fread(dest, sizeof(char), MAXBUFLEN, fp);
  if (ferror( fp ) != 0) {
    DIE("Error reading file");
  }
  dest[new_len++] = '\0'; /* Just to be safe. */
  return new_len;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Expected filename as input\n");
    return 1;
  }

  FILE *fp;
  fp = fopen(argv[1], "r");
  char input[MAXBUFLEN + 1];
  size_t file_len = read_whole_file(fp, argv[2], input);

  LexerState state = {
    .input = input,
    .input_length = file_len,
    .input_pos = 0,
    .token_pos = 0,
  };

  lex(&state);

  for (int i = 0; i < state.token_pos; i++) {
    printToken(&(state.tokens[i]));
  }

  fclose(fp);
}
