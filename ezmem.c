#include "ezmem.h"
#include <stdlib.h>
#include <stdio.h>

ezmem_DebugLevel_e ezmem_debugMode = 0;

/* Initialize debug system and register exit handler */
void ezmem_init(int dbg) {
    atexit(ezmem_exit);
    ezmem_debugMode = (ezmem_DebugLevel_e)dbg;
}

/* Leak report at program exit */
void ezmem_exit(void) {
    if (ezmem_debugMode >= EZMEM_DEBUG_TRACK && ezmem_allocCount > 0) {
        fprintf(stderr, ANSI_COLOR_YELLOW "Warning: %lu memory allocations not freed:\n" ANSI_COLOR_RESET, (unsigned long)ezmem_allocCount);
        {
            size_t i;
            for (i = 0; i < ezmem_allocCount; ++i) {
                s_ezmemRecord* rec = &ezmem_allocBuffer[i];
                fprintf(stderr, ANSI_COLOR_YELLOW "  Leak: %p (%lu bytes) allocated at %s:%d (%s)\n" ANSI_COLOR_RESET,
                       rec->ptr,
                       (unsigned long)rec->size,
                       rec->file,
                       rec->line,
                       rec->func);
            }
        }
    }
}

/* Search for a tracked pointer */
int ezmem_find_alloc(void* ptr) {
    size_t i;
    for (i = 0; i < ezmem_allocCount; ++i) {
        if (ezmem_allocBuffer[i].ptr == ptr) {
            return (int)i;
        }
    }
    return -1;
}

/* Remove a pointer from tracking buffer */
int ezmem_remove_alloc(void* ptr) {
    int idx = ezmem_find_alloc(ptr);
    if (idx >= 0) {
        size_t i;
        for (i = (size_t)idx; i < ezmem_allocCount - 1; ++i) {
            ezmem_allocBuffer[i] = ezmem_allocBuffer[i + 1];
        }
        --ezmem_allocCount;
        return 1;
    }
    return 0;
}

/* Allocate memory with debug info */
void* ez_malloc_dbg(size_t size, const char* file, int line, const char* func) {
    void* ptr = malloc(size);
    if (ezmem_debugMode) {
        if (!ptr) {
            fprintf(stderr, ANSI_COLOR_RED "Error: malloc failed at %s:%d (%s) for %lu bytes\n" ANSI_COLOR_RESET,
                    file, line, func, (unsigned long)size);
        } else {
            printf("malloc %p of %lu bytes at %s:%d (%s)\n",
                   ptr, (unsigned long)size, file, line, func);
            if (ezmem_debugMode > EZMEM_DEBUG_LOG && ezmem_allocCount < EZMEM_MAX_ALLOCATIONS) {
                ezmem_allocBuffer[ezmem_allocCount].ptr = ptr;
                ezmem_allocBuffer[ezmem_allocCount].size = size;
                ezmem_allocBuffer[ezmem_allocCount].file = file;
                ezmem_allocBuffer[ezmem_allocCount].line = line;
                ezmem_allocBuffer[ezmem_allocCount].func = func;
                ++ezmem_allocCount;
            }
        }
    }
    return ptr;
}

/* Allocate zeroed memory with debug info */
void* ez_calloc_dbg(size_t nmemb, size_t size, const char* file, int line, const char* func) {
    void* ptr = calloc(nmemb, size);
    if (ezmem_debugMode) {
        if (!ptr) {
            fprintf(stderr, ANSI_COLOR_RED "Error: calloc failed at %s:%d (%s) for %lu members of %lu bytes\n" ANSI_COLOR_RESET,
                    file, line, func, (unsigned long)nmemb, (unsigned long)size);
        } else {
            printf("calloc %p of %lu members of %lu bytes at %s:%d (%s)\n",
                   ptr, (unsigned long)nmemb, (unsigned long)size, file, line, func);
            if (ezmem_debugMode > EZMEM_DEBUG_LOG && ezmem_allocCount < EZMEM_MAX_ALLOCATIONS) {
                ezmem_allocBuffer[ezmem_allocCount].ptr = ptr;
                ezmem_allocBuffer[ezmem_allocCount].size = nmemb * size;
                ezmem_allocBuffer[ezmem_allocCount].file = file;
                ezmem_allocBuffer[ezmem_allocCount].line = line;
                ezmem_allocBuffer[ezmem_allocCount].func = func;
                ++ezmem_allocCount;
            }
        }
    }
    return ptr;
}

/* Reallocate memory with debug info */
void* ez_realloc_dbg(void* ptr, size_t size, const char* file, int line, const char* func) {
    void* new_ptr = realloc(ptr, size);
    if (ezmem_debugMode) {
        if (!new_ptr) {
            fprintf(stderr, ANSI_COLOR_RED "Error: realloc failed at %s:%d (%s) for %lu bytes\n" ANSI_COLOR_RESET,
                    file, line, func, (unsigned long)size);
        } else {
            printf("realloc %p (from %p to %lu bytes) at %s:%d (%s)\n",
                   new_ptr, ptr, (unsigned long)size, file, line, func);
            if (ezmem_debugMode > EZMEM_DEBUG_LOG) {
                ezmem_remove_alloc(ptr);
                if (ezmem_allocCount < EZMEM_MAX_ALLOCATIONS) {
                    ezmem_allocBuffer[ezmem_allocCount].ptr = new_ptr;
                    ezmem_allocBuffer[ezmem_allocCount].size = size;
                    ezmem_allocBuffer[ezmem_allocCount].file = file;
                    ezmem_allocBuffer[ezmem_allocCount].line = line;
                    ezmem_allocBuffer[ezmem_allocCount].func = func;
                    ++ezmem_allocCount;
                }
            }
        }
    }
    return new_ptr;
}

/* Free memory with debug info */
void ez_free_dbg(void* ptr, const char* file, int line, const char* func) {
    int removed;
    switch (ezmem_debugMode) {
        case EZMEM_DEBUG_OFF:
            free(ptr);
            break;

        case EZMEM_DEBUG_LOG:
            printf("freed %p at %s:%d (%s)\n", ptr, file, line, func);
            free(ptr);
            break;

        case EZMEM_DEBUG_TRACK:
            removed = ezmem_remove_alloc(ptr);
            if (removed) {
                printf("freed %p at %s:%d (%s)\n", ptr, file, line, func);
            } else {
                fprintf(stderr, ANSI_COLOR_YELLOW "Warning: freed untracked pointer %p at %s:%d (%s)\n" ANSI_COLOR_RESET,
                       ptr, file, line, func);
            }
            free(ptr);
            break;

        case EZMEM_DEBUG_STRICT:
            removed = ezmem_remove_alloc(ptr);
            if (removed) {
                printf("freed %p at %s:%d (%s)\n", ptr, file, line, func);
                free(ptr);
            } else {
                fprintf(stderr, ANSI_COLOR_RED "Error: attempted to free untracked pointer %p at %s:%d (%s)\n" ANSI_COLOR_RESET,
                        ptr, file, line, func);
                /* Do not free untracked memory in STRICT mode */
            }
            break;

        default:
            free(ptr);
            break;
    }
}