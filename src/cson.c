#include <stdio.h>
#include <stdlib.h>

#include "lexer.h"
#include "parser.h"

#define MAXBUFLEN 1000000

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

  TokenList tokens = lex(input);

  JSONNode *parsed = parse(tokens.tokens, tokens.length);
  printTree(parsed);

  JSONNode_free(parsed);
  TokenList_free(&tokens);
  fclose(fp);
}
