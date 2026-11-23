#pragma once

typedef struct node {
    void *data;
    struct node *next;
} Node;

Node *make_node(void);

void destroy_node(Node* node);
