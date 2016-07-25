#include <stdlib.h>
#include "defs.h"
#include "pool_alloc.h"

static void* pool_alloc_small(pool_t *pool, size_t size, int align);
static void* pool_alloc_block(pool_t *pool, size_t size);
static void* pool_alloc_large(pool_t* pool, size_t size);

pool_t* create_pool(size_t size)
{
  pool_t *pool;
  pool = (pool_t *)malloc(size);
  if(pool == NULL){
    return NULL;
  }

  pool->d.last = (u_char *) pool + sizeof(pool_t);
  pool->d.end = (u_char *) pool + size;
  pool->d.next = NULL;
  pool->d.failed = 0;

  pool->current = pool;
  pool->large = NULL;
  pool->clean = NULL;

  return pool;
}

void destroy_pool(pool_t *pool)
{
  pool_t *p, *n;
  pool_large_t *l;
  pool_cleanup_t *c;

  for(c = pool->clean; c; c = c->next){
    if(c->handler){
      c->handler(c->data);
    }
  }

  for(l = pool->large; l; l = l->next){
    if(l->alloc){
      free(l->alloc);
    }
  }
  for(p = pool, n = pool->d.next; /*void*/; p = n, n = n->d.next){
    free(p);
    if(n == NULL)
      break;
  }
}

void reset_pool(pool_t *pool)
{
  pool_t *p;
  pool_large_t *l;

  for(l = pool->large; l; l = l->next){
    if(l->alloc)
      free(l->alloc);
  }
  
  for(p = pool; p; p = p->d.next){
    p->d.last = (u_char *) p + sizeof(pool_t);
    p->d.failed = 0;
  }
  pool->current = pool;
  pool->large = NULL;
}

void* pool_alloc(pool_t *pool, size_t size)
{
  if(size <= pool->max_size){
    return pool_alloc_small(pool, size, 1);
  }
  return pool_alloc_large(pool, size);
}

void* pool_nalloc(pool_t *pool, size_t size)
{
  if(size <= pool->max_size){
    return pool_alloc_small(pool, size, 0);
  }
  return pool_alloc_large(pool, size);
}

static void* pool_alloc_small(pool_t *pool, size_t size, int align)
{
  u_char *m;
  pool_t *p;
  p = pool->current;

  do {
    m = p->d.last;
    if(align){
      m = ALIGN_PTR(m, ALIGNMENT);
    }
    if((size_t)(p->d.end - m) >= size){
      p->d.last = m + size;
      return m;
    }
    p = p->d.next;
  } while(p);

  return pool_alloc_block(pool, size);
}

static void* pool_alloc_block(pool_t *pool, size_t size)
{
  u_char *m;
  size_t psize;
  pool_t *p, *n;

  psize = (size_t)(pool->d.end - (u_char *)pool);
  m = (u_char *) malloc(psize);
  if(m == NULL)
    return NULL;

  n = (pool_t *) m;
  n->d.end = m + psize;
  n->d.next = NULL;
  n->d.failed = 0;

  m += sizeof(pool_data_t);
  m = ALIGN_PTR(m, ALIGNMENT);
  n->d.last = m + size;

  for(p = pool->current; p->d.next; p = p->d.next){
    if(p->d.failed++ > 4){
      pool->current = p->d.next;
    }
  }

  p->d.next = n;
  return m;
}

static void* pool_alloc_large(pool_t* pool, size_t size)
{
  void *p;
  int   n;
  pool_large_t *large;

  p = malloc(size);
  if(p == NULL)
    return NULL;
  
  n = 0;
  for(large = pool->large; large; large = large->next){
    if(large->alloc == NULL){
      large->alloc = p;
      return p;
    }
    if(n++ > 3){
      break;
    }
  }
  
  large = (pool_large_t *)pool_alloc_small(pool, sizeof(pool_large_t), 1);
  if(large == NULL){
    free(p);
    return NULL;
  }
  large->alloc = p;
  large->next = pool->large;
  pool->large = large;
  return p;
}

void* pool_memalign(pool_t *pool, size_t size, size_t alignment)
{
  void *p;
  pool_large_t *large;
  p = malloc(size);
  if(p == NULL)
    return NULL;

  large = (pool_large_t *)pool_alloc_small(pool, sizeof(pool_large_t), 1);
  if(large == NULL)
    return NULL;

  large->alloc = p;
  large->next = pool->large;
  pool->large = large;
  return p;
}

int pool_free(pool_t *pool, void *p)
{
  pool_large_t *l;
  for(l = pool->large; l; l = l->next){
    if( p == l->alloc ) {
      free(l->alloc);
      l->alloc = NULL;

      return 0;
    }
  }
  return -1;
}

void* pool_calloc(pool_t *pool, size_t size)
{
  void *p;
  p = pool_alloc(pool, size);
  if(p)
    memset(p, 0, size);
  return p;
}
