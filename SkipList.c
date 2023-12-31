#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "MemTable.h"

struct slice{
	char *data;
	int len;
};

#define cmp_lt(a,b) (strcmp(a,b)<0)
#define cmp_eq(a,b) (strcmp(a,b)==0)

#define NIL list->hdr

struct pool {
    struct pool *next;
    char *ptr;
    unsigned int rem;
};

static struct pool *pool_new (size_t size)
{
    struct pool *pool = malloc(sizeof(struct pool));
    pool->next = NULL;
    pool->ptr = malloc(size);

    return pool;
}

static void pool_destroy (struct pool *pool)
{
    while (pool->next != NULL) {
		struct pool *next = pool->next;
		free(pool->ptr);
		free (pool);
		pool = next;
    }
}

static void *pool_alloc (struct skiplist *list,size_t size)
{
	struct pool *pool = pool_new (size);
	pool->next = list->pool;
	list->pool = pool;
	return pool->ptr;
}

struct skiplist *skiplist_new(size_t size)
{
	int i;
	struct skiplist *list=malloc(sizeof(struct skiplist));
	list->hdr = malloc(sizeof(struct skipnode) + MAXLEVEL*sizeof(struct skipnode *));

	for (i = 0; i <= MAXLEVEL; i++)
		list->hdr->forward[i] = NIL;

	list->level = 0;
	list->size=size;
	list->count=0;
	list->pool=(struct pool *) list->pool_embedded;
	list->pool->next=NULL;
	return list;
}

void skiplist_free(struct skiplist *list)
{
	pool_destroy(list->pool);
	free(list->hdr);
	free(list);
}

int skiplist_notfull(struct skiplist *list)
{
	if(list->count < list->size)
		return 1;

	return 0;
}

int skiplist_insert(struct skiplist *list,struct slice *sk,UINT val,OPT opt) 
{
	int i, new_level;
	struct skipnode *update[MAXLEVEL+1];
	struct skipnode *x;

	char *key=sk->data;

	if(!skiplist_notfull(list))
		return 0;

	x = list->hdr;
	for (i = list->level; i >= 0; i--) {
		while (x->forward[i] != NIL 
				&& cmp_lt(x->forward[i]->key, key))
			x = x->forward[i];
		update[i] = x;
	}

	x = x->forward[0];
	if (x != NIL && cmp_eq(x->key, key)){
		x->val = val;
		x->opt = opt;
		return(1);
	}

	for (new_level = 0; rand() < RAND_MAX/2 && new_level < MAXLEVEL; new_level++);

	if (new_level > list->level) {
		for (i = list->level + 1; i <= new_level; i++)
			update[i] = NIL;

		list->level = new_level;
	}

	if ((x =pool_alloc(list,sizeof(struct skipnode) + new_level*sizeof(struct skipnode *))) == 0)
		__DEBUG("%s","memory *ERROR*");

	memcpy(x->key,key,sk->len);
	x->val=val;
	x->opt=opt;

	for (i = 0; i <= new_level; i++) {
		x->forward[i] = update[i]->forward[i];
		update[i]->forward[i] = x;
	}
	list->count++;

	return(1);
}

void skiplist_delete(struct skiplist *list,char* data) 
{
	int i;
	struct skipnode *update[MAXLEVEL+1], *x;

	x = list->hdr;
	for (i = list->level; i >= 0; i--) {
		while (x->forward[i] != NIL 
				&& cmp_lt(x->forward[i]->key, data))
			x = x->forward[i];
		update[i] = x;
	}
	x = x->forward[0];
	if (x == NIL || !cmp_eq(x->key, data))
		return;

	for (i = 0; i <= list->level; i++) {
		if (update[i]->forward[i] != x)
			break;
		update[i]->forward[i] = x->forward[i];
	}
	free (x);

	while ((list->level > 0)
			&& (list->hdr->forward[list->level] == NIL))
		list->level--;
}

struct skipnode *skiplist_lookup(struct skiplist *list,char* data) 
{
	int i;
	struct skipnode *x = list->hdr;
	for (i = list->level; i >= 0; i--) {
		while (x->forward[i] != NIL 
				&& cmp_lt(x->forward[i]->key, data))
			x = x->forward[i];
	}
	x = x->forward[0];
	if (x != NIL && cmp_eq(x->key, data)) 
		return (x);

	return NULL;
}


void skiplist_dump(struct skiplist *list)
{
	int i;
    struct skipnode *x = list->hdr->forward[0];

	printf("--skiplist dump:level<%d>,size:<%d>,count:<%d>\n",
			list->level,
			(int)list->size,
			(int)list->count);

	for(i=0;i<list->count;i++){
		printf("	[%d]key:<%s>;val<%llu>;opt<%s>\n",
				i,
				x->key,
				x->val,
				x->opt==ADD?"ADD":"DEL");
		x=x->forward[0];
	}
}