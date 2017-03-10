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

void add_contact_info(ContactList *contact_list, ContactInfo *contact_info);
bool remove_contact_info(ContactList *contact_list, ContactInfo *contact_info);
ContactInfo *find_contact_info(ContactList *contact_list, ContactInfo *contact_info);
void sort_contact_list(ContactList *contact_list, int (*comparator)(ContactInfo *left, ContactInfo *right));

/*
 * Tree data structure and functions
 */
#endif
