#include "elist.h"
#include <stdlib.h>
#include <string.h>

void e_list_push(eArena *arena, eListNode **head, void *data, size_t size)
{
    eListNode *node = e_arena_alloc(arena, sizeof(eListNode));
    node->data = e_arena_alloc(arena, size);
    memcpy(node->data, data, size);
    node->next = NULL;

    if(*head == NULL)
    {
        *head = node;

        return;
    }

    eListNode *current = *head;
    while(current->next != NULL)
    {
        current = current->next;
    }

    current->next = node;
}

void *e_list_at(eListNode *head, size_t index)
{
    size_t counter = 0;
    eListNode *current = head;
    while(current != NULL)
    {
        if(counter == index)
        {
            return current->data;
        }
        
        current = current->next;
        counter++;
    }

    return NULL;
}

size_t e_list_len(eListNode *head)
{
    size_t counter = 0;
    eListNode *current = head;
    while(current != NULL)
    {
        current = current->next;
        counter++;
    }

    return counter;
}
