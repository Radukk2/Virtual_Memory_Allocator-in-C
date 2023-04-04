#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "vma.h"


int main(void)
{
    char comanda[30];
    while(1){
        arena_t *new_arena;
        int invalid_command = 0;
        fgets(comanda, sizeof(comanda), stdin);
        if (strncmp(comanda, "ALLOC_ARENA", 11) == 0) { 
            int dim = atoi(strtok(comanda, "ALLOC_ARENA \n"));
            new_arena = (arena_t*)alloc_arena(dim);
            invalid_command = 1;
        }
        if (strncmp(comanda, "DEALLOC_ARENA", 13) == 0) {
            dealloc_arena(new_arena);
            break;
            invalid_command = 1;
        }
        if (strncmp(comanda, "ALLOC_BLOCK", 11) == 0) {
            int add = atoi(strtok(comanda, "ALLOC_BlOCK \n"));
            int dim = atoi(strtok(NULL, "\n"));
            alloc_block(new_arena, add, dim);
            invalid_command = 1;
        }
        if (strncmp(comanda, "FREE_BLOCK", 10) == 0) {
            int add = atoi(strtok(comanda, "FREE_BLOCK \n"));
            free_block(new_arena, add);
            invalid_command = 1;
        }
        if (strncmp(comanda, "READ", 4) == 0) {
            int add = atoi(strtok(comanda, "READ \n"));
            int dim = atoi(strtok(NULL, "\n"));
            read(new_arena, add, dim);
            invalid_command = 1;
        }
        if (strncmp(comanda, "WRITE", 5) == 0) {
            int add = atoi(strtok(comanda, "WRITE \n"));
            int dim = atoi(strtok(NULL, "\n"));
            int8_t data = atoi(strtok(NULL, "\n"));
            write(new_arena, add ,dim, &data);
            invalid_command = 1;
        }
        if (strncmp(comanda, "PMAP", 4) == 0) {
            pmap(new_arena);
            invalid_command = 1;
        }
        if (strncmp(comanda, "MPROTECT", 8) == 0) {
            int add = atoi(strtok(comanda, "MPROTECT \n"));
            int8_t dim = atoi(strtok(NULL, "\n"));
            mprotect(new_arena, add, &dim);
            invalid_command = 1;
        }
        if (invalid_command == 0)
            printf("Invalid command. Please try again.\n");
    }
    return 0;
}