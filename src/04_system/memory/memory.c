#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BLOCK_SIZE (1024 * 1024) // 1 MiB
#define BLOCK_COUNT 50           // number of memory blocks to allocate

int main() {
    void* blocks[BLOCK_COUNT];

    for (int i = 0; i < BLOCK_COUNT; ++i) {
        blocks[i] = malloc(BLOCK_SIZE);
        if (blocks[i] == NULL) {
            printf("Allocation failed at %d MiB\n", i);
            break;
        }

        // Fill the memory with zeros to ensure physical allocation
        memset(blocks[i], 0, BLOCK_SIZE);
        printf("Block %d allocated and initialized\n", i + 1);

        // Slow down allocation to observe behavior gradually
        usleep(100000); // 0.1 second
    }

    printf("Done allocating. Press Enter to free memory...\n");
    getchar();

    for (int i = 0; i < BLOCK_COUNT; ++i) {
        free(blocks[i]);
    }

    return 0;
}
