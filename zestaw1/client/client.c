#include <stdio.h>
#include <time.h>
#include <sys/times.h>
#include "../library/contact_book.h"
#include "random_data.h"

#ifdef DYNAMIC

#include <dlfcn.h>

void *library = dlopen("libcontact_dynamic.so", RTLD_LAZY);

ContactList * (*list_init)() = dlsym(library, "list_init");
void (*list_add)(ContactList *, ContactList *) = dlsym(library, "list_add");
void (*list_remove)(ContactList *, ContactInfo *) = dlsym(library, "list_remove");
ContactInfo * (*list_find)(ContactList *, Key, char *) = dlsym(library, "list_find");
void (*list_sort)(ContactList *, Key) = dlsym(library, "list_sort");
void (*destroy_list_and_data)(ContactList *) = dlsym(library, "destroy_list_and_data");
void (*destroy_list)(ContactList *) = dlsym(library, "destroy_list");

ContactTree * (*tree_init)(Key) = dlsym(library, "tree_init");
void (*tree_add)(ContactTree *, ContactInfo *) = dlsym(library, "tree_add");
void (*tree_remove)(ContactTree *, ContactInfo *) = dlsym(library, "tree_remove");
ContactInfo * (*tree_find)(ContactTree *, Key, char *) = dlsym(library, "tree_find");
ContactTree * (*tree_rebuild)(ContactTree *, Key) = dlsym(library, "tree_rebuild");
void (*destroy_tree_and_data)(ContactTree *) = dlsym(library, "destroy_tree_and_data");
void (*destroy_tree)(ContactTree *) = dlsym(library, "destroy_tree");

#endif

void print_usage();
void run_tree_measurements();
void run_list_measurements();
ContactInfo *get_mock_info();

int main(int argc, char **argv)
{
    if(argc < 2) {
        print_usage();
        exit(EXIT_FAILURE);
    }
    if(!strncmp("tree", argv[1], 4) || (argv[2] != NULL && !strncmp("tree", argv[2], 4))) {
        run_tree_measurements();
    }
    if(!strncmp("list", argv[1], 4) || (argv[2] != NULL && !strncmp("list", argv[2], 4))) {
        run_list_measurements();
    }
    return 0;
}

void print_usage()
{
    pritnf("Usage: ./client [-tree] [-list]\n");
    printf("\n");
    printf("\tOptions: -tree: Run measurements for r-b tree\n");
    printf("\t         -list: Run measurements for doubly linked list\n");
}

void get_mock_info()
{
    ContactInfo *current_info = malloc(sizeof(ContactInfo));
    current_info->address = mock_address;
    strcpy(current_info->name, names[rand() % 100]);
    strcpy(current_info->surname, surnames[rand() % 100]);
    strcpy(current_info->birthdate, birthdates[rand() % 100]);
    strcpy(current_info->mail, emails[rand() % 100]);
    strcpy(current_info->phone, phones[rand() % 100]);
    return current_info;
}

void run_tree_measurements()
{
    ContactInfo *current_info, *optimistic, *pessimistic;
    srand(time(NULL));
    
    printf("Contact tree creation (1000 records):\n");
    ContactTree *tree = tree_init(surname);
    for(int i = 0; i < 1000; i++) {
        current_info = get_mock_info();
        // TODO: Measure time
        tree_add(tree, current_info);
    }

    printf("Adding new element:\n");
    printf("(i) Pessimistic\n");
    current_info = get_mock_info();
    strcpy(current_info->surname, "Zalewski");
    // TODO: Measure time
    tree_add(tree, current_info);
    printf("(ii) Optimistic\n");
    current_info = get_mock_info();
    strcpy(current_info->surname, "Abazur");
    // TODO: Measure time
    tree_add(tree, current_info);

    printf("Searching element:\n");
    printf("(i) Pessimistic\n");
    // TODO: Measure time
    pessimistic = tree_find(tree, surname, "Zalewski");
    printf("(ii) Optimistic\n");
    // TODO: Measure time
    optimistic = tree_find(tree, surname, "Abazur");

    printf("Deleting element:\n");
    printf("(i) Pessimistic\n");
    //TODO: Measure time
    tree_remove(tree, pessimistic);
    printf("(i) Optimistic\n");
    //TODO: Measure time
    tree_remove(tree, optimistic);

    printf("Rebuilding tree:\n");
    //TODO: Measure time
    tree = tree_rebuild(tree, email);

    destry_tree_and_data(tree);
}

void run_list_measurements()
{
    ContactInfo *current_info, *optimistic, *pessimistic;
    srand(time(NULL));

    printf("Contact list creation (1000) records):\n");
    ContactList *list = list_init();
    for(int i = 0; i < 1000; i++) {
        current_info = get_mock_info();
        // TODO: Measure time
        list_add(list, current_info);    
    }

    printf("Sorting list:\n");
    //TODO: Measure time
    list_sort(list, surname);

    printf("Adding new element:\n");
    printf("(i) Pessimistic\n");
    current_info = get_mock_info();
    strcpy(current_info->surname, "Zalewski");
    //TODO: Measure time
    list_add(list, current_info);
    printf("(ii) Optimistic\n");
    current_info = get_mock_info();
    strcpy(current_info->surname, "Abazur");
    // TODO: Measure time
    list_add(list, current_info);

    printf("Searching element\n");
    printf("(i) Pessimistic\n");
    // TODO: Measure time
    pessimistic = list_find(list, surname, "Zalewski");
    printf("(ii) Optimistic\n");
    // TODO: Measure time
    optimistic = list_find(list, surname, "Abazur");

    printf("Deleting element\n");
    printf("(i) Pessimistic\n");
    // TODO: Measure time
    list_remove(list, pessimistic);
    printf("(ii) Optimistic\n");
    //TODO: Measure time
    list_remove(list, optimistic);

    printf("Resorting list\n");
    // TODO: Measure time
    list_sort(list, phone);

    destroy_list_and_data(list);
}
