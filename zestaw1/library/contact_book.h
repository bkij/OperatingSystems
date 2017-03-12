#ifndef CONTACT_BOOK_H
#define CONTACT_BOOK_H

#include <stdbool.h>
#include "contact_info.h"

/*
 * Generic helpers
 */
typedef enum {surname, birthdate, mail, phone} Key;
int (*get_comparator(Key key))(ContactInfo *, ContactInfo*);

/*
 * List data structure and functions
 */

typedef struct ContactNode {
    ContactInfo *contact_data;
    struct ContactNode *prev;
    struct ContactNode *next;
} ContactNode;

typedef struct ContactList {
    ContactNode *first;
    ContactNode *last;
} ContactList;

ContactList *list_init();
void list_add(ContactList *list, ContactInfo *contact_info);
void list_remove(ContactList *list, ContactInfo *contact_info);
ContactInfo *list_find(ContactList *list, Key key, char *search_key);
void list_sort(ContactList *list, Key key);
void destroy_list_and_data(ContactList *list);
void destroy_list(ContactList *list);

/*
 * Tree data structure and functions
 */

typedef enum {red, black} Color;

typedef struct RBNode {
    struct RBNode *parent;
    struct RBNode *left;
    struct RBNode *right;
    Color color;
    ContactInfo *contact_data;
} RBNode;

/*
 * Tree contains a key and a comparator
 * for the key
 *
 * There can only be one ContactInfo data with
 * the given key at a time in the structue
 */
typedef struct ContactTree {
    RBNode *root;
    Key key;
    int (*comparator)(ContactInfo *left, ContactInfo *right);
} ContactTree;

ContactTree *tree_init(Key key);
void tree_add(ContactTree *contact_tree, ContactInfo *contact_info);
bool tree_remove(ContactTree *contact_tree, ContactInfo *contact_info);
ContactInfo *tree_find(ContactTree *contact_tree, Key key, char *search_key);
ContactTree *tree_rebuild(ContactTree *contact_tree, Key key); 
void destroy_tree_and_data(ContactTree *contact_tree);
void destroy_tree(ContactTree *contact_tree);
#endif
