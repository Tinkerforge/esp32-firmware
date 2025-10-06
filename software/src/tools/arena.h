#pragma once

#include <stdint.h>
#include <stddef.h>

#include <type_traits>
#include <initializer_list>
#include <utility>

//#include "gcc_warnings.h"

struct Arena {
    /*
    alloc(5, 1);
    alloc(8, 8);

    |00 01 02 03|04 05 06 07 08|09 0A 0B 0C|0D 0E 0F|10 11 12 13 14 15 16 17|
    |--canary---|----alloc-----|--canary---|--pad.--|---------alloc---------|
    |C0 C0 C5 C1|01 02 03 04 05|C0 C0 C8 C8|B2 B1 B0|01 02 03 04 05 06 07 08|

    Canary bytes are 0xCn, n is the next blocks size (12 bit) and alignment (4 bit) -> MAX_ALLOC = 2^12 - 1
    Padding bytes are 0xBn, n is how many padding bytes follow -> MAX_ALIGNMENT = 16

    */

    // Max allowed alignment is 16 bytes to fit into canary and padding bytes
    static constexpr size_t MAX_ALIGNMENT = 16;

    // Max allowed alloc size is 2^12 because there are 4 canary bytes with 4 bits of payload each.
    // The last canary byte's 4 bits store the requested alignment -> 12 are left over for the requested allocation size.
    // TODO: We don't allow allocating 0 bytes, so we could store (requested allocation) - 1 to allow allocating 2^12 bytes.
    static constexpr size_t MAX_ALLOC_BITS = 12;
    static constexpr size_t MAX_ALLOC = (1 << MAX_ALLOC_BITS) - 1;

    // Each canary byte fits 4 bits of the allocated size.
    // The last 4 bits are the requested alignment (- 1 so that 16 byte alignment is stored as 0xCF. Aligning to 0 bytes does not make any sense)
    static constexpr size_t CANARY_SIZE = (MAX_ALLOC_BITS / 4 + MAX_ALIGNMENT) / 4;

    // Same signature as aligned_alloc (available in C11 and C++17)
    using aligned_alloc_fn_t = void *(size_t alignment, size_t size);

    // We want to allocate a block of memory and its header in one go.
    // Aligning the header with MAX_ALIGNMENT makes sure that the block itself is also aligned with MAX_ALIGNMENT.
    // Fortunately on the ESP this struct is 16 bytes, so this does not create any padding.
    struct alignas(MAX_ALIGNMENT) BlockHeader {
        BlockHeader *next;
        size_t capacity;
        size_t used;
        uint8_t *memory; // Will always point directly behind the header. TODO Store something more useful here?
    };
    static_assert(std::is_trivial_v<BlockHeader>);
    static_assert(sizeof(BlockHeader) == MAX_ALIGNMENT);

    // minimum_alignment can be more than one for example in IRAM.
    // initial_capacity is the size of the first block
    // block_capacity_ is the size of blocks allocated when all previous blocks are full.
    //     Note that larger allocations (block_capacity_ < size <= MAX_ALLOC) will be allocated
    //     as a separate block, so this is where smaller allocs are stored.
    Arena(aligned_alloc_fn_t *aligned_alloc_function,
                    size_t minimum_alignment,
                    size_t initial_capacity,
                    size_t block_capacity_);

    bool setup(std::initializer_list<std::pair<void *, size_t>> preallocated_blocks);

    // Don't copy or move the arena.
    Arena(const Arena&) = delete;
    Arena &operator=(const Arena&) = delete;
    Arena(Arena&&) = delete;
    Arena &operator=(Arena&&) = delete;

    void *aligned_alloc(size_t alignment, size_t size);
    void *alloc(size_t size);

#ifdef DEBUG_FS_ENABLE
    bool check_integrity() const;
#endif
    // Statistics
    size_t allocs = 0;
    size_t padding_overhead = 0;
    size_t allocated_blocks = 0;

    void print_statistics();

//private:
    BlockHeader *alloc_next_block(size_t capacity);

    aligned_alloc_fn_t *aligned_alloc_fn;
    size_t min_alignment;
    size_t first_block_capacity;
    size_t block_capacity;
    BlockHeader *head;
};

