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
	node* new_node = malloc(sizeof(node));
	new_node->data = malloc(list->data_size);
	memcpy(new_node->data, (void *)new_data, list->data_size);
	new_node->next = NULL;
	new_node->prev = NULL;
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
	if (poz > list->size - 1) {
		node* curent = get_node(list, list->size - 1);
		curent->next = new_node;
		list->tail = new_node;
		new_node->prev = curent;
		list->size++;
		return;
	}
	node* curent = get_node(list, poz - 1);
    new_node->prev = curent;
    new_node->next = curent->next;
    curent->next->prev = new_node;
    curent->next = new_node;
	list->size++;
	printf("%d", list->size);
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
	while(curent) {
		node* aux = curent->next;
		free(curent->data);
		free(curent);
		curent = aux;
	}
	free(*list);
	list = NULL;
}

void free_rw_buffer (block_t *block)
{
	node* curent = ((list_t *)block->miniblock_list)->head;
	while (curent){
		free(((miniblock_t *)curent->data)->rw_buffer);
		curent = curent->next;
	}
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
	node *curent = ((list_t *)arena->alloc_list)->head;
	while(curent){
		free_rw_buffer((block_t *)curent->data);
		free_function((list_t**)&((block_t*)curent->data)->miniblock_list);
		curent = curent->next;
	}
	if (arena->alloc_list)
		free_function(&arena->alloc_list);
	free(arena);
}

int intersection(arena_t *arena, block_t *block)
{
	node *curent = arena->alloc_list->head;
	int dimensiune = block->start_address + block->size;
	int limita_stanga = 0;
	int limita_dreapta = ((block_t *)curent->data)->start_address;
	int contor = 0;
	while(curent) {
		if (dimensiune >= limita_stanga && dimensiune <= limita_dreapta) {
			if (block->start_address < limita_stanga)
				return -1;
			return contor;
		}
		limita_stanga = ((block_t *)curent->data)->start_address + ((block_t *)curent->data)->size;
		if (curent->next == NULL) {
			limita_dreapta = arena->arena_size;
			if (dimensiune >= limita_stanga && dimensiune <= limita_dreapta) {
				if (block->start_address < limita_stanga)
					return -1;
				return contor + 1;
			}
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
	return mini;
}

void merger(block_t* first, block_t* last)
{
	block_t *merged = malloc(sizeof(block_t));
	merged->miniblock_list = create_list(sizeof(miniblock_t));
	merged->start_address = first->start_address;
	merged->size = first->size + last->size;
	((list_t *)(first->miniblock_list))->tail->next = ((list_t *)(last->miniblock_list))->head;
	((list_t *)(last->miniblock_list))->head->prev = ((list_t *)(last->miniblock_list))->tail;
	((list_t *)(merged->miniblock_list))->head = ((list_t *)(first->miniblock_list))->head;
	((list_t *)(merged->miniblock_list))->tail = ((list_t *)(last->miniblock_list))->tail;
	free_rw_buffer(first);
	free_rw_buffer(last);
	free_function((list_t **)&first->miniblock_list);
	free_function((list_t **)&last->miniblock_list);
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
	add_node((list_t *)block->miniblock_list, 0, mini);
	free(mini);
	if (arena->alloc_list->size == 0) {
		add_node(arena->alloc_list, 0, block);
		free(block);
		return;
	}
	if (intersection(arena, block) == -1) {
		printf("This zone was already allocated.\n");
		free_rw_buffer(block);
		free_function((list_t**)&block->miniblock_list);
		free(block);
		return;
	}
	int poz = intersection(arena, block);
	add_node(arena->alloc_list, poz, block);
	free(block);
	node* curent = get_node(arena->alloc_list, poz);
	long long right, left;
	if (curent->next)
		right = ((block_t *)curent->next->data)->start_address;
	else
		right = arena->arena_size;
	if (curent->prev)
		left = ((block_t *)curent->prev->data)->start_address +((block_t *)curent->prev->data)->size;
	else
		left = 0;
	if (right == ((block_t *)curent->data)->size + ((block_t *)curent->data)->start_address)
		merger((block_t *)curent->data, (block_t *)curent->next->data);
	if (left  == ((block_t *)curent->data)->start_address)
		merger(((block_t *)curent->prev->data), (block_t *)curent->data);
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
	if (block_finder(arena, address) == NULL) {
		printf("Invalid address for free.\n");
		return;
	}
	block_t *block = block_finder(arena, address);
	node *curr = ((list_t *)block->miniblock_list)->head;
	int counter = -1;
	int pos = 0;
	while (curr){
		if (address == ((miniblock_t *)curr->data)->start_address) {
			counter = ((miniblock_t *)curr->data)->size;
			break;
		}
		curr = curr->next;
		pos++;
	}
	int block_poz = intersection(arena, block);
	if (counter == -1) {
		printf("Invalid address for free.\n");
		return;
	}
	if (!curr->next && !curr->prev) {
		node *nod_mini = remove_node((list_t *)block->miniblock_list, pos);
		free(nod_mini->data);
		free(nod_mini);
		node *nod = nod = remove_node(arena->alloc_list, block_poz);
	}
	if (!curr->next) {
		block->size -= counter;
		node * nod = remove_node((list_t *)block->miniblock_list, pos);
		free(nod->data);
		free(nod);
		return;
	}
	if (!curr->prev) {
		block->start_address += counter;
		node * nod = remove_node((list_t *)block->miniblock_list, pos);
		free(nod->data);
		free(nod);
		return;
	}
	block_t *last_block = malloc(sizeof(block_t));
	last_block->start_address = block->start_address + counter;
	last_block->size = block->size;
	last_block->miniblock_list = create_list(sizeof(miniblock_t));
	node *temp = curr->next;
	int i = 0;
	while (temp){
		add_node((list_t *)last_block->miniblock_list, i, temp->data);
		free(temp->data);
		i++;
		temp = temp->next;
	}
	block_t *first_block = malloc(sizeof(block_t));
	first_block->start_address = block->start_address;
	first_block->size = block->start_address + counter;
	first_block->miniblock_list = create_list(sizeof(miniblock_t));
	temp = curr->next;
	while (temp) {
		add_node((list_t *)first_block->miniblock_list, 0, temp->data);
		free(temp->data);
		temp = temp->prev;
	}
	free_function(block->miniblock_list);
	free(block);
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
		printf("Warning: size was bigger than the block size.");
		printf("Reading %ld characters.\n", block->size);
		size = block->size;
	}
}

void write(arena_t *arena, const uint64_t address, const uint64_t size, int8_t *data)
{
	if (!block_finder(arena, address))
	{
		printf("Invalid address for write.\n");
		return;
	}
	block_t *block = block_finder(arena, address);
	if (block->size < size) {
		printf("Warning: size was bigger than the block size.");
		printf("Writing %ld characters.\n", block->size);
	}
}

char* permissions(unsigned int number)
{
	if (number == 0)
		return "---";
	if (number == 1)
		return "--X";
	if (number == 2)
		return "-W-";
	if (number == 3)
		return "-WX";
	if (number == 4)
		return "R--";
	if (number == 5)
		return "R-X";
	if (number == 6)
		return "RW-";
	if (number == 7)
		return "RWX";
}

void pmap(const arena_t *arena)
{
	node *curr = arena->alloc_list->head;
	long long mem = 0;
	int mini_nr = 0;
	while (curr) {
		mem += ((block_t *)curr->data)->size;
		mini_nr += ((list_t *)((block_t *)curr->data)->miniblock_list)->size;
		curr = curr->next;
	}
	int i = 1;
	curr = arena->alloc_list->head;
	printf("Total memory: %lX bytes\n", arena->arena_size);
	printf("Free memory : %llX bytes\n", arena->arena_size - mem);
	printf("Number of allocated blocks: %d\n", arena->alloc_list->size);
	printf("Number of allocated miniblocks: %d\n", mini_nr);
	while(curr) {
		if (i == 1)
			printf("\n");
		printf("Block %d begin\n", i);
		long long start = ((block_t *)curr->data)->start_address;
		long long fin = start + ((block_t *)curr->data)->size;
		printf("Zone: %llx - %llx\n", start, fin);
		node *mini = ((list_t *)((block_t *)curr->data)->miniblock_list)->head;
		int j = 1;
		while(mini) {
			long long addres1 = ((miniblock_t *)mini->data)->start_address;
			long long addres2 = addres1 + ((miniblock_t *)mini->data)->size;
			printf("Miniblock %d:\t\t%llx\t\t-\t\t%llx", j, addres1, addres2);
			char *perms = permissions(((miniblock_t *)mini->data)->perm);
			printf("\t\t| %s\n", perms);
			mini = mini->next;
			j++;
		}
		printf("Block %d end\n\n", i);
		curr = curr->next;
		i++;
	}

}

void mprotect(arena_t *arena, uint64_t address, int8_t *permission)
{

}