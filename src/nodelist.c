#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nodelist.h"
#include "parser.h"

NodeList *NodeList_new() {
  JSONNode *items = malloc(sizeof(JSONNode) * NODELIST_START_CAPACITY);
  NodeList list = {
    .items = items,
    .length = 0,
    .capcity = NODELIST_START_CAPACITY
  };
  NodeList *listPtr = malloc(sizeof(NodeList));
  *listPtr = list;
  return listPtr;
}

void NodeList_free(NodeList *ptr) {
  NodeList list = *ptr;
  for (int i = 0; i < list.length; i++) {
    _JSONNode_free(list.items + i, true);
  }
  free(ptr->items);
  free(ptr);
}

static void resize(NodeList *list) {
  int newCapacity = list->capcity * NODELIST_RESIZE_FACTOR;
  JSONNode *newItems = reallocarray(list->items, newCapacity, sizeof(JSONNode));
  list->items = newItems;
  list->capcity = newCapacity;
}

JSONNode *NodeList_insert(NodeList *list, JSONNode node) {
  int newLength = list->length + 1;
  if (newLength > list->capcity) {
    resize(list);
  }

  JSONNode *ptr = &list->items[list->length];
  *ptr = node;

  list->length = newLength;
  return ptr;
}

JSONNode *NodeList_insertNew(NodeList *list) {
  return NodeList_insert(list, (JSONNode){});
}


