#include <assert.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include "contact_book.h"

/*
 * Generic functions
 */

int (*get_comparator(Key key))(ContactInfo *, ContactInfo *)
{
    switch(key) {
        case surname:
            return by_surname;
        case mail:
            return by_mail;
        case phone:
            return by_phone;
        case birthdate:
            return by_birthdate;
        default:
            return NULL;
    }
}


ContactInfo *create_dummy_info(Key key, char *search_key)
{
    ContactInfo *dummy_info = malloc(sizeof(ContactInfo));
    switch(key) {
        case surname:
            strncpy(dummy_info->surname, search_key, sizeof(dummy_info->surname));
            dummy_info->surname[sizeof(dummy_info->surname) - 1] = '\0';
            break;
        case birthdate:
            strncpy(dummy_info->birthdate, search_key, sizeof(dummy_info->birthdate));
            dummy_info->birthdate[sizeof(dummy_info->birthdate) - 1] = '\0';
            break;
        case mail:
            strncpy(dummy_info->mail, search_key, sizeof(dummy_info->mail));
            dummy_info->mail[sizeof(dummy_info->mail) - 1] = '\0';
            break;
        case phone:
            strncpy(dummy_info->phone, search_key, sizeof(dummy_info->phone));
            dummy_info->phone[sizeof(dummy_info->phone) - 1] = '\0';
            break;
        //TODO: default?
    }
    return dummy_info;
}

/*
 * List functions implementations
 */

/*
 *  HELPER FUNCTIONS
 */

ContactNode *node_find_min(ContactNode *current, int (*comparator)(ContactInfo *left, ContactInfo *right))
{
    ContactNode *min = current;
    while(current != NULL) {
        if(comparator(current->contact_data, min->contact_data) < 0) {
            min = current;
        }
        current = current->next;
    }
    return min;
}

void insert(ContactNode **sorted_begin, ContactNode **sorted_end, ContactNode *min, int (*comparator)(ContactInfo *left, ContactInfo *right))
{
    ContactNode *current = *sorted_begin;
    while(current != (*sorted_end)->next && comparator(min->contact_data, current->contact_data) > 0) {
        current = current->next;
    }
    // Insert node at the beginning
    if(current == *sorted_begin) {
        sorted_begin = &min;
        (*sorted_begin)->next = current;
        current->prev = *sorted_begin;
        return;
    }
    // Insert node at the end
    if(current == (*sorted_end)->next) {
        (*sorted_end)->next = min;
        min->prev = *sorted_end;
        sorted_end = &min;
    }
    // Insert in the middle
    current->prev->next = min;
    min->next = current;
}

void destroy_list_and_data(ContactList *list)
{
    ContactNode *current = list->last;
    while(current->prev != NULL) {
        current = current->prev;
        free(current->next->contact_data);
        free(current->next);
    }
    free(list->first->contact_data);
    free(list->first);
    free(list);
}

void destroy_list(ContactList *list)
{
    ContactNode *current = list->last;
    while(current != NULL && current->prev != NULL) {
        current = current->prev;
        free(current->next);
    }
    if(current != NULL) {
        free(list->first);
    }
    free(list);
}

/*
 *  API FUNCTIONS
 */
ContactList *list_init()
{
    ContactList *list = malloc(sizeof(ContactList));
    list->first = NULL;
    list->last = NULL;
    return list;
}

void list_add(ContactList *list, ContactInfo *contact_info)
{
    ContactNode *tmp = malloc(sizeof(ContactNode));
    tmp->next = NULL;
    tmp->contact_data = contact_info;

    if(list->first == NULL) {
        list->first = tmp;
        list->first->prev = NULL;
        list->last = list->first;
    }
    else {
        tmp->prev = list->last;
        list->last->next = tmp;
        list->last = tmp;
    }
}

void list_remove(ContactList *list, ContactInfo *contact_info)
{
    ContactNode *current = list->first;
    while(current != NULL) {
        if(current->contact_data == contact_info) {
            if(current->prev == NULL) {
                // Contact info found at the beginning of the list
                list->first = current->next;
                list->first->prev = NULL;
                break;
            }
            if(current->next == NULL) {
                // Contact info found at the end of the list
                list->last = current->prev;
                list->last->next = NULL;
                break;
            }
            current->prev->next = current->next;
            current->next->prev = current->prev;
            break;
        }
        current = current->next;
    }
    if(current != NULL) {
        free(current);
    }
}

ContactInfo *list_find(ContactList *list, Key key, char *search_key)
{
    int (*comparator)(ContactInfo *, ContactInfo *) = get_comparator(key);
    assert(comparator != NULL);
    
    ContactInfo *search_key_container = create_dummy_info(key, search_key);
    ContactNode *current = list->first;
    while(current != NULL && comparator(search_key_container, current->contact_data) != 0) {
        current = current->next;
    }
    free(search_key_container);
    return current == NULL ? NULL : current->contact_data;
}

void list_sort(ContactList *list, Key key) 
{
    int (*comparator)(ContactInfo *, ContactInfo *) = get_comparator(key);
    assert(comparator != NULL);
    ContactNode *sorted_begin = list->first;
    ContactNode *sorted_end = list->first;
    while(sorted_end->next != NULL) {
        ContactNode *min = node_find_min(sorted_end->next, comparator);
        insert(&sorted_begin, &sorted_end, min, comparator);
    }
    list->first = sorted_begin;
    list->last = sorted_end;
}


/*
 * Red-black tree functions implementations
 *
 * Based on r-b tree as described by Sedgewick and Wayne
 * in Algorithms 4th edition. More precisely it is a
 * left leaning r-b tree
 *
 * http://algs4.cs.princeton.edu/home/
 */

/*
 *  HELPER FUNCTIONS
 */

// Generic helpers and local operations

bool is_red(RBNode *node)
{
    if(node == NULL) {
        // Leaves are black
        return black; 
    }
    return node->color == red;
}

/*
 * Exchanges current child with new child in parent
 */
void parent_set_child(RBNode *current_child, RBNode *new_child)
{
    RBNode *parent = current_child->parent;
    if(parent == NULL) {
        // Current child is root
        return;
    }
    if(parent->left == current_child) {
        parent->left = new_child;
    }
    else {
        parent->right = new_child;
    }
}

/*
 * Rotates a temporary node with a right red child
 * to a node with a left red child
 */
void rotate_left(RBNode *node)
{
    assert(is_red(node->right));

    RBNode *new_child = node->right;
    new_child->color = black;

    parent_set_child(node, new_child);
    
    node->right = new_child->left;
    new_child->left = node;
    node->parent = new_child;
    node->color = red;
}

/*
 * Rotates a temporary node with a left red child
 * to a node with a right red child
 *
 * Needed as a step before recoloring parent
 * and its children to black
 */
void rotate_right(RBNode *node)
{
    assert(is_red(node->left));

    RBNode *new_child = node->left;
    new_child->color = black;

    parent_set_child(node, new_child);

    node->left = new_child->right;
    new_child->right = node;
    node->parent = new_child;
    node->parent->right = node;
    node->color = red;
}

void flip_colors(RBNode *node)
{
    assert(node->color != node->left->color && node->left->color == node->right->color);
    node->color = node->color == red? black : red;
    node->left->color = node->left->color == red ? black : red;
    node->right->color = node->right->color == red ? black : red;
}

void create_node_right(RBNode *node, ContactInfo *data)
{
    node->right = malloc(sizeof(RBNode));
    node->right->contact_data = data;
    node->right->parent = node;
    node->right->left = NULL;
    node->right->right = NULL;
    node->right->color = red;
    node->right->deleted = false;
}

void create_node_left(RBNode *node, ContactInfo *data)
{
    node->left = malloc(sizeof(RBNode));
    node->left->contact_data = data;
    node->left->parent = node;
    node->left->left = NULL;
    node->left->right = NULL;
    node->left->color = red;
    node->left->deleted = false;
}

void create_root(ContactTree *tree, ContactInfo *data)
{
    tree->root = malloc(sizeof(RBNode));
    tree->root->contact_data = data;
    tree->root->parent = NULL;
    tree->root->left = NULL;
    tree->root->right = NULL;
    tree->root->color = black;
    tree->root->deleted = false;
}


// Tree destruction functions

void destroy_node_and_data(RBNode *node)
{
    if(node == NULL) {
        return;
    }
    destroy_node_and_data(node->left);
    destroy_node_and_data(node->right);
    free(node->contact_data);
    free(node);
}

void destroy_tree_and_data(ContactTree *contact_tree)
{
    RBNode *root = contact_tree->root;
    destroy_node_and_data(root);
    free(contact_tree);
}

void destroy_node(RBNode *node)
{
    if(node == NULL) {
        return;
    }
    destroy_node(node->left);
    destroy_node(node->right);
    free(node);
}

void destroy_tree(ContactTree *contact_tree)
{
    RBNode *root = contact_tree->root;
    destroy_node(root);
    free(contact_tree);
}

// Initialization and helper
void set_comparator(ContactTree *contact_tree)
{
    switch(contact_tree->key) {
        case surname:
            contact_tree->comparator = by_surname;
            break;
        case birthdate:
            contact_tree->comparator = by_birthdate;
            break;
        case mail:
            contact_tree->comparator = by_mail;
            break;
        case phone:
            contact_tree->comparator = by_phone;
            break;
        default:
            fprintf(stderr, "Fatal error - contact tree key not found. Aborting...");
            destroy_tree_and_data(contact_tree);
            exit(EXIT_FAILURE);
    }
}

ContactTree *tree_init(Key key)
{
    ContactTree *contact_tree = malloc(sizeof(ContactTree));
    contact_tree->root = NULL;
    contact_tree->key = key;
    set_comparator(contact_tree);

    return contact_tree;
}


// Rebuilding helpers
void build_new_node(ContactTree *new_tree, RBNode *node)
{
    if(node == NULL) {
        return;
    }
    build_new_node(new_tree, node->left);
    build_new_node(new_tree, node->right);
    tree_add(new_tree, node->contact_data);
}

void build_new_tree(ContactTree *new_tree, ContactTree *old_tree)
{
    build_new_node(new_tree, old_tree->root);
}

/*
 *  API FUNCTIONS
 */

void tree_add(ContactTree *tree, ContactInfo *data)
{
    if(tree->root == NULL) {
        create_root(tree, data);
        return;
    }
    RBNode *current = tree->root;
    while(true) {
        if(tree->comparator(data, current->contact_data) > 0) {
            // Data is bigger than current node's data, insert if right is NULL or go right
            if(current->right == NULL) {
                create_node_right(current, data);
                break;
            }
            current = current->right;
        }
        else if(tree->comparator(data, current->contact_data) < 0) {
            // Data is smaller than current node's data, insert if left is NULL or go right
            if(current->left == NULL) {
                create_node_left(current, data);
            }
            current = current->left;
        }
        else {
            // Given data already in the tree
            current->contact_data = data;
            current->deleted = false;
            return;
        }
    }
    // Fix the tree upwards
    while(current != tree->root) {
        if(!is_red(current->left) && is_red(current->right)) {
            rotate_left(current);
        }
        if(is_red(current->left) && is_red(current->left->left)) {
            rotate_right(current);
        }
        if(is_red(current->left) && is_red(current->right)) {
            flip_colors(current);
        }
        current = current->parent;
    }
    tree->root->color = black;    // Root is always black
}

//TODO: Change to void with equality on address instead of comparator
void tree_remove(ContactTree *tree, ContactInfo *data)
{
    if(tree->root == NULL) {
        return;
    }
    RBNode *current = tree->root;
    while(current != NULL) {
        if(tree->comparator(data, current->contact_data) > 0) {
            current = current->right;
        }
        else if (tree->comparator(data, current->contact_data) < 0) {
            current = current->left;
        }
        else {
            current->deleted = true;
            return;
        }
    }
}

// Tree find function assumes the given search key is valid
ContactInfo *tree_find(ContactTree *tree, Key key, char *search_key)
{
    if(tree->key != key) {
        fprintf(stderr, "Key given to search function and tree key don't match\n");
        return NULL;
    }
    ContactInfo *to_return = NULL;
    RBNode *current = tree->root;

    // Create temporary ContactInfo structure to re-use comparator functions
    ContactInfo *to_compare = create_dummy_info(key, search_key);
    while(current != NULL) {
        if(tree->comparator(to_compare, current->contact_data) > 0) {
            // Data bigger than current, go right
            current = current->right;
        }
        else if(tree->comparator(to_compare, current->contact_data) < 0) {
            // Data smaller than current, go left
            current = current->left;
        }
        else {
            if(current->deleted == true) {
                to_return = NULL;
            }
            else {
                to_return = current->contact_data;
            }
            break;
        }
    }

    free(to_compare);
    return to_return;
}

ContactTree *tree_rebuild(ContactTree *tree, Key key)
{
    ContactTree *new_tree = tree_init(key);
    build_new_tree(new_tree, tree);
    destroy_tree(tree);
    return new_tree;
}
