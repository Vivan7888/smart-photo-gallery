#ifndef ALBUM_TREE_H
#define ALBUM_TREE_H

#ifdef __cplusplus
extern "C" {
#endif

#define ALBUM_NAME_LENGTH 100

/* Stores one photo ID inside an album node. */
typedef struct AlbumPhotoNode {
    int photo_id;
    struct AlbumPhotoNode *next;
} AlbumPhotoNode;

/*
   N-ary tree node using First-Child / Next-Sibling representation.
*/
typedef struct AlbumNode {
    char album_name[ALBUM_NAME_LENGTH];

    AlbumPhotoNode *photos;

    struct AlbumNode *parent;
    struct AlbumNode *first_child;
    struct AlbumNode *next_sibling;
} AlbumNode;

typedef struct {
    AlbumNode *root;
} AlbumTree;

/* Tree setup and cleanup */
void album_tree_init(AlbumTree *tree, const char *root_name);
void album_tree_clear(AlbumTree *tree);

/* Tree operations */
AlbumNode *album_tree_find(const AlbumTree *tree, const char *album_name);

AlbumNode *album_tree_add_child(
    AlbumTree *tree,
    const char *parent_name,
    const char *new_album_name
);

/* Photo operations inside album nodes */
int album_tree_add_photo(
    AlbumTree *tree,
    const char *album_name,
    int photo_id
);

int album_tree_remove_photo(
    AlbumTree *tree,
    const char *album_name,
    int photo_id
);

/* Removes an album only if it has no photos and no child albums. */
int album_tree_remove_empty_album(
    AlbumTree *tree,
    const char *album_name
);

/* Tree traversal for testing */
void album_tree_display(const AlbumTree *tree);

/* Statistics */
int album_tree_count_albums(const AlbumTree *tree);
int album_tree_count_photos(const AlbumTree *tree);

#ifdef __cplusplus
}
#endif

#endif