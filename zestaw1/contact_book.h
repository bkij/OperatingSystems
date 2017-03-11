#ifndef CONTACT_BOOK_H
#define CONTACT_BOOK_H

#include <stdbool.h>
#include "contact_info.h"


/*
 * List data structure and functions
 */

typedef struct ContactNode {
    ContactInfo *contact_data;
    ContactNode *prev;
    ContactNode *next;
} ContactNode;

typedef struct ContactList {
    ContactNode *first;
    ContactNode *last;
}

void list_add(ContactList *contact_list, ContactInfo *contact_info);
bool list_remove(ContactList *contact_list, ContactInfo *contact_info);
ContactInfo *list_find(ContactList *contact_list, ContactInfo *contact_info);
void list_sort(ContactList *contact_list, int (*comparator)(ContactInfo *left, ContactInfo *right));

/*
 * Tree data structure and functions
 */

typedef enum {red, black} Color;
typedef enum {surname, birthdate, mail, phone} Key;

typedef struct RBNode {
    RBNode *parent;
    RBNode *left;
    RBNode *right;
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
}

ContactTree *tree_init(Key key);
void tree_add(ContactTree *contact_tree, ContactInfo *contact_info);
bool tree_remove(ContactTree *contact_tree, ContactInfo *contact_info);
ContactInfo *tree_find(ContactTree *contact_tree, Key key, char *search_key);
ContactTree *tree_rebuild(ContactTree *contact_tree, Key key); 
#endif
