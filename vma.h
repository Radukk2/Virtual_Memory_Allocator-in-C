#pragma once
#include <inttypes.h>
#include <stddef.h> 
#include <stdlib.h>
#include <string.h>

typedef struct node node;
struct node{
	void *data;
	node *next, *prev;
};

typedef struct list_t list_t;
struct list_t{ 
	node *head, *tail;
	int size;
	int data_size;
};

typedef struct {
	uint64_t start_address; 
	size_t size; 
	void* miniblock_list;
} block_t;

typedef struct {
   uint64_t start_address;
   size_t size;
   uint8_t perm;
   void* rw_buffer;
} miniblock_t;

typedef struct {
   uint64_t arena_size;
   list_t *alloc_list;
} arena_t;

list_t* create_list(unsigned int data_size);
node* get_node(list_t* list, unsigned int poz);
void add_node(list_t* list, unsigned int poz, const void* new_data);
node* remove_node(list_t* list, unsigned int poz);
void free_function(list_t** list);
int intersection(arena_t *arena, block_t *block);
arena_t* alloc_arena(const uint64_t size);
void dealloc_arena(arena_t* arena);
miniblock_t * creator(block_t * parent);
void merger(block_t* first, block_t* last);
void alloc_block(arena_t* arena, const uint64_t address, const uint64_t size);
block_t *block_finder(arena_t *arena, const uint64_t address);
void free_block(arena_t* arena, const uint64_t address);
void read(arena_t* arena, uint64_t address, uint64_t size);
void write(arena_t* arena, const uint64_t address,  const uint64_t size, int8_t *data);
void pmap(const arena_t* arena);
int interpretare_string(char string[100]);
void mprotect(arena_t* arena, uint64_t address, int8_t *permission);