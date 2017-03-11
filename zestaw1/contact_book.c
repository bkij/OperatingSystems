#include <assert.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include "contact_book.h"

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
        if(comparator(current, min) < 0) {
            min = current;
        }
        current = current->next;
    }
    return min;
}

void insert(ContactNode **sorted_begin, ContactNode **sorted_end, ContactNode *min, int (*comparator)(ContactInfo *left, ContactInfo *right))
{
    ContactNode *current = *sorted_begin;
    while(current != (*sorted_end)->next, comparator(min->contact_data, current->contact_data) > 0) {
        current = current->next;
    }
    // Insert node at the beginning
    if(current == sorted_begin) {
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
    if(list->first == NULL) {
        first = malloc(sizeof(ContactNode));
        first->prev = NULL;
        first->next = NULL;
        first->contact_data = current_info;
        list->last = first;
    }
    else {
        list->last->next = malloc(sizeof(ContactNode));
        list->last->next->prev = list->last->next;
        list->last->next->next = NULL;
        list->last->next->contact_data = contact_info;
        list->last = list->last->next;
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
                removed = true;
                break;
            }
            current->prev->next = current->next;
            break;
        }
        current = current->next;
    }
    free(current);
    return removed;
}

ContactInfo *list_find(ContactList *list, int (*comparator)(ContactInfo *left, ContactInfo *right), char *search_key)
{
    ContactNode *current = list->first;
    while(current != NULL && comparator(search_key, current->contact_data)) {
        current = current->next;
    }
    return current == NULL ? NULL : current->contact_data;
}

void list_sort(ContactList *list, int (*comparator)(ContactInfo *left, ContactInfo *right))
{
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
}

void create_node_left(RBNode *node, ContactInfo *data)
{
    node->left = malloc(sizeof(RBNode));
    node->left->contact_data = data;
    node->left->parent = node;
    node->left->left = NULL;
    node->left->right = NULL;
    node->left->color = red;
}

void create_root(ContactTree *tree, ContactInfo *data)
{
    tree->root = malloc(sizeof(RBNode));
    tree->root->contact_data = data;
    tree->root->parent = NULL;
    tree->root->left = NULL;
    tree->root->right = NULL;
    tree->root->color = black;
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

// Deletion helpers


RBNode *fix_upwards(RBNode *current)
{
    if(is_red(current->right)) {
        rotate_left(current);
        current = current->parent;
    }
    if(is_red(current->left) && is_red(current->left->left)) {
        rotate_right(current);
        current = current->parent;
    }
    if(is_red(current->left) && is_red(current->right)) {
        flip_colors(current);
    }
    return current;
}

RBNode *move_red_left(RBNode *current)
{
    flip_colors(current);
    if(current->right != NULL && is_red(current->right->left)) {
        rotate_right(current->right->left);
        rotate_left(current->right);
        flip_colors(current);
    }

    return current;
}

RBNode *move_red_right(RBNode *current)
{
    flip_colors(current);
    if(current->left != NULL && is_red(current->left->left)) {
        rotate_right(current);
        current = current->parent;
        flip_colors(current);
    }

    return current;
}

RBNode *find_min(RBNode *current)
{
    while(current->left != NULL) {
        current = current->left;
    }

    return current;
}

RBNode *delete_min(RBNode *min)
{
    if(min->left == NULL) {
        free(min);
        return NULL;
    }
    if(!is_red(min->left) && !is_red(min->left->left)) {
        min = move_red_left(min);
    }
    min->left = delete_min(min->left);
    
    return fix_upwards(min);
}

RBNode *delete_node(RBNode *current, ContactInfo *data, int (*comparator)(ContactInfo *left, ContactInfo *right))
{
    if(comparator(data, current->contact_data) < 0) {
        if(current->left != NULL) {
            if(!is_red(current->left) && !is_red(current->left->left)) {
                current = move_red_left(current);
            }
            current->left = delete_node(current->left, data, comparator);
        }
    }
    else {
        if(is_red(current->left)) {
            rotate_right(current);
            current = current->parent;
        }
        if(!comparator(data, current->contact_data) && current->right == NULL) {
            free(current);
            return NULL;
        }
        if(current->right != NULL) {
            if(!is_red(current->right) && !is_red(current->right->left)) {
                current = move_red_right(current);
            }
            if(!comparator(data, current->contact_data)) {
                current->contact_data = find_min(current->right)->contact_data;
                current->right = delete_min(current->right);
            }
            else {
                current->right = delete_node(current->right, data, comparator);
            }
        }
    }
    return fix_upwards(current);
}


/*
 *  API FUNCTIONS
 */

// TODO: Update on data already in tree
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
            printf("Data already in the tree according to current key\n");
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
bool tree_remove(ContactTree *tree, ContactInfo *data)
{
    if(tree->root == NULL) {
        return false;
    }
    tree->root = delete_node(tree->root, data, tree->comparator);
    tree->root->color = black;
    return true;
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
            to_return = current->contact_data;
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
