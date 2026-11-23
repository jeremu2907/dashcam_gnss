#include <stdlib.h>

#include "node.h"

Node *make_node(void)
{
    Node *node = malloc(sizeof *node);
    if (node == NULL)
        return NULL;

    node->data = NULL;
    node->next = NULL;
    return node;
}

void destroy_node(Node* node)
{
    free(node->data);
    node->next = NULL;
}