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
void left_rotate(RBNode *node)
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
            destroy_deallocate(contact_tree);
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
}

ContactInfo *tree_find(
