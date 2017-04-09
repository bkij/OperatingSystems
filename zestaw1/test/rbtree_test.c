#include "../library/contact_book.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <malloc.h>

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
    printf("Checking tree rebuilding...\n");
    ContactTree *tree3 = tree_init(surname);

    ContactInfo *dummy_info3 = malloc(sizeof(ContactInfo));
    ContactInfo *dummy_info4 = malloc(sizeof(ContactInfo));

    strncpy(dummy_info3->surname, "Kowalski", strlen("Kowalski"));
    dummy_info3->surname[8] = '\0';
    strncpy(dummy_info3->birthdate, "04121995", strlen("04121995"));
    dummy_info3->birthdate[9] = '\0';

    strncpy(dummy_info4->surname, "Zalewski", strlen("Zalewski"));
    dummy_info4->surname[8] = '\0';
    strncpy(dummy_info4->birthdate, "01121995", strlen("01121995"));
    dummy_info4->birthdate[9] = '\0';

    tree_add(tree3, dummy_info3);
    tree_add(tree3, dummy_info4);

    printf("Rebuilding...\n");
    tree3 = tree_rebuild(tree3, birthdate);
    assert(tree3->root->contact_data == dummy_info4);
    assert(tree3->root->right->contact_data == dummy_info3);

    printf("Checking destruction with data...\n");
    destroy_tree_and_data(tree3);
    return 0;
}
