#ifndef NODELIST_H
#define NODELIST_H

#include "parser.h"

#define NODELIST_START_CAPACITY 10
#define NODELIST_RESIZE_FACTOR 2

typedef struct NodeList {
  JSONNode *items;
  int length;
  int capcity;
} NodeList;

NodeList *NodeList_new();
void NodeList_free(NodeList *ptr);
JSONNode *NodeList_insert(NodeList *list, JSONNode node);

#endif
