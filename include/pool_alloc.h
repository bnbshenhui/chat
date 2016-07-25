#ifndef POOL_ALLOC_H_LNQVOFYP
#define POOL_ALLOC_H_LNQVOFYP

#include <sys/types.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus__
extern "C" {
#endif

#define PAGE_SIZE   4096
#define ALIGNMENT  sizeof(unsigned long)
#define POOL_ALIGNMENT 16

#define ALIGN(d, a)   (((d) + (a - 1)) & ~(a - 1))
#define ALIGN_PTR(p, a) \
  (u_char *)(((uintptr_t)(p) + ((uintptr_t)a - 1)) & ~((uintptr_t)a - 1))

#define MAC_ALLOC_FROM_POOL   (PAGE_SIZE - 1)
#define DEFAULT_POOL_SIZE     (16 * 1024)
#define MIN_POOL_SIZE \
  ALIGN((sizeof(pool_t) + 2 * sizeof(pool_large_t)), POOL_ALIGNMENT)


typedef void (*pool_cleanup_pt)(void* data);
typedef struct pool_cleanup_s pool_cleanup_t;

struct pool_cleanup_s {
  pool_cleanup_pt   handler;
  void              *data;
  pool_cleanup_t    *next;
};

typedef struct pool_large_s pool_large_t;
typedef struct pool_s       pool_t;

struct pool_large_s{
  pool_large_t    *next;
  void            *alloc;
};

typedef struct{
  u_char        *last;
  u_char        *end;
  pool_t        *next;
  int           failed;
}pool_data_t;

struct pool_s{
  pool_data_t     d;
  size_t          max_size;
  pool_t          *current;
  pool_large_t    *large;
  pool_cleanup_t  *clean;
};

void* m_alloc(size_t size);
void* m_calloc(size_t);

pool_t* create_pool(size_t size);
void destroy_pool(pool_t *pool);
void reset_pool(pool_t *pool);

void* pool_alloc(pool_t *pool, size_t size);
void* pool_nalloc(pool_t *pool, size_t size);
void* pool_calloc(pool_t *pool, size_t size);
void* pool_memalign(pool_t *pool, size_t size, size_t alignment);
int pool_free(pool_t* pool, void *p);

pool_cleanup_t* pool_cleanup_add(pool_t *pool, size_t size);
#ifdef __cplusplus__
}
#endif

#endif /* end of include guard: POLL_ALLOC_H_LNQVOFYP */
