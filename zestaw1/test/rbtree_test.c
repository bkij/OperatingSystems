#include "../library/contact_book.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

int main()
{
    printf("Checking init and dealloc...\n");
    ContactTree *tree = tree_init(surname);
    assert(tree->key == surname);
    destroy_tree(tree);

    printf("Checking add and find...\n");
    ContactTree *tree2 = tree_init(surname);
    
    ContactInfo dummy_info;
    ContactInfo dummy_info2;
    strncpy(dummy_info.surname, "Kowalski", strlen("Kowalski"));
    strncpy(dummy_info2.surname, "Kondrat", strlen("Kondrat"));
    dummy_info.surname[8] = '\0';
    dummy_info2.surname[7] = '\0';


    tree_add(tree2, &dummy_info);
    tree_add(tree2, &dummy_info2);

    assert(tree_find(tree2, surname, "Kowalski") != NULL);
    assert(tree_find(tree2, surname, "Kowalski") == &dummy_info);
    assert(tree_find(tree2, surname, "Kondrat") != NULL);
    assert(tree_find(tree2, surname, "Kondrat") == &dummy_info2);

    printf("Checking delete...\n");
    tree_remove(tree2, &dummy_info);
    assert(tree_find(tree2, surname, "Kowalski") == NULL);

    destroy_tree(tree2);
    return 0;
}
