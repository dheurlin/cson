#include <stdlib.h>
#include "nodelist.h"

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

  list->items[list->length - 1] = node;
  list->length = newLength;

  return &(list->items[list->length - 1]);
}
