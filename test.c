/* #define EZMEM_DISABLE */
#include "ezmem.h"

int main() {
	ezmem_init(3);

	int* ptr = ez_malloc(5 * sizeof(int));
	char* ptr2 = ez_calloc(5, sizeof(char));
	ptr = ez_realloc(ptr, 10 * sizeof(int));
	ez_free(ptr);
	ez_free(ptr);
	return 0;
}