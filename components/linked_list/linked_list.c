#include <stdlib.h>

#include "esp_log.h"

#include "linked_list.h"

#define TAG "linked_list"

LinkedList *make_linked_list(int node_type_id)
{
    LinkedList *linked_list = malloc(sizeof(LinkedList));
    linked_list->front = NULL;
    linked_list->back = NULL;
    linked_list->node_type_id = node_type_id;
    linked_list->size = 0;

    return linked_list;
}

bool is_linked_list_corrupted(LinkedList *linked_list)
{
    if (linked_list == NULL)
    {
        ESP_LOGE(TAG, "List is NULL");
        return true;
    }

    Node *front = linked_list->front;
    Node *back = linked_list->back;

    if (linked_list->size == 0)
    {
        if (front != NULL || back != NULL)
        {
            ESP_LOGE(TAG, "Empty list has a front or back node");
            return true;
        }
        return false;
    }

    if (front == NULL || back == NULL || back->next != NULL)
    {
        ESP_LOGE(TAG, "Invalid endpoints for a nonempty list");
        return true;
    }

    if (linked_list->size == 1)
    {
        if (front != back)
        {
            ESP_LOGE(TAG, "Single-node list has different front and back");
            return true;
        }
    }
    else if (front == back || front->next == NULL)
    {
        ESP_LOGE(TAG, "Multi-node list has no distinct second node");
        return true;
    }

    return false;
}

void push_back_take(LinkedList *linked_list,
                    Node **node,
                    int node_type_id)
{
    if (is_linked_list_corrupted(linked_list))
    {
        return;
    }

    if (node == NULL || *node == NULL)
    {
        ESP_LOGE(TAG, "Expected a list and a node to append");
        return;
    }

    if (node_type_id != linked_list->node_type_id)
    {
        ESP_LOGE(TAG, "Node type or size does not match the list");
        return;
    }

    Node *incoming = *node;
    if (linked_list->size == 0)
    {
        linked_list->front = incoming;
    }
    else
    {
        linked_list->back->next = incoming;
    }

    linked_list->back = incoming;
    linked_list->size++;
    *node = NULL;
}

void pop_front(LinkedList *linked_list)
{
    if (is_linked_list_corrupted(linked_list))
    {
        return;
    }

    if (linked_list->size == 0)
    {
        return;
    }

    if (linked_list->size == 1)
    {
        linked_list->back = NULL;
    }

    Node *front = linked_list->front;
    linked_list->front = front->next;
    linked_list->size--;

    front->next = NULL;
    destroy_node(front);
}

void clear_linked_list(LinkedList *linked_list)
{
    if (is_linked_list_corrupted(linked_list))
    {
        return;
    }

    while (linked_list->size > 0)
    {
        pop_front(linked_list);
    }
}

void destroy_linked_list(LinkedList **linked_list)
{
    clear_linked_list(*linked_list);
    free(*linked_list);
    linked_list = NULL;
}
