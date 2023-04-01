#include <stdio.h>
#include <stddef.h> 
#include <stdlib.h>
#include <string.h>
#include "vma.h"

list_t* create_list(unsigned int data_size)
{
	list_t* list;
	list = NULL;
	list = (list_t*)malloc(sizeof(list_t));
	if (list == NULL)
		exit(0);
	list->head = NULL;
	list->size = 0;
	list->data_size = data_size;
}

node* get_node(list_t* list, unsigned int poz)
{
	node* curent = list->head;
	while(poz) {
		curent = curent->next;
		poz--;
	}
	return curent;
}

void add_node(list_t* list, unsigned int poz, const void* new_data)
{
	node* new_node;
	new_node = malloc(sizeof(node));
	new_node = malloc(list->data_size);
	new_node->next = NULL;
	new_node->prev = NULL;
	memcpy(new_node->data, (void *)new_data, list->data_size);
	if (list->size == 0) {
		list->head = new_node;
		list->tail = new_node;
        list->size = 1;
        list->head->next = NULL;
        list->head->prev = NULL;
        return;
	}
	if (poz == 0) {
		new_node->next = list->head;
		list->head->prev = new_node;
		list->head = new_node;
		list->size++;
		return;
	}
	if (poz >= list->size - 1) {
		node* curent = get_node(list, list->size - 1);
		curent->next = new_node;
		list->tail = new_node;
		new_node->prev = curent;
		list->size++;
		return;
	}
	node* curent = get_node(list, poz);
    new_node->prev = curent;
    new_node->next = curent->next;
    curent->next->prev = new_node;
    curent->next = new_node;
	list->size++;
}

node* remove_node(list_t* list, unsigned int poz)
{
	if (list->size == 0)
		return 0;
	if (poz == 0) {
		node* curent = list->head;
		list->head = curent->next;
		curent->next = NULL;
		list->size--;
	}
	if (poz >= list->size - 1) {
		node* curent = list->tail;
		list->tail = curent->prev;
		curent->prev = NULL;
		list->size--;
		return curent;
	}
		if (poz == 1) {
		node* curent = list->head->next;
		list->head->next = list->head->next->next;
		curent->next->prev = list->head;
		list->size--;
		return curent;
	}
	node* curent= get_node(list, poz);
	curent->prev->next = curent->next;
	curent->next->prev = curent->prev;
	list->size--;
	return curent;

}

void free_function(list_t** list)
{
	node* curent = (*list)->head;
	while(curent->next) {
		node* aux = curent->next;
		free(curent->data);
		free(curent);
		curent = aux;
	}
	free(list);
	list = NULL;
}
arena_t *alloc_arena(const uint64_t size)
{
	arena_t* new_arena = malloc(sizeof(arena_t));
	new_arena->alloc_list = create_list(sizeof(block_t));
	new_arena->arena_size = size;
    return new_arena;
}

void dealloc_arena(arena_t *arena)
{
	if (((block_t *)arena->alloc_list)->miniblock_list)
		free_function(((block_t *)arena->alloc_list)->miniblock_list);
	if (arena->alloc_list)
		free_function(&arena->alloc_list);
	free(arena);
}
int intersection(arena_t *arena, block_t *block)
{
	node *curent = arena->alloc_list->head;
	int dimensiune = (int)block->start_address + (int)block->size;
	int limita_stanga = 0;
	int limita_dreapta = ((block_t *)curent->data)->start_address;
	int contor = 0;
	while(curent) {
		if (dimensiune > limita_stanga && dimensiune < limita_dreapta)
			return contor;
		limita_stanga = ((block_t *)curent->data)->start_address + ((block_t *)curent->data)->size;
		if (curent->next == NULL) {
			limita_dreapta = arena->arena_size;
			if (dimensiune > limita_stanga && dimensiune < limita_dreapta)
				return contor;
		} else
			limita_dreapta = ((block_t *)curent->next->data)->start_address;
		curent = curent->next;
		contor++;
	}
	return -1;
}

miniblock_t * creator(block_t * parent) 
{
	miniblock_t * mini = malloc(sizeof(miniblock_t));
	mini->size = parent->size;
	mini->start_address = parent->start_address;
	mini->perm = 6;
	mini->rw_buffer = malloc(parent->size);
}

void merger(block_t* first, block_t* last)
{
	block_t *merged = malloc(sizeof(block_t));
	merged->start_address = first->start_address;
	merged->size = first->size + last->size;
	((list_t *)(first->miniblock_list))->tail->next = ((list_t *)(last->miniblock_list))->head;
	((list_t *)(last->miniblock_list))->head->prev = ((list_t *)(last->miniblock_list))->tail;
	((list_t *)(merged->miniblock_list))->head = ((list_t *)(first->miniblock_list))->head;
	((list_t *)(merged->miniblock_list))->tail = ((list_t *)(last->miniblock_list))->tail;
	free_function(first->miniblock_list);
	free_function(last->miniblock_list);
	free(first);
	free(last);
}

void alloc_block(arena_t *arena, const uint64_t address, const uint64_t size)
{
	if (address > arena->arena_size) {
		printf("The allocated address is outside the size of arena\n");
		return;
	}
	if (address + size > arena->arena_size) {
		printf("The end address is past the size of the arena\n");
		return;
	}
	block_t *block = malloc(sizeof(block_t));
	block->size = size;
	block->start_address = address;
	block->miniblock_list = create_list(sizeof(miniblock_t));
	miniblock_t *mini = creator(block);
		add_node(block->miniblock_list, 0, mini);
	if (arena->alloc_list->size == 0) {
		add_node(arena->alloc_list, 0, block);
		return;
	}
	if (intersection(arena, block) == -1) {
		printf("This zone was already allocated.\n");
		free_function(block->miniblock_list);
		free(block);
		return;
	}
	int poz = intersection(arena, block);
	node* curent = get_node(arena->alloc_list, poz);
	int right = ((block_t *)curent->next->data)->start_address;
	int left = ((block_t *)curent->prev->data)->start_address +
							((block_t *)curent->prev->data)->size;
	add_node(arena->alloc_list, poz, block);
	if (poz == 0) {
		if (right - 1 == ((block_t *)curent->data)->size + ((block_t *)curent->data)->start_address) {
			merger(block, (block_t *)curent);
			return;
		}
		return;
	}
	if (curent->next == NULL) { 
		if (left + 1 == ((block_t *)curent->data)->start_address) {
			merger(block, (block_t *)curent);
			return;
		}
		return;
	}
	if (right - 1 == ((block_t *)curent->data)->size + ((block_t *)curent->data)->start_address)
		merger(block, (block_t *)curent);
	if (left + 1 == ((block_t *)curent->data)->start_address)
		merger(block, (block_t *)curent);
}

block_t *block_finder(arena_t *arena, uint64_t address)
{
	node *curent = arena->alloc_list->head;
	while (curent){
		long long start = ((block_t *)curent)->start_address;
		long long end = start +((block_t *)curent)->size;
		if (start >= address && address <= end)
			break;
		curent = curent->next;
	}
	return NULL;
}

void free_block(arena_t *arena, const uint64_t address)
{

}

void read(arena_t *arena, uint64_t address, uint64_t size)
{
	if (!block_finder(arena, address))
	{
		printf("Invalid address for read.\n");
		return;
	}
	block_t * block = block_finder(arena, address);
	if (block->size < size) {
		printf("Warning: size was bigger than the block size. Reading %d characters.\n", block->size);
		return;
	}
}

void write(arena_t *arena, const uint64_t address, const uint64_t size, int8_t *data)
{
	if (!block_finder(arena, address))
	{
		printf("Invalid address for write.\n");
		return;
	}
	block_t * block = block_finder(arena, address);
	if (block->size < size) {
		printf("Warning: size was bigger than the block size. Writing %d characters.\n", block->size);
		return;
	}
}

void pmap(const arena_t *arena)
{

}

void mprotect(arena_t *arena, uint64_t address, int8_t *permission)
{

}