
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>

#include "Allocator.h"



int32_t
main(
	int32_t argc,
	char** argv
)
{
	uint32_t	leak_alloc_1 = 256;
	uint32_t	leak_alloc_2 = 128;
	uint32_t	free_alloc = 24;
	char*		leaked_buffer_1 = (char*)MALLOC(leak_alloc_1);
	char*		leaked_buffer_2 = (char*)MALLOC(leak_alloc_2);
	char*		freed_buffer = (char*)MALLOC(free_alloc);

	printf("\n##############################\n");
	printf("### C++ Memory Manager PoC ###\n");
	printf("##############################\n\n");
	printf("allocated %i bytes of memory\n", leak_alloc_1);
	printf("allocated %i bytes of memory\n", leak_alloc_2);
	printf("allocated %i bytes of memory\n", free_alloc);

	strncpy(leaked_buffer_1, "This is allocated memory that will not be freed", leak_alloc_1 - 1);
	strncpy(freed_buffer, "This buffer is freed", free_alloc-1);
	strncpy(leaked_buffer_2, argv[0], leak_alloc_2 - 1);

	printf("freeing one buffer...\n");
	FREE(freed_buffer);
	printf("closing the application without freeing the remaining allocated memory...\n\n");

	return EXIT_SUCCESS;
}
