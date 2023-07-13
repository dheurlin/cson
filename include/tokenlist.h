#ifndef TOKENLIST_H
#define TOKENLIST_H

#include "lexer.h"

typedef struct TokenList {
  Token *tokens;
  int length;
  int capacity;
} TokenList;

void TokenList_free(TokenList *list);
Token *TokenList_insertNew(TokenList *list);
void TokenList_free(TokenList *list);


#endif
