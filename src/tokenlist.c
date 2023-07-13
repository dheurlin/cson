#include "tokenlist.h"
#include <stdlib.h>

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

