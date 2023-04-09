#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "vma.h"

list_t *create_list(unsigned int data_size)
{
	list_t *list;
	list = NULL;
	list = (list_t *)malloc(sizeof(list_t));
	if (!list)
		exit(0);
	list->head = NULL;
	list->size = 0;
	list->data_size = data_size;
}

node *get_node(list_t *list, unsigned int poz)
{
	node *curent = list->head;
	while (poz) {
		curent = curent->next;
		poz--;
	}
	return curent;
}

void add_node(list_t *list, unsigned int poz, const void *new_data)
{
	node *new_node = malloc(sizeof(node));
	if (!new_node)
		exit(0);
	new_node->data = malloc(list->data_size);
	if (!new_node->data)
		exit(0);
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
		node *curent = get_node(list, list->size - 1);
		curent->next = new_node;
		list->tail = new_node;
		new_node->prev = curent;
		list->size++;
		return;
	}
	node *curent = get_node(list, poz - 1);
	new_node->prev = curent;
	new_node->next = curent->next;
	curent->next->prev = new_node;
	curent->next = new_node;
	list->size++;
}

node *remove_node(list_t *list, unsigned int poz)
{
	if (list->size == 0)
		return 0;
	if (poz == 0 && list->size == 1) {
		node *curent = list->head;
		list->head = NULL;
		curent->prev = NULL;
		list->size--;
		return curent;
	}
	if (poz == 0) {
		node *curent = list->head;
		list->head = curent->next;
		curent->next->prev = NULL;
		list->size--;
		return curent;
	}
	if (poz >= list->size - 1) {
		node *curent = list->tail;
		list->tail = curent->prev;
		if (list->tail)
			list->tail->next = NULL;
		list->size--;
		return curent;
	}
	if (poz == 1) {
		node *curent = list->head->next;
		list->head->next = list->head->next->next;
		curent->next->prev = list->head;
		list->size--;
		return curent;
	}
	node *curent = get_node(list, poz);
	curent->prev->next = curent->next;
	curent->next->prev = curent->prev;
	list->size--;
	return curent;
}

void free_function(list_t **list)
{
	node *curent = (*list)->head;
	while (curent) {
		node *aux = curent->next;
		free(curent->data);
		free(curent);
		curent = aux;
	}
	free(*list);
	list = NULL;
}

void free_rw_buffer(block_t *block)
{
	node *curent = ((list_t *)block->miniblock_list)->head;
	while (curent) {
		if (((miniblock_t *)curent->data)->rw_buffer)
			free(((miniblock_t *)curent->data)->rw_buffer);
		curent = curent->next;
	}
}

arena_t *alloc_arena(const uint64_t size)
{
	arena_t *new_arena = malloc(sizeof(arena_t));
	if (!new_arena)
		exit(0);
	new_arena->alloc_list = create_list(sizeof(block_t));
	new_arena->arena_size = size;
	return new_arena;
}

void dealloc_arena(arena_t *arena)
{
	node *curent = ((list_t *)arena->alloc_list)->head;
	while (curent) {
		free_rw_buffer((block_t *)curent->data);
		free_function((list_t **)&((block_t *)curent->data)->miniblock_list);
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
	while (curent) {
		if (dimensiune >= limita_stanga && dimensiune <= limita_dreapta) {
			if (block->start_address >= limita_stanga)
				return contor;
		}
		int l = ((block_t *)curent->data)->start_address;
		limita_stanga = l + ((block_t *)curent->data)->size;
		if (!curent->next) {
			limita_dreapta = arena->arena_size;
			if (dimensiune >= limita_stanga && dimensiune <= limita_dreapta) {
				if (block->start_address >= limita_stanga)
					return contor + 1;
			}
		} else {
			limita_dreapta = ((block_t *)curent->next->data)->start_address;
		}
		curent = curent->next;
		contor++;
	}
	return -1;
}

miniblock_t *creator(block_t *parent)
{
	miniblock_t *mini = malloc(sizeof(miniblock_t));
	if (!mini)
		exit(0);
	mini->size = parent->size;
	mini->start_address = parent->start_address;
	mini->perm = 6;
	mini->rw_buffer = malloc(1);
	if (!mini->rw_buffer)
		exit(0);
	return mini;
}

void merger(block_t *first, block_t *last)
{
	first->size = first->size + last->size;
	node *curent = ((list_t *)(last->miniblock_list))->head;
	while (curent) {
		size_t size = ((list_t *)(first->miniblock_list))->size;
		add_node(((list_t *)(first->miniblock_list)), size + 1, curent->data);
		curent = curent->next;
	}
}

void alloc_block(arena_t *arena, const uint64_t address, const uint64_t size)
{
	if (address >= arena->arena_size) {
		printf("The allocated address is outside the size of arena\n");
		return;
	}
	if (address + size > arena->arena_size) {
		printf("The end address is past the size of the arena\n");
		return;
	}
	block_t *block = malloc(sizeof(block_t));
	if (!block)
		exit(0);
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
		free_function((list_t **)&block->miniblock_list);
		free(block);
		return;
	}
	int poz = intersection(arena, block);
	add_node(arena->alloc_list, poz, block);
	free(block);
	node *curent = get_node(arena->alloc_list, poz);
	long long right, left;
	if (curent->next)
		right = ((block_t *)curent->next->data)->start_address;
	else
		right = arena->arena_size;
	if (curent->prev) {
		long long l = ((block_t *)curent->prev->data)->start_address;
		left = l + ((block_t *)curent->prev->data)->size;
	} else {
		left = 0;
	}
	long start = ((block_t *)curent->data)->start_address;
	long sze = ((block_t *)curent->data)->size;
	if (right == sze + start && right != arena->arena_size) {
		merger((block_t *)curent->data, (block_t *)curent->next->data);
		node *nod = remove_node((list_t *)arena->alloc_list, poz + 1);
		free_function((list_t **)&((block_t *)nod->data)->miniblock_list);
		free(nod->data);
		free(nod);
	}
	if (left  == ((block_t *)curent->data)->start_address && left != 0) {
		merger(((block_t *)curent->prev->data), (block_t *)curent->data);
		node *nod = remove_node((list_t *)arena->alloc_list, poz);
		free_function((list_t **)&((block_t *)nod->data)->miniblock_list);
		free(nod->data);
		free(nod);
	}
}

int block_nr(arena_t *arena, const uint64_t address)
{
	node *curent = arena->alloc_list->head;
	int poz = 0;
	while (curent) {
		long long start = ((block_t *)curent->data)->start_address;
		long long end = start + ((block_t *)curent->data)->size;
		if (start <= address && address <= end)
			return poz;
		curent = curent->next;
		poz++;
	}
	return -1;
}

block_t *block_finder(arena_t *arena,  uint64_t address)
{
	node *curent = arena->alloc_list->head;
	while (curent) {
		long long start = ((block_t *)curent->data)->start_address;
		long long end = start + ((block_t *)curent->data)->size;
		if (start <= address && address <= end)
			return ((block_t *)curent->data);
		curent = curent->next;
	}
	return NULL;
}

void free_block_and_mini_list(block_t *block)
{
	free(block->miniblock_list);
	free(block);
}

void free_node_and_data(node *node)
{
	free(node->data);
	free(node);
}

void new_dimensions(block_t *block, node *curr, int counter)
{
	block->size -= counter;
	if (!curr->prev)
		block->start_address += counter;
}

void free_block(arena_t *arena, const uint64_t address)
{
	if (!block_finder(arena, address)) {
		printf("Invalid address for free.\n");
		return;
	}
	block_t *block = block_finder(arena, address);
	node *curr = (((list_t *)block->miniblock_list)->head);
	int counter = -1;
	int pos = 0;
	while (curr) {
		if (address == ((miniblock_t *)curr->data)->start_address) {
			counter = ((miniblock_t *)curr->data)->size;
			break;
		}
		curr = curr->next;
		pos++;
	}
	int block_poz = block_nr(arena, address);
	if (counter == -1) {
		printf("Invalid address for free.\n");
		return;
	}
	if (!curr->next && !curr->prev) {
		node *nod_mini = remove_node((list_t *)block->miniblock_list, pos);
		free_rw_buffer(block);
		free(((miniblock_t *)curr->data)->rw_buffer);
		free_function((list_t **)&block->miniblock_list);
		free_node_and_data(nod_mini);
		node *nod = remove_node(arena->alloc_list, block_poz);
		free_node_and_data(nod);
		return;
	}
	if (!curr->next || !curr->prev) {
		new_dimensions(block, curr, counter);
		node *nod = remove_node((list_t *)block->miniblock_list, pos);
		free(((miniblock_t *)nod->data)->rw_buffer);
		free_node_and_data(nod);
		return;
	}
	block_t *last_block = malloc(sizeof(block_t));
	int nxt_add = ((miniblock_t *)curr->next->data)->start_address;
	last_block->start_address = nxt_add;
	last_block->size = block->start_address - nxt_add + block->size;
	last_block->miniblock_list = create_list(sizeof(miniblock_t));
	node *temp = curr->next;
	int i = 0;
	while (temp) {
		add_node((list_t *)last_block->miniblock_list, i++, temp->data);
		free(temp->data);
		temp = temp->next;
	}
	block_t *first_block = malloc(sizeof(block_t));
	first_block->start_address = block->start_address;
	first_block->size = nxt_add - counter - first_block->start_address;
	first_block->miniblock_list = create_list(sizeof(miniblock_t));
	temp = curr->prev;
	while (temp) {
		add_node((list_t *)first_block->miniblock_list, 0, temp->data);
		free(temp->data);
		temp = temp->prev;
	}
	node *bl1 = remove_node(arena->alloc_list, block_poz);
	add_node(arena->alloc_list, block_poz, first_block);
	free(first_block);
	add_node(arena->alloc_list, block_poz + 1, last_block);
	free(last_block);
	i = 0;
	node *curr2 = ((list_t *)block->miniblock_list)->head;
	while (curr2) {
		node *aux = curr2->next;
		if (i++ == pos) {
			free(((miniblock_t *)curr2->data)->rw_buffer);
			free(curr2->data);
		}
		free(curr2);
		curr2 = aux;
	}
	free_block_and_mini_list(block);
	free(bl1);
}

int read_perms(int nr)
{
	if (nr >= 4)
		return 1;
	return 0;
}

int write_perms(int nr)
{
	if (nr == 2 || nr == 6 || nr == 7)
		return 1;
	return 0;
}

void read(arena_t *arena, uint64_t address, uint64_t size)
{
	if (!block_finder(arena, address)) {
		printf("Invalid address for read.\n");
		return;
	}
	block_t *block = block_finder(arena, address);
	long long final = block->size + block->start_address;
	long long max = final - address;
	long true_size = size;
	if (size > max) {
		printf("Warning: size was bigger than the block size. ");
		printf("Reading %lld characters.\n", max);
		true_size = max;
	}
	node *curr = ((list_t *)block->miniblock_list)->head;
	while (curr) {
		int start = ((miniblock_t *)curr->data)->start_address;
		int end = start + ((miniblock_t *)curr->data)->size;
		if (start <= address && address <= end)
			break;
		curr = curr->next;
	}
	while (curr) {
		int start = ((miniblock_t *)curr->data)->start_address;
		int end = start + ((miniblock_t *)curr->data)->size;
		if (start <= address && address <= end)
			break;
		curr = curr->next;
	}
	node *curr2 = curr;
	while (curr2) {
		if (read_perms(((miniblock_t *)curr2->data)->perm) != 1) {
			printf("Invalid permissions for read.\n");
			return;
		}
		curr2 = curr2->next;
	}
	long l_max = true_size + address;
	long cursor = ((miniblock_t *)(curr->data))->start_address;
	while (true_size) {
		static int i = -1;
		if (cursor <= address) {
			cursor++;
			i++;
			continue;
		}
		if (cursor >= address && cursor <= l_max) {
			printf("%c", ((int8_t *)((miniblock_t *)curr->data)->rw_buffer)[i]);
			i++;
		}
		int left = ((miniblock_t *)curr->data)->start_address;
		int right = left + ((miniblock_t *)curr->data)->size;
		if (cursor >= right) {
			curr = curr->next;
			i = 0;
		}
		true_size--;
		cursor++;
	}
	printf("\n");
}

void write(arena_t *arena, uint64_t address, const uint64_t size, int8_t *data)
{
	if (!block_finder(arena, address)) {
		printf("Invalid address for write.\n");
		return;
	}
	block_t *block = block_finder(arena, address);
	long long final = block->size + block->start_address;
	long long max = final - address;
	long true_size = size;
	if (size > max) {
		printf("Warning: size was bigger than the block size. ");
		printf("Writing %lld characters.\n", max);
		true_size = max;
	}
	node *curr = ((list_t *)block->miniblock_list)->head;
	while (curr) {
		int start = ((miniblock_t *)curr->data)->start_address;
		int end = start + ((miniblock_t *)curr->data)->size;
		if (start <= address && address <= end)
			break;
		curr = curr->next;
	}
	node *curr2 = curr;
	node *curr3 = curr;
	while (curr3) {
		if (write_perms(((miniblock_t *)curr3->data)->perm) != 1) {
			printf("Invalid permissions for write.\n");
			return;
		}
		curr3 = curr3->next;
	}
	long l_max = true_size + address;
	while (curr2) {
		int start = ((miniblock_t *)curr2->data)->start_address;
		int end = start + ((miniblock_t *)curr2->data)->size;
		if (start < l_max) {
			free(((miniblock_t *)curr2->data)->rw_buffer);
			long sze = ((miniblock_t *)curr2->data)->size + 1;
			((miniblock_t *)curr2->data)->rw_buffer = malloc(sze);
			if (!sze)
				exit(0);
		} else {
			break;
		}
		curr2 = curr2->next;
	}
	long cursor = ((miniblock_t *)(curr->data))->start_address;
	int j = 0;
	while (true_size > 0) {
		static int i = -1;
		if (cursor <= address) {
			cursor++;
			i++;
			continue;
		}
		if (cursor >= address && cursor <= l_max) {
			((int8_t *)((miniblock_t *)curr->data)->rw_buffer)[i] = data[j];
			i++;
			j++;
		}
		int left = ((miniblock_t *)curr->data)->start_address;
		int right = left + ((miniblock_t *)curr->data)->size;
		if (cursor >= right) {
			curr = curr->next;
			i = 0;
		}
		true_size--;
		cursor++;
	}
}

char *permissions(unsigned int number)
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
	printf("Total memory: 0x%lX bytes\n", arena->arena_size);
	printf("Free memory: 0x%llX bytes\n", arena->arena_size - mem);
	printf("Number of allocated blocks: %d\n", arena->alloc_list->size);
	printf("Number of allocated miniblocks: %d\n", mini_nr);
	while (curr) {
		printf("\n");
		printf("Block %d begin\n", i);
		long long start = ((block_t *)curr->data)->start_address;
		long long fin = start + ((block_t *)curr->data)->size;
		printf("Zone: 0x%llX - 0x%llX\n", start, fin);
		node *mini = ((list_t *)((block_t *)curr->data)->miniblock_list)->head;
		int j = 1;
		while (mini) {
			long long add1 = ((miniblock_t *)mini->data)->start_address;
			long long add2 = add1 + ((miniblock_t *)mini->data)->size;
			printf("Miniblock %d:\t\t0x%llX\t\t-\t\t0x%llX", j, add1, add2);
			char *perms = permissions(((miniblock_t *)mini->data)->perm);
			printf("\t\t| %s\n", perms);
			mini = mini->next;
			j++;
		}
		printf("Block %d end\n", i);
		curr = curr->next;
		i++;
	}
}

int verif_str(char str[10])
{
	if (strcmp(str, "PROT_NONE") == 0)
		return 0;
	if (strcmp(str, "PROT_READ") == 0)
		return 4;
	if (strcmp(str, "PROT_WRITE") == 0)
		return 2;
	if (strcmp(str, "PROT_EXEC") == 0)
		return 1;
}

int8_t interpretare_string(char string[100])
{
	char *perm1 = strtok(string, " |\n");
	int i = 0;
	i += verif_str(perm1);
	perm1 = strtok(NULL, " |\n");
	while (perm1) {
		i += verif_str(perm1);
		perm1 = strtok(NULL, " |\n");
	}
	return i;
}

void mprotect(arena_t *arena, uint64_t address, int8_t *permission)
{
	if (!block_finder(arena, address)) {
		printf("Invalid address for mprotect.\n");
		return;
	}
	block_t *block = block_finder(arena, address);
	node *curr = (((list_t *)block->miniblock_list)->head);
	while (curr) {
		if (address == ((miniblock_t *)curr->data)->start_address)
			break;
		curr = curr->next;
	}
	if (!curr) {
		printf("Invalid address for mprotect.\n");
		return;
	}
	((miniblock_t *)curr->data)->perm = *permission;
}
