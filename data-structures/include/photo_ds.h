#ifndef PHOTO_DS_H
#define PHOTO_DS_H

#ifdef __cplusplus
extern "C" {
#endif

#define PHOTO_NAME_LENGTH 260
#define PHOTO_PATH_LENGTH 1024
#define PHOTO_DATE_LENGTH 16
#define PHOTO_ALBUM_LENGTH 100

typedef struct PhotoRecord {
    int id;
    char name[PHOTO_NAME_LENGTH];
    char file_path[PHOTO_PATH_LENGTH];
    char date_added[PHOTO_DATE_LENGTH];
    char archive_path[PHOTO_PATH_LENGTH];
    char album_name[PHOTO_ALBUM_LENGTH];
    long long original_size;
    long long stored_size;
    int archived;
    struct PhotoRecord *next;
} PhotoRecord;

typedef struct {
    PhotoRecord *head;
    int next_id;
} PhotoList;

typedef struct QueueNode {
    int photo_id;
    struct QueueNode *next;
} QueueNode;

typedef struct {
    QueueNode *front;
    QueueNode *rear;
} PhotoQueue;

typedef struct ActionNode {
    int photo_id;
    int action_type;
    struct ActionNode *next;
} ActionNode;

typedef struct {
    ActionNode *top;
} ActionStack;

enum PhotoActionType {
    ACTION_ARCHIVE = 1
};

void photo_list_init(PhotoList *list);
void photo_list_clear(PhotoList *list);
PhotoRecord *photo_list_add(PhotoList *list, const PhotoRecord *record);
PhotoRecord *photo_list_find(PhotoList *list, int id);
int photo_list_remove(PhotoList *list, int id);
int photo_list_count(const PhotoList *list);
void photo_list_sort_by_name(PhotoList *list);
void photo_list_sort_by_date(PhotoList *list);
void photo_list_sort_by_size(PhotoList *list);

void photo_queue_init(PhotoQueue *queue);
void photo_queue_clear(PhotoQueue *queue);
void photo_queue_enqueue(PhotoQueue *queue, int photo_id);
int photo_queue_dequeue(PhotoQueue *queue, int *photo_id);

void action_stack_init(ActionStack *stack);
void action_stack_clear(ActionStack *stack);
void action_stack_push(ActionStack *stack, int photo_id, int action_type);
int action_stack_pop(ActionStack *stack, int *photo_id, int *action_type);

#ifdef __cplusplus
}
#endif

#endif
