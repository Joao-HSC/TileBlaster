#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "vars.h"
#include "stack.h"
#include "tiles.h"
#include "mem.h"

/******************************************************************************
 * extract_path_2()
 *
 * Arguments: node - leaf node
 *
 * Returns: the head of the path list
 *
 * Description: Find the path back to the root knowing the final node for the 2nd variant
 *****************************************************************************/

Coordinates_plus *extract_path_2(Node *current, int n_rows){

    Coordinates_plus *path_head = NULL;
    Coordinates_plus *aux = NULL;
    Node *prev = NULL;
    /* create a list with the instructions to follow while tracing back the tree */
    while (current != NULL)
    {
        Coordinates_plus *path_node = (Coordinates_plus *)malloc(sizeof(Coordinates_plus));
        path_node->row = current->coordinates->row;
        path_node->col = current->coordinates->col;
        path_node->score = current->score;
        path_node->next = path_head;
        path_head = path_node;
        prev = current;
        current = current->parent;
        while (prev->children != NULL)
        {
            aux = prev->children;
            prev->children = aux->next;
            free(aux);
        }
        free(prev->coordinates);
        free_tileset(prev->tileset, n_rows);
        free(prev);
    }

    return path_head;
}

/******************************************************************************
 * extract_path_3()
 *
 * Arguments: node - leaf node
 *
 * Returns: the head of the path list
 *
 * Description: Find the path back to the root knowing the final node for the 3rd variant
 *****************************************************************************/

Coordinates_plus *extract_path_3(Node *current){

    Coordinates_plus *path_head = NULL;
    /* create a list with the instructions to follow while tracing back the tree */
    while (current != NULL)
    {
        Coordinates_plus *path_node = (Coordinates_plus *)malloc(sizeof(Coordinates_plus));
        path_node->row = current->coordinates->row;
        path_node->col = current->coordinates->col;
        path_node->score = current->score; 
        path_node->next = path_head;
        path_head = path_node;
        current = current->parent;
    }

    return path_head;
}

/******************************************************************************
 * create_child()
 *
 * Arguments: current - node we're exploring, node_aux - element of the children list, n_rows - number of rows,
 * n_cols - number of columns, visited - boolean matrix
 *
 * Returns:
 *
 * Description: creates a new node
 *****************************************************************************/

Node *create_child(Node *current, Coordinates_plus *node_aux, int n_rows, int n_cols, bool **visited){

    int** aux = NULL;

    Node *child = (Node *)malloc(sizeof(Node));
    child->parent = current;
    child->coordinates = (Coordinates *)malloc(sizeof(Coordinates));
    child->coordinates->row = node_aux->row;
    child->coordinates->col = node_aux->col;

    aux = alloc_tileset(n_rows, n_cols);

    for (int i = 0; i < n_rows; i++) {
        for (int j = 0; j < n_cols; j++) {
            aux[i][j] = current->tileset[i][j];
        }
    }

    aux = gravity(coords_replace(aux, node_aux->row, node_aux->col, n_rows, n_cols), n_rows, n_cols);

    child->tileset = aux;
    child->children = coords_list(child->tileset, visited, n_rows, n_cols);
    reset_visit(visited, n_rows, n_cols);
    child->score = current->score + node_aux->score;
    child->potential = best_score_possible(child->tileset, n_rows, n_cols);
    return child;
}

/******************************************************************************
 * next_branch()
 *
 * Arguments: current - leaf node, n_rows - number of rows in each tileset
 *
 * Returns: the next node we're able to investigate which has not been analyzed yet
 *
 * Description: will start a new branch so we can find an alternative path which
 * will give us a different score while freeing the nodes we're not using anymore
 *****************************************************************************/

Node *next_branch(Node *current, int n_rows) {
    Coordinates_plus *aux = NULL;
    Node *parent = NULL;

    /* while we don't reach the parent of the head_node (NULL), backtrack */
    while (current != NULL) {
        parent = current->parent;

        /* if there's children associated with a node, free one and explore the next */
        if (current->children != NULL) {
            if (current->children->next != NULL) {
                aux = current->children;
                current->children = current->children->next;
                free(aux);
                return current;
            }
            free(current->children);
        }

        /* if we reach the head_node and there's no more kids, return NULL */
        if (parent == NULL) {
            free(current->coordinates);
            free_tileset(current->tileset, n_rows);
            free(current);
            return NULL;
        }

        /* if there's no children associated with a node or all children have been explored, backtrack */
        free(current->coordinates);
        free_tileset(current->tileset, n_rows);
        free(current);

        /* move up the tree */
        current = parent;
    }

    return NULL;  
}


/******************************************************************************
 * var_1()
 *
 * Arguments: tileset - initial tileset, visited - boolean matrix , n_rows - number of rows in the tileset,
 *  n_columns - number of columns in the tileset
 *
 * Returns: A list with the coordinates that have been broken until the tileset is unsolvable
 *
 * Description: Just break everything
 *****************************************************************************/
Coordinates_plus *var_1(int **tileset, bool **visited, int n_rows, int n_cols)
{

    int restart = 1;
    int aux = 0;
    Coordinates_plus *current = NULL;
    Coordinates_plus *head_coords = NULL;

    while (1){
        restart = 0;
        /* analyze the matrix every square or so (almost like in a chess manner) */
        for (int i = 0; i < n_rows; i++){
            if (restart == 1)
                break;

            for (int j = (i % 2 == 0) ? 1 : 0; j < n_cols; j += 2){
                aux = coords_analyze(tileset, visited, i, j, n_rows, n_cols);

                /* if there's a stain make a list with the coordinates of that stain */
                if (aux > 1){
                    restart = 1;

                    if (current == NULL){
                        current = (Coordinates_plus *)malloc(sizeof(Coordinates_plus));
                        if (current == NULL){
                            exit(1);
                        }

                        head_coords = current;
                    }
                    else{
                        current->next = (Coordinates_plus *)malloc(sizeof(Coordinates_plus));
                        if (current->next == NULL){
                            exit(1);
                        }
                        current = current->next;
                    }

                    current->row = i;
                    current->col = j;
                    current->score = score(aux);
                    current->next = NULL;
                    /* refresh the tileset with the broken stain */
                    tileset = coords_replace(tileset, i, j, n_rows, n_cols);
                    tileset = gravity(tileset, n_rows, n_cols);
                    visited = reset_visit(visited, n_rows, n_cols);
                }
            }
        }

        if (restart == 0){
            break;
        }
    }
    return head_coords;
}

/******************************************************************************
 * dfs_2()
 *
 * Arguments: tileset - initial tileset, v - min score, n_rows - number of rows in the tileset,
 *  n_columns - number of columns in the tileset, visited - boolean matrix
 *
 * Returns: A list with the coordinates of the path to the minimum v score
 *
 * Description: Find the leaf which corresponds to the minimum score and extract its path
 *****************************************************************************/

Coordinates_plus* dfs_2(int **tileset, int v, int n_rows, int n_cols, bool **visited){

    if(v > best_score_possible(tileset, n_rows, n_cols)){
        free_tileset(tileset, n_rows);
        return NULL;
    }

    Node *head_node = NULL;
    Node *current = NULL;
    Coordinates_plus *aux = NULL;

    /* initialize head_node */
    current = (Node *)malloc(sizeof(Node));
    head_node = current;
    head_node->parent = NULL;
    head_node->coordinates = (Coordinates *)malloc(sizeof(Coordinates));
    head_node->coordinates->row = 0;
    head_node->coordinates->col = 0;
    head_node->tileset = tileset;
    head_node->score = 0;
    head_node->potential = best_score_possible(tileset, n_rows, n_cols);
    head_node->children = coords_list(head_node->tileset, visited, n_rows, n_cols);
    reset_visit(visited, n_rows, n_cols);
    
    /* if the head_node has no children it means there's nothing we can do */
    if (head_node->children == NULL)
    {
        free(head_node->coordinates);
        free_tileset(head_node->tileset, n_rows);
        free(head_node);
        return NULL;
    }
    /* creates a child to the head node and we analyze it after */
    head_node->child = create_child(head_node, head_node->children, n_rows, n_cols, visited);
    current = current->child;

    /* while none of the conditions inside is met we'll keep backtracking and branching the tree */
    while (1){
        /*
         * if we find a leaf node we'll check its score,
         * if it's greater or equal than the the minimum score, we'll retrieve its path
         * else we'll try to find a new branch
         */
        if (current->children == NULL){
            if (current->score >= v){
                return extract_path_2(current, n_rows);
            }
            /* 
            * if the backtracking returns a new node with children, start creating the new branch
            * else return the list with the instrucitons for a set of plays 
            * which will give us a score greater or equal than v
            */
            current = next_branch(current, n_rows);
            if (current == NULL){
                return NULL;
            }
            if(v < current->potential + current->score){
            current->child = create_child(current, current->children, n_rows, n_cols, visited);
            current = current->child; 
            }
        }
         if(v < current->potential + current->score){
            current->child = create_child(current, current->children, n_rows, n_cols, visited);
            current = current->child; 
        }
        else{
            while(current->children != NULL){
                    aux = current->children;
                    current->children = current->children->next;
                    free(aux);
                }
            current = next_branch(current, n_rows);
            if (current == NULL){
                return NULL;
            }
        }
        
    }
}

/******************************************************************************
 * dfs_3()
 *
 * Arguments: tileset - initial tileset, n_rows - number of rows in the tileset,
 *  n_columns - number of columns in the tileset, visited - boolean matrix
 *
 * Returns: A list with the coordinates of the path to the max score
 *
 * Description: Find the leaf which corresponds to the max score and extract its path
 *****************************************************************************/

Coordinates_plus* dfs_3(int **tileset, int n_rows, int n_cols, bool **visited){

    Node *head_node = NULL;
    Node *current = NULL;
    Coordinates_plus *best_path = NULL;
    Coordinates_plus *aux = NULL;
    int max_score = -1;

    /* initialize head_node */
    head_node = (Node *)malloc(sizeof(Node));
    current = head_node;
    head_node->parent = NULL;
    head_node->child = NULL;
    head_node->coordinates = (Coordinates *)malloc(sizeof(Coordinates));
    head_node->coordinates->row = 0;
    head_node->coordinates->col = 0;
    head_node->tileset = tileset;
    head_node->score = 0;
    head_node->potential = best_score_possible(tileset, n_rows, n_cols);
    head_node->children = coords_list(head_node->tileset, visited, n_rows, n_cols);
    reset_visit(visited, n_rows, n_cols);

    /* if the head_node has no children it means there's nothing we can do */
    if (head_node->children == NULL){
        free(head_node->coordinates);
        free_tileset(head_node->tileset, n_rows);
        free(head_node);
        return NULL;
    }
    
    current->child = create_child(current, current->children, n_rows, n_cols, visited);
    current = current->child;

    /* while none of the conditions inside is met we'll keep backtracking and branching the tree */
    while (1){
        /*
         * if we find a leaf node we'll check its score to see if it's greater than max_score
         * if so, we'll retrieve its path and find a new branch
         * else we'll just try to find a new branch
         */
        if (current->children == NULL){
            if (current->score > max_score){
                max_score = current->score;

                /* free the previous best path */
                while (best_path != NULL){
                    aux = best_path;
                    best_path = best_path->next;
                    free(aux);
                }

                /* extract the new best path */
                best_path = extract_path_3(current);
            }
            /* 
            * if the backtracking returns a new node with children, start creating the new branch
            * else return the list with the instructions for the best score
            */
            current = next_branch(current, n_rows);
            if (current == NULL){
                return best_path;
            }
        }
        if(max_score < current->potential + current->score){
            current->child = create_child(current, current->children, n_rows, n_cols, visited);
            current = current->child; 
        }
        else{
            while(current->children != NULL){
                    aux = current->children;
                    current->children = current->children->next;
                    free(aux);
                }
            current = next_branch(current, n_rows);
            if (current == NULL){
                return best_path;
            }
        }
        
    }
}
