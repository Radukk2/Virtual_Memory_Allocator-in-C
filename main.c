#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "vma.h"


int main(void)
{
    char comanda[30], string[30], c;
    long long dim, add;
    // freopen ("ceva.txt", "wt", stdout);
    while(1){
        arena_t *new_arena;
        int invalid_command = 0;
        scanf("%s", comanda);
        if (strcmp(comanda, "ALLOC_ARENA") == 0) { 
            scanf("%lld", &dim);
            new_arena = (arena_t*)alloc_arena(dim);
            invalid_command = 1;
        }
        if (strcmp(comanda, "DEALLOC_ARENA") == 0) {
            dealloc_arena(new_arena);
            break;
            invalid_command = 1;
        }
        if (strcmp(comanda, "ALLOC_BLOCK") == 0) {
            scanf("%lld", &add);
            scanf("%lld", &dim);
            alloc_block(new_arena, add, dim);
            invalid_command = 1;
        }
        if (strcmp(comanda, "FREE_BLOCK") == 0) {
            scanf("%lld", &add);
            free_block(new_arena, add);
            invalid_command = 1;
        }
        if (strcmp(comanda, "READ") == 0) {
           scanf("%lld", &add);
           scanf("%lld", &dim);
            read(new_arena, add, dim);
            invalid_command = 1;
        }
        if (strcmp(comanda, "WRITE") == 0) {
            scanf("%lld", &add);
            scanf("%lld%c", &dim, &c);
            uint8_t *data = malloc( dim * sizeof(char));
            int i = 0;
            while(i < dim) {
                data[i] = getc(stdin);
                i++;
            }
            write(new_arena, add ,dim, data);
            free(data);
            invalid_command = 1;
        }
        if (strcmp(comanda, "PMAP") == 0) {
            pmap(new_arena);
            invalid_command = 1;
        }
        if (strcmp(comanda, "MPROTECT") == 0) {
            scanf("%lld ", &add);
            char string[100];
            fgets(string, 100, stdin);
            int8_t num = interpretare_string(string);
            mprotect(new_arena, add, &num);
            invalid_command = 1;
        }
        if (invalid_command == 0)
            printf("Invalid command. Please try again.\n");
    }
    return 0;
}