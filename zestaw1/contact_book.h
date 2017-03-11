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

/*
 * ContactInfo poitner below is actually an
 * array of size data_size. That's because
 * I allow multiple values for the same key.
 * Those values are stored in a single node as this array.
 */
typedef struct RBNode {
    RBNode *parent;
    RBNode *left;
    RBNode *right;
    Color color;
    ContactInfo *contact_data;
    int data_size;
} RBNode;

typedef struct ContactTree {
    RBNode root;
}

void tree_add(ContactTree *contact_tree, ContactInfo *contact_info);
bool tree_remove(ContactTree *contact_tree, ContactInfo *contact_info);
ContactInfo *tree_find(ContactTree *contact_tree, ContactInfo *contact_info);

/*
 * Using an enum to signal the new key on which to rebuild the tree
 */
typedef enum {surname_rebuild, birthdate_rebuild, mail_rebuild, phone_rebuild} rebuild_case; 
void tree_rebuild(ContactTree *contact_tree, rebuild_case how);
#endif
