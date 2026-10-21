#include "album_tree.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void copy_album_name(char *destination, const char *source) {
    strncpy(destination, source, ALBUM_NAME_LENGTH - 1);
    destination[ALBUM_NAME_LENGTH - 1] = '\0';
}

static AlbumNode *create_album_node(const char *album_name) {
    AlbumNode *node = (AlbumNode *)malloc(sizeof(AlbumNode));

    if (node == NULL) {
        return NULL;
    }

    copy_album_name(node->album_name, album_name);

    node->photos = NULL;
    node->parent = NULL;
    node->first_child = NULL;
    node->next_sibling = NULL;

    return node;
}

static void free_photo_ids(AlbumPhotoNode *head) {
    while (head != NULL) {
        AlbumPhotoNode *next = head->next;
        free(head);
        head = next;
    }
}

static void free_album_branch(AlbumNode *node) {
    if (node == NULL) {
        return;
    }

    AlbumNode *child = node->first_child;

    while (child != NULL) {
        AlbumNode *next_child = child->next_sibling;
        free_album_branch(child);
        child = next_child;
    }

    free_photo_ids(node->photos);
    free(node);
}

static AlbumNode *find_album_recursive(
    AlbumNode *node,
    const char *album_name
) {
    if (node == NULL) {
        return NULL;
    }

    if (strcmp(node->album_name, album_name) == 0) {
        return node;
    }

    AlbumNode *child = node->first_child;

    while (child != NULL) {
        AlbumNode *found = find_album_recursive(child, album_name);

        if (found != NULL) {
            return found;
        }

        child = child->next_sibling;
    }

    return NULL;
}

static int photo_exists_in_album(
    const AlbumNode *album,
    int photo_id
) {
    AlbumPhotoNode *current = album->photos;

    while (current != NULL) {
        if (current->photo_id == photo_id) {
            return 1;
        }

        current = current->next;
    }

    return 0;
}

static void display_recursive(const AlbumNode *node, int level) {
    if (node == NULL) {
        return;
    }

    for (int i = 0; i < level; i++) {
        printf("  ");
    }

    printf("- %s\n", node->album_name);

    AlbumNode *child = node->first_child;

    while (child != NULL) {
        display_recursive(child, level + 1);
        child = child->next_sibling;
    }
}

static int count_albums_recursive(const AlbumNode *node) {
    if (node == NULL) {
        return 0;
    }

    int count = 1;
    AlbumNode *child = node->first_child;

    while (child != NULL) {
        count += count_albums_recursive(child);
        child = child->next_sibling;
    }

    return count;
}

static int count_photos_recursive(const AlbumNode *node) {
    if (node == NULL) {
        return 0;
    }

    int count = 0;
    AlbumPhotoNode *photo = node->photos;

    while (photo != NULL) {
        count++;
        photo = photo->next;
    }

    AlbumNode *child = node->first_child;

    while (child != NULL) {
        count += count_photos_recursive(child);
        child = child->next_sibling;
    }

    return count;
}

void album_tree_init(AlbumTree *tree, const char *root_name) {
    if (tree == NULL) {
        return;
    }

    tree->root = create_album_node(root_name);
}

void album_tree_clear(AlbumTree *tree) {
    if (tree == NULL) {
        return;
    }

    free_album_branch(tree->root);
    tree->root = NULL;
}

AlbumNode *album_tree_find(
    const AlbumTree *tree,
    const char *album_name
) {
    if (tree == NULL || tree->root == NULL || album_name == NULL) {
        return NULL;
    }

    return find_album_recursive(tree->root, album_name);
}

AlbumNode *album_tree_add_child(
    AlbumTree *tree,
    const char *parent_name,
    const char *new_album_name
) {
    if (tree == NULL || tree->root == NULL ||
        parent_name == NULL || new_album_name == NULL ||
        new_album_name[0] == '\0') {
        return NULL;
    }

    if (album_tree_find(tree, new_album_name) != NULL) {
        return NULL;
    }

    AlbumNode *parent = album_tree_find(tree, parent_name);

    if (parent == NULL) {
        return NULL;
    }

    AlbumNode *new_node = create_album_node(new_album_name);

    if (new_node == NULL) {
        return NULL;
    }

    new_node->parent = parent;
    new_node->next_sibling = parent->first_child;
    parent->first_child = new_node;

    return new_node;
}

int album_tree_add_photo(
    AlbumTree *tree,
    const char *album_name,
    int photo_id
) {
    AlbumNode *album = album_tree_find(tree, album_name);

    if (album == NULL || photo_exists_in_album(album, photo_id)) {
        return 0;
    }

    AlbumPhotoNode *new_photo =
        (AlbumPhotoNode *)malloc(sizeof(AlbumPhotoNode));

    if (new_photo == NULL) {
        return 0;
    }

    new_photo->photo_id = photo_id;
    new_photo->next = album->photos;
    album->photos = new_photo;

    return 1;
}

int album_tree_remove_photo(
    AlbumTree *tree,
    const char *album_name,
    int photo_id
) {
    AlbumNode *album = album_tree_find(tree, album_name);

    if (album == NULL) {
        return 0;
    }

    AlbumPhotoNode *previous = NULL;
    AlbumPhotoNode *current = album->photos;

    while (current != NULL) {
        if (current->photo_id == photo_id) {
            if (previous == NULL) {
                album->photos = current->next;
            } else {
                previous->next = current->next;
            }

            free(current);
            return 1;
        }

        previous = current;
        current = current->next;
    }

    return 0;
}

int album_tree_remove_empty_album(
    AlbumTree *tree,
    const char *album_name
) {
    AlbumNode *album = album_tree_find(tree, album_name);

    if (album == NULL || album == tree->root ||
        album->photos != NULL || album->first_child != NULL) {
        return 0;
    }

    AlbumNode *parent = album->parent;
    AlbumNode *previous = NULL;
    AlbumNode *current = parent->first_child;

    while (current != NULL) {
        if (current == album) {
            if (previous == NULL) {
                parent->first_child = current->next_sibling;
            } else {
                previous->next_sibling = current->next_sibling;
            }

            free(current);
            return 1;
        }

        previous = current;
        current = current->next_sibling;
    }

    return 0;
}

void album_tree_display(const AlbumTree *tree) {
    if (tree == NULL || tree->root == NULL) {
        printf("Album tree is empty.\n");
        return;
    }

    display_recursive(tree->root, 0);
}

int album_tree_count_albums(const AlbumTree *tree) {
    if (tree == NULL) {
        return 0;
    }

    return count_albums_recursive(tree->root);
}

int album_tree_count_photos(const AlbumTree *tree) {
    if (tree == NULL) {
        return 0;
    }

    return count_photos_recursive(tree->root);
}