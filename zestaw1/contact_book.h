#ifndef CONTACT_BOOK_H
#define CONTACT_BOOK_H

#include <stdbool.h>
#include "contact_info.h"


/*
 * List data structure and functions
 */

typedef struct ContactNode {
    ContactInfo contact_data;
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

#endif
