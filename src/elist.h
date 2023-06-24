#pragma once

#include <stddef.h>
#include <stdbool.h>

#define E_LIST_AT(_head, _index, _type) ((_type) e_list_at((_head), (_index)))

typedef struct elistnode eListNode;

struct elistnode
{
    void *data;

    eListNode *next;
};

void e_list_push(eListNode **head, void *data, size_t size);

void e_list_free(eListNode **head);

void *e_list_at(eListNode *head, size_t index);

size_t e_list_len(eListNode *head);
