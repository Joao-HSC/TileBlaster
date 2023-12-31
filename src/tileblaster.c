#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "mem.h"
#include "files.h"
#include "tiles.h"
#include "vars.h"

int main(int argc, char *argv[]) {

    int n_cols = 0;
    int n_rows = 0;
    int v = 0;
    int points, n_broken;
    int **tileset;
    bool **visited;
    FILE *file = NULL;
    Coordinates_plus* current;
    Coordinates_plus* head_coords;
    Coordinates_plus* aux = NULL;

    file = open_file(argc, argv);
    char *ss_name = create_ss(argv);

    while (fscanf(file, "%d %d %d", &n_rows, &n_cols, &v) == 3) {
  
        head_coords = NULL;
        points = 0;
        n_broken = -1;

        tileset = read_store_tileset(file, n_rows, n_cols);

        /* Variant 1 */
        /*
        * We start by analyzing the tileset like a chess board starting at (1 , 1). Everytime we find
        * adjacent tiles we'll break them no matter what, apply gravity/push and start from (1, 1) again
        * until there's no stains left.
        */
        if (v == -1) {
            visited = alloc_visit(n_rows, n_cols);
            head_coords = var_1(tileset, visited, n_rows, n_cols);
            free_tileset(tileset, n_rows);
            free_visited(visited, n_rows);
        }
                        

        /* Variant 2 */
        /*
        * We'll start by starting the making of an implicit tree, and everytime we reach a leaf,
        * we'll check whether or not the score associated with that node is greater or
        * equal than v. If so, we'll return a list of Coordinates which will give
        * us the specific path for us to achieve that score.
        */
        else if(v >= 0){

            visited = alloc_visit(n_rows, n_cols);
            
            head_coords = dfs_2(tileset, v, n_rows, n_cols, visited);
            
            free_visited(visited, n_rows);
            
        }

        /* Variant 3 */
        /*
        * We'll start by starting the making of a branch, and everytime we reach a leaf,
        * we'll check whether or not the score associated with that node is greater
        * than the previous. If so, we'll store the node with the highest score and
        * extract its path in the end.
        */
        else if(v == -3){
            visited = alloc_visit(n_rows, n_cols);
            
            head_coords = dfs_3(tileset, n_rows, n_cols, visited);
            
            free_visited(visited, n_rows);
        }

        /* calculate points and n_broken based on the head_coords list */
        current = head_coords;
  
        if(v == -1){
            for (n_broken = 0; current != NULL; n_broken++) {
                points += current->score;
                current = current->next;
            }
        }    
        else{
            for (n_broken = -1; current != NULL; n_broken++) {
                if(current->next == NULL) points = current->score;
                current = current->next;
            }
        }     
        if(v == 0 && points == 0){
            n_broken = 0;
            points = 0;
        }
        else if(v > 0 && points == 0){     
            n_broken = 0;
            points = -1;
        }
        else if(v == -3 && points == 0){     
            n_broken = 0;
        }
        
        /* write file and get ready to restart */
        write_file(argv, n_rows, n_cols, v, points, n_broken, head_coords, ss_name);       

        /* free list */
        current = head_coords;
        while (current != NULL) {
            aux = current;  
            current = current->next;          
            free(aux);                       
        }
    }

    fclose(file);
    return 0;
}
