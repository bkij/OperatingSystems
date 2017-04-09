#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/times.h>
#include <unistd.h>
#include "random_data.h"
#include "../library/contact_book.h"

#define CLK sysconf(_SC_CLK_TCK)

#ifdef DYNAMIC

#include <dlfcn.h>
void run_tree_measurements_dynamic();
void run_list_measurements_dynamic();

#else

void run_tree_measurements();
void run_list_measurements();

#endif

void print_times(struct tms *begin, clock_t clock_curr_before);
void print_usage();
ContactInfo *get_mock_info();

int main(int argc, char **argv)
{
    if(argc < 2) {
        print_usage();
        exit(EXIT_FAILURE);
    }
    if(!strncmp("-tree", argv[1], 5) || (argv[2] != NULL && !strncmp("-tree", argv[2], 5))) {
        #ifdef DYNAMIC
        run_tree_measurements_dynamic();
        #else
        run_tree_measurements();
        #endif
    }
    if(!strncmp("-list", argv[1], 5) || (argv[2] != NULL && !strncmp("-list", argv[2], 5))) {
        #ifdef DYNAMIC
        run_list_measurements_dynamic();
        #else
        run_list_measurements();
        #endif
    }
    return 0;
}

void print_times(struct tms *begin, clock_t clock_before)
{
    struct tms end;
    times(&end);
    printf("Real: %.6f, System: %.6f, User: %.6f\n", (double)(clock() - clock_before) / CLOCKS_PER_SEC, (double)(end.tms_stime - begin->tms_stime) / CLK, (double)(end.tms_utime - begin->tms_utime) / CLK);
}

void print_usage()
{
    printf("Usage: ./client [-tree] [-list]\n");
    printf("\n");
    printf("\tOptions: -tree: Run measurements for r-b tree\n");
    printf("\t         -list: Run measurements for doubly linked list\n");
}

ContactInfo *get_mock_info()
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


#ifdef DYNAMIC
void run_tree_measurements_dynamic()
{
    void *library = dlopen("../library/libcontact_dynamic.so", RTLD_LAZY);
    
    ContactTree * (*tree_init)(Key);
    tree_init = dlsym(library, "tree_init");
    void (*tree_add)(ContactTree *, ContactInfo *);
    tree_add = dlsym(library, "tree_add");
    void (*tree_remove)(ContactTree *, ContactInfo *);
    tree_remove = dlsym(library, "tree_remove");
    ContactInfo * (*tree_find)(ContactTree *, Key, char *);
    tree_find = dlsym(library, "tree_find");
    ContactTree * (*tree_rebuild)(ContactTree *, Key);
    tree_rebuild = dlsym(library, "tree_rebuild");
    void (*destroy_tree_and_data)(ContactTree *);
    destroy_tree_and_data = dlsym(library, "destroy_tree_and_data");

    ContactInfo *current_info, *optimistic, *pessimistic;
    
    struct tms begin;
    clock_t clock_curr;
    srand(time(NULL));
    
    printf("Contact tree creation (1000 records):\n");
    ContactTree *tree = tree_init(surname);
    for(int i = 0; i < 1000; i++) {
        current_info = get_mock_info();
        times(&begin);
        clock_curr = clock();
        tree_add(tree, current_info);
        print_times(&begin, clock_curr);
    }

    printf("Adding new element:\n");
    printf("(i) Pessimistic\n");
    current_info = get_mock_info();
    strcpy(current_info->surname, "Zalewski");
    times(&begin);
    clock_curr = clock();
    tree_add(tree, current_info);
    print_times(&begin, clock_curr);
    printf("(ii) Optimistic\n");
    current_info = get_mock_info();
    strcpy(current_info->surname, "Abazur");
    times(&begin);
    clock_curr = clock();
    tree_add(tree, current_info);
    print_times(&begin, clock_curr);

    printf("Searching element:\n");
    printf("(i) Pessimistic\n");
    times(&begin);
    clock_curr = clock();
    pessimistic = tree_find(tree, surname, "Zalewski");
    print_times(&begin, clock_curr);
    printf("(ii) Optimistic\n");
    times(&begin);
    clock_curr = clock();
    optimistic = tree_find(tree, surname, "Abazur");
    print_times(&begin, clock_curr);

    printf("Deleting element:\n");
    printf("(i) Pessimistic\n");
    times(&begin);
    clock_curr = clock();
    tree_remove(tree, pessimistic);
    print_times(&begin, clock_curr);
    printf("(i) Optimistic\n");
    times(&begin);
    clock_curr = clock();
    tree_remove(tree, optimistic);
    print_times(&begin, clock_curr);

    printf("Rebuilding tree:\n");
    times(&begin);
    clock_curr = clock();
    tree = tree_rebuild(tree, mail);
    print_times(&begin, clock_curr);

    destroy_tree_and_data(tree);
    dlclose(library);
}


void run_list_measurements_dynamic()
{
    void *library = dlopen("../library/libcontact_dynamic.so", RTLD_LAZY);
    
    ContactList * (*list_init)();
    list_init = dlsym(library, "list_init");
    void (*list_add)(ContactList *, ContactInfo *);
    list_add = dlsym(library, "list_add");
    void (*list_remove)(ContactList *, ContactInfo *);
    list_remove = dlsym(library, "list_remove");
    ContactInfo * (*list_find)(ContactList *, Key, char *);
    list_find = dlsym(library, "list_find");
    void (*list_sort)(ContactList *, Key);
    list_sort = dlsym(library, "list_sort");
    void (*destroy_list_and_data)(ContactList *);
    destroy_list_and_data = dlsym(library, "destroy_list_and_data");

    ContactInfo *current_info, *optimistic, *pessimistic;
    struct tms begin;
    srand(time(NULL));
    clock_t clock_curr;

    printf("Contact list creation (1000) records):\n");
    ContactList *list = list_init();
    for(int i = 0; i < 1000; i++) {
        current_info = get_mock_info();
        times(&begin);
        clock_curr = clock();
        list_add(list, current_info);    
        print_times(&begin, clock_curr);
    }

    printf("Sorting list:\n");
    times(&begin);
    clock_curr = clock();
    list_sort(list, surname);
    print_times(&begin, clock_curr);

    printf("Adding new element:\n");
    printf("(i) Pessimistic\n");
    current_info = get_mock_info();
    strcpy(current_info->surname, "Zalewski");
    times(&begin);
    clock_curr = clock();
    list_add(list, current_info);
    print_times(&begin, clock_curr);
    printf("(ii) Optimistic\n");
    current_info = get_mock_info();
    strcpy(current_info->surname, "Abazur");
    times(&begin);
    clock_curr = clock();
    list_add(list, current_info);
    print_times(&begin, clock_curr);

    printf("Searching element\n");
    printf("(i) Pessimistic\n");
    times(&begin);
    clock_curr = clock();
    pessimistic = list_find(list, surname, "Zalewski");
    print_times(&begin, clock_curr);
    printf("(ii) Optimistic\n");
    times(&begin);
    clock_curr = clock();
    optimistic = list_find(list, surname, "Abazur");
    print_times(&begin, clock_curr);

    printf("Deleting element\n");
    printf("(i) Pessimistic\n");
    times(&begin);
    clock_curr = clock();
    list_remove(list, pessimistic);
    print_times(&begin, clock_curr);
    printf("(ii) Optimistic\n");
    times(&begin);
    clock_curr = clock();
    list_remove(list, optimistic);
    print_times(&begin, clock_curr);

    printf("Resorting list\n");
    times(&begin);
    clock_curr = clock();
    list_sort(list, phone);
    print_times(&begin, clock_curr);
  
    destroy_list_and_data(list); 
    dlclose(library);
}
#else

void run_tree_measurements()
{
    ContactInfo *current_info, *optimistic, *pessimistic;
    
    struct tms begin;
    clock_t clock_curr;
    srand(time(NULL));
    
    printf("Contact tree creation (1000 records):\n");
    ContactTree *tree = tree_init(surname);
    for(int i = 0; i < 1000; i++) {
        current_info = get_mock_info();
        times(&begin);
        clock_curr = clock();
        tree_add(tree, current_info);
        print_times(&begin, clock_curr);
    }

    printf("Adding new element:\n");
    printf("(i) Pessimistic\n");
    current_info = get_mock_info();
    strcpy(current_info->surname, "Zalewski");
    times(&begin);
    clock_curr = clock();
    tree_add(tree, current_info);
    print_times(&begin, clock_curr);
    printf("(ii) Optimistic\n");
    current_info = get_mock_info();
    strcpy(current_info->surname, "Abazur");
    times(&begin);
    clock_curr = clock();
    tree_add(tree, current_info);
    print_times(&begin, clock_curr);

    printf("Searching element:\n");
    printf("(i) Pessimistic\n");
    times(&begin);
    clock_curr = clock();
    pessimistic = tree_find(tree, surname, "Zalewski");
    print_times(&begin, clock_curr);
    printf("(ii) Optimistic\n");
    times(&begin);
    clock_curr = clock();
    optimistic = tree_find(tree, surname, "Abazur");
    print_times(&begin, clock_curr);

    printf("Deleting element:\n");
    printf("(i) Pessimistic\n");
    times(&begin);
    clock_curr = clock();
    tree_remove(tree, pessimistic);
    print_times(&begin, clock_curr);
    printf("(i) Optimistic\n");
    times(&begin);
    clock_curr = clock();
    tree_remove(tree, optimistic);
    print_times(&begin, clock_curr);

    printf("Rebuilding tree:\n");
    times(&begin);
    clock_curr = clock();
    tree = tree_rebuild(tree, mail);
    print_times(&begin, clock_curr);

    destroy_tree_and_data(tree);
}

void run_list_measurements()
{
    ContactInfo *current_info, *optimistic, *pessimistic;
    
    struct tms begin;
    srand(time(NULL));
    clock_t clock_curr;

    printf("Contact list creation (1000) records):\n");
    ContactList *list = list_init();
    for(int i = 0; i < 1000; i++) {
        current_info = get_mock_info();
        times(&begin);
        clock_curr = clock();
        list_add(list, current_info);    
        print_times(&begin, clock_curr);
    }

    printf("Sorting list:\n");
    times(&begin);
    clock_curr = clock();
    list_sort(list, surname);
    print_times(&begin, clock_curr);

    printf("Adding new element:\n");
    printf("(i) Pessimistic\n");
    current_info = get_mock_info();
    strcpy(current_info->surname, "Zalewski");
    times(&begin);
    clock_curr = clock();
    list_add(list, current_info);
    print_times(&begin, clock_curr);
    printf("(ii) Optimistic\n");
    current_info = get_mock_info();
    strcpy(current_info->surname, "Abazur");
    times(&begin);
    clock_curr = clock();
    list_add(list, current_info);
    print_times(&begin, clock_curr);

    printf("Searching element\n");
    printf("(i) Pessimistic\n");
    times(&begin);
    clock_curr = clock();
    pessimistic = list_find(list, surname, "Zalewski");
    print_times(&begin, clock_curr);
    printf("(ii) Optimistic\n");
    times(&begin);
    clock_curr = clock();
    optimistic = list_find(list, surname, "Abazur");
    print_times(&begin, clock_curr);

    printf("Deleting element\n");
    printf("(i) Pessimistic\n");
    times(&begin);
    clock_curr = clock();
    list_remove(list, pessimistic);
    print_times(&begin, clock_curr);
    printf("(ii) Optimistic\n");
    times(&begin);
    clock_curr = clock();
    list_remove(list, optimistic);
    print_times(&begin, clock_curr);

    printf("Resorting list\n");
    times(&begin);
    clock_curr = clock();
    list_sort(list, phone);
    print_times(&begin, clock_curr);

    destroy_list_and_data(list);
}
#endif