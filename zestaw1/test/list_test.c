#include "../library/contact_book.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

int main()
{
    printf("Checking init and dealloc...\n");
    ContactList *list = list_init();
    destroy_list(list);

    printf("Checking add and find...\n");
    ContactList *list2 = list_init();

    ContactInfo dummy_info;
    ContactInfo dummy_info2;
    strncpy(dummy_info.surname, "Kowalski", strlen("Kowalski"));
    strncpy(dummy_info2.surname, "Kondrat", strlen("Kondrat"));
    dummy_info.surname[8] = '\0';
    dummy_info2.surname[7] = '\0';

    list_add(list2, &dummy_info);
    list_add(list2, &dummy_info2);

    assert(list2->first->contact_data == &dummy_info);
    assert(list2->last->contact_data == &dummy_info2);
    assert(list2->first->next == list2->last);
    assert(list2->last->prev = list2->first);


    assert(list_find(list2, surname, "Kowalski") != NULL);
    assert(list_find(list2, surname, "Kowalski") == &dummy_info);
    assert(list_find(list2, surname, "Kondrat") != NULL);
    assert(list_find(list2, surname, "Kondrat") == &dummy_info2);

    printf("Checking delete...\n");
    list_remove(list2, &dummy_info);
    assert(list2->first == list2->last);
    assert(list2->first->contact_data == &dummy_info2);
    assert(list_find(list2, surname, "Kowalski") == NULL);

    destroy_list(list2);
    return 0;
}
