#ifndef EZMEM_H
#define EZMEM_H

#include <stddef.h>
#include <stdlib.h>

#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_RED     "\x1b[1;31m" 
#define ANSI_COLOR_RESET   "\x1b[0m"

/* Debug levels */
typedef enum {
    EZMEM_DEBUG_OFF = 0,   /* No logging or tracking */
    EZMEM_DEBUG_LOG = 1,   /* Log allocations and frees */
    EZMEM_DEBUG_TRACK = 2, /* Track allocations and frees */
    EZMEM_DEBUG_STRICT = 3 /* Strict tracking, error on untracked free */
} ezmem_DebugLevel_e;

extern ezmem_DebugLevel_e ezmem_debugMode;

typedef struct {
    void* ptr;
    size_t size;
    const char* file;
    int line;
    const char* func;
} s_ezmemRecord;

#define EZMEM_MAX_ALLOCATIONS 1024

/* Internal tracking buffer */
static s_ezmemRecord ezmem_allocBuffer[EZMEM_MAX_ALLOCATIONS];
static size_t ezmem_allocCount = 0;

/* Public API */
void ezmem_init(int dbg);
void ezmem_exit(void);
int ezmem_find_alloc(void* ptr);
int ezmem_remove_alloc(void* ptr);

void* ez_malloc_dbg(size_t size, const char* file, int line, const char* func);
void* ez_calloc_dbg(size_t nmemb, size_t size, const char* file, int line, const char* func);
void* ez_realloc_dbg(void* ptr, size_t size, const char* file, int line, const char* func);
void ez_free_dbg(void* ptr, const char* file, int line, const char* func);

/* Macros for automatic debug info */
#if defined(EZMEM_DISABLE)
    #define ez_malloc(size) malloc(size)
    #define ez_calloc(nmemb, size) calloc(nmemb, size)
    #define ez_realloc(ptr, size) realloc(ptr, size)
    #define ez_free(ptr) free(ptr)
#else
    #define ez_malloc(size) ez_malloc_dbg(size, __FILE__, __LINE__, __FUNCTION__)
    #define ez_calloc(nmemb, size) ez_calloc_dbg(nmemb, size, __FILE__, __LINE__, __FUNCTION__)
    #define ez_realloc(ptr, size) ez_realloc_dbg(ptr, size, __FILE__, __LINE__, __FUNCTION__)
    #define ez_free(ptr) ez_free_dbg(ptr, __FILE__, __LINE__, __FUNCTION__)
#endif

#endif /* EZMEM_H */
