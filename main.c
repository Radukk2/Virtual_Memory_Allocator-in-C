#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "vma.h"


int main(void)
{
    char comanda[30], string[30];
    long long dim, add;
    while(1){
        arena_t *new_arena;
        int invalid_command = 0;
        scanf("%s", comanda);
        if (strncmp(comanda, "ALLOC_ARENA", 11) == 0) { 
            scanf("%lld", &dim);
            new_arena = (arena_t*)alloc_arena(dim);
            invalid_command = 1;
        }
        if (strncmp(comanda, "DEALLOC_ARENA", 13) == 0) {
            dealloc_arena(new_arena);
            break;
            invalid_command = 1;
        }
        if (strncmp(comanda, "ALLOC_BLOCK", 11) == 0) {
            scanf("%lld", &add);
            scanf("%lld", &dim);
            alloc_block(new_arena, add, dim);
            invalid_command = 1;
        }
        if (strncmp(comanda, "FREE_BLOCK", 10) == 0) {
            scanf("%lld", &add);
            free_block(new_arena, add);
            invalid_command = 1;
        }
        if (strncmp(comanda, "READ", 4) == 0) {
           scanf("%lld", &add);
           scanf("%lld", &dim);
            read(new_arena, add, dim);
            invalid_command = 1;
        }
        if (strncmp(comanda, "WRITE", 5) == 0) {
            scanf("%lld", &add);
            scanf("%lld", &dim);
            char *data = malloc( dim * sizeof(char));
            int i = 0;
            while(i < dim) {
                data[i] = getc(stdin);
                i++;
            }
            printf("%s", data);
            write(new_arena, add ,dim, string);
            free(data);
            invalid_command = 1;
        }
        if (strncmp(comanda, "PMAP", 4) == 0) {
            pmap(new_arena);
            invalid_command = 1;
        }
        if (strncmp(comanda, "MPROTECT", 8) == 0) {
            scanf("%lld", &add);
            scanf("%s", string);
            mprotect(new_arena, add, string);
            invalid_command = 1;
        }
        if (invalid_command == 0)
            printf("Invalid command. Please try again.\n");
    }
    return 0;
}