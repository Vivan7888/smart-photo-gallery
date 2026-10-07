#include "photo_ds.h"

#include <stdlib.h>
#include <string.h>

static void copy_record(PhotoRecord *destination, const PhotoRecord *source) {
    *destination = *source;
    destination->next = NULL;
}

void photo_list_init(PhotoList *list) {
    list->head = NULL;
    list->next_id = 1;
}

void photo_list_clear(PhotoList *list) {
    PhotoRecord *current = list->head;
    while (current != NULL) {
        PhotoRecord *next = current->next;
        free(current);
        current = next;
    }
    photo_list_init(list);
}

PhotoRecord *photo_list_add(PhotoList *list, const PhotoRecord *record) {
    PhotoRecord *node = (PhotoRecord *)malloc(sizeof(PhotoRecord));
    if (node == NULL) return NULL;

    copy_record(node, record);
    node->id = record->id > 0 ? record->id : list->next_id++;
    if (node->id >= list->next_id) list->next_id = node->id + 1;

    if (list->head == NULL) {
        list->head = node;
    } else {
        PhotoRecord *tail = list->head;
        while (tail->next != NULL) tail = tail->next;
        tail->next = node;
    }
    return node;
}

PhotoRecord *photo_list_find(PhotoList *list, int id) {
    PhotoRecord *current = list->head;
    while (current != NULL) {
        if (current->id == id) return current;
        current = current->next;
    }
    return NULL;
}

int photo_list_remove(PhotoList *list, int id) {
    PhotoRecord *previous = NULL;
    PhotoRecord *current = list->head;
    while (current != NULL) {
        if (current->id == id) {
            if (previous == NULL) list->head = current->next;
            else previous->next = current->next;
            free(current);
            return 1;
        }
        previous = current;
        current = current->next;
    }
    return 0;
}

int photo_list_count(const PhotoList *list) {
    int count = 0;
    const PhotoRecord *current = list->head;
    while (current != NULL) {
        ++count;
        current = current->next;
    }
    return count;
}

static void swap_photo_data(PhotoRecord *first, PhotoRecord *second) {
    PhotoRecord *first_next = first->next;
    PhotoRecord *second_next = second->next;
    PhotoRecord temp = *first;
    *first = *second;
    *second = temp;
    first->next = first_next;
    second->next = second_next;
}

void photo_list_sort_by_name(PhotoList *list) {
    for (PhotoRecord *a = list->head; a != NULL; a = a->next) {
        for (PhotoRecord *b = a->next; b != NULL; b = b->next) {
            if (strcmp(a->name, b->name) > 0) swap_photo_data(a, b);
        }
    }
}

void photo_list_sort_by_date(PhotoList *list) {
    for (PhotoRecord *a = list->head; a != NULL; a = a->next) {
        for (PhotoRecord *b = a->next; b != NULL; b = b->next) {
            if (strcmp(a->date_added, b->date_added) > 0) swap_photo_data(a, b);
        }
    }
}

void photo_list_sort_by_size(PhotoList *list) {
    for (PhotoRecord *a = list->head; a != NULL; a = a->next) {
        for (PhotoRecord *b = a->next; b != NULL; b = b->next) {
            if (a->original_size > b->original_size) swap_photo_data(a, b);
        }
    }
}

void photo_queue_init(PhotoQueue *queue) { queue->front = queue->rear = NULL; }

void photo_queue_clear(PhotoQueue *queue) {
    int ignored;
    while (photo_queue_dequeue(queue, &ignored)) { }
}

void photo_queue_enqueue(PhotoQueue *queue, int photo_id) {
    QueueNode *node = (QueueNode *)malloc(sizeof(QueueNode));
    if (node == NULL) return;
    node->photo_id = photo_id;
    node->next = NULL;
    if (queue->rear == NULL) queue->front = queue->rear = node;
    else { queue->rear->next = node; queue->rear = node; }
}

int photo_queue_dequeue(PhotoQueue *queue, int *photo_id) {
    if (queue->front == NULL) return 0;
    QueueNode *node = queue->front;
    *photo_id = node->photo_id;
    queue->front = node->next;
    if (queue->front == NULL) queue->rear = NULL;
    free(node);
    return 1;
}

void action_stack_init(ActionStack *stack) { stack->top = NULL; }

void action_stack_clear(ActionStack *stack) {
    int photo_id, action_type;
    while (action_stack_pop(stack, &photo_id, &action_type)) { }
}

void action_stack_push(ActionStack *stack, int photo_id, int action_type) {
    ActionNode *node = (ActionNode *)malloc(sizeof(ActionNode));
    if (node == NULL) return;
    node->photo_id = photo_id;
    node->action_type = action_type;
    node->next = stack->top;
    stack->top = node;
}

int action_stack_pop(ActionStack *stack, int *photo_id, int *action_type) {
    if (stack->top == NULL) return 0;
    ActionNode *node = stack->top;
    *photo_id = node->photo_id;
    *action_type = node->action_type;
    stack->top = node->next;
    free(node);
    return 1;
}
