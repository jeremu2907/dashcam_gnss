#pragma once

#include <stddef.h>
#include "node.h"

#define LINKED_LIST_NODE_TYPE_ID_DEFAULT -1
#define LINKED_LIST_SIZE_DEFAULT 0

typedef struct linked_list
{
    Node *front;
    Node *back;

    int node_type_id;

    size_t size;
} LinkedList;

LinkedList *make_linked_list(int node_type_id);

void push_back_take(LinkedList *linked_list,
                    Node **node,
                    int node_type_id);

void pop_front(LinkedList *linked_list);

void clear_linked_list(LinkedList *linked_list);

void destroy_linked_list(LinkedList **linked_list);