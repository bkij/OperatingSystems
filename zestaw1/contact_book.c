#include <assert.h>
#include <malloc.h>
#include "contact_book.h"

/*
 * List functions implementations
 */





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
    if(parent->left == current->child) {
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

/*
 * Invoked when the right and left children are red
 * to preserve invariants
 */
void flip_colors(RBNode *node)
{
    node->color = red;
    node->left->color = black;
    node->right->color = red;
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

void destroy_tree_and_data(ContactTree *contact_tree)
{
    RBNode *root = contact_tree->root;
    destroy_node_and_data(root);
    free(contact_tree_ptr);
}

void destroy_node_and_data(RBNode *node)
{
    if(node == NULL) {
        return;
    }
    destroy_node(node->left);
    destroy_node(node->right);
    free(node->data);
    free(node);
}

void destroy_tree(ContactTree *contact_tree)
{
    RBNode *root = contact_tree->root;
    destroy_node(root);
    free(contact_tree_ptr);
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
            contact_tree->comparator = by_mail
            break;
        case phone:
            contact_tree->comparator = by_phone;
            break;
        default:
            fprintf(stderr, "Fatal error - contact tree key not found. Aborting...");
            destroy_tree_and_data(&contact_tree);
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
}

void build_new_node(ContactTree *new_tree, RBNode *node)
{
    if(node == NULL) {
        return;
    }
    build_new_node(node->left);
    build_new_node(node->right);
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
            printf("Data already in the tree according to current key\n");
            return;
        }
    }
    // Fix the tree upwards
    while(current != tree->root) {
        if(!is_red(current->left) && is_red(current->right)) {
            rotate_left(current);
            current = current->parent;
        }
        if(is_red(current->left) && is_red(current->current->left)) {
            rotate_right(current);
            current = current->parent;
        }
        if(is_red(current->left) && is_red(current->right)) {
            flip_colors(current);
            current = current->parent;
        }
    }
    root->color = black;    // Root is always black
}

bool tree_remove(ContactTree *tree, ContactInfo *data)
{
    return false;   //TODO
}

// Tree find function assumes the given search key is valid, possible TODO: change
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
        if(tree->comparator(to_compare, current->data) > 0) {
            // Data bigger than current, go right
            current = current->right;
        }
        else if(tree->comparator(to_compare, current->data) < 0) {
            // Data smaller than current, go left
            current = current->left;
        }
        else {
            to_return = current->data;
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
