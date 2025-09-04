#include "linear_allocator.h"

#include <stdlib.h>
#include <string.h>

#include <algorithm>
#include <bit>

#include "event_log_prefix.h"
#include "main_dependencies.h"

#include "gcc_warnings.h"

LinearAllocator::LinearAllocator(aligned_alloc_fn_t *aligned_alloc_function,
                                 size_t minimum_alignment,
                                 size_t initial_capacity,
                                 size_t block_capacity_)
    : aligned_alloc_fn(aligned_alloc_function),
      min_alignment(minimum_alignment),
      first_block_capacity(initial_capacity),
      block_capacity(block_capacity_),
      head(nullptr)
{}

static LinearAllocator::BlockHeader *adopt_memory(void *mem, size_t size)
{
    auto *result = static_cast<LinearAllocator::BlockHeader *>(mem);
    result->next = nullptr;
    result->capacity = size - sizeof(LinearAllocator::BlockHeader);
    result->used = 0;
    // + 1 block header points to the memory behind the header
    result->memory = reinterpret_cast<uint8_t *>(result + 1);

    return result;
}

bool LinearAllocator::setup(std::initializer_list<std::pair<void *, size_t>> preallocated_blocks)
{
    size_t preallocd = 0;

    BlockHeader **ptr = &this->head;
    for (const auto &b : preallocated_blocks) {
        if (b.second < sizeof(LinearAllocator::BlockHeader))
            continue;

        *ptr = adopt_memory(b.first, b.second);
        preallocd += (*ptr)->capacity;

        ptr = &((*ptr)->next);
    }

    if (preallocd < this->first_block_capacity)
        *ptr = this->alloc_next_block(this->first_block_capacity - preallocd);

    return true;
}

LinearAllocator::BlockHeader *LinearAllocator::alloc_next_block(size_t capacity)
{
    // BlockHeader has alignas(MAX_ALIGNMENT)
    // -> it is padded to MAX_ALIGNMENT
    // The memory block itself (behind the header) is also aligned to MAX_ALIGNMENT.
    void *ptr = aligned_alloc_fn(MAX_ALIGNMENT, sizeof(BlockHeader) + capacity);
    if (ptr == nullptr) {
        return nullptr;
    }

    ++this->allocated_blocks;

    return adopt_memory(ptr, sizeof(BlockHeader) + capacity);
}

// Returns how many bytes of padding have to be added to *offset* to be aligned to *alignment*
// Assumes that *offset* = 0 is always aligned (i.e. aligned to MAX_ALIGNMENT)
// *alignment* must be a power of two!
static size_t get_padding(size_t offset, size_t alignment)
{
    auto misaligned = offset & (alignment - 1);
    auto padding = misaligned == 0 ? 0 : (alignment - misaligned);
    return padding;
}

void *LinearAllocator::aligned_alloc(size_t align, size_t size)
{
    align = std::max(align, min_alignment);

    // Checking this here makes sure that min_alignment (user-controlled) is less than MAX_ALIGNMENT.
    if (align > MAX_ALIGNMENT) {
        return nullptr;
    }

    // Don't allow allocating 0 byte blocks:
    // If we don't write a canary and the alignment is fine,
    // we would hand out pointers to the same block more than once.
    // That's probably a bad idea for aliasing reasons even though we never free blocks?
    if (size == 0) {
        return nullptr;
    }

    // Alignment must be power of two
    if (!std::has_single_bit(align)) {
        return nullptr;
    }

    // Allocating the first block failed.
    // TODO: Do we have to check this every time?
    //       Should we try to allocate the first block now?
    if (this->head == nullptr) {
        return nullptr;
    }

#ifdef DEBUG_FS_ENABLE
    // Include heap canary when calculating (mis)alignment
    size += CANARY_SIZE;
#endif

    size_t padding = 0;
    uint8_t *start = nullptr;

    BlockHeader *block = this->head;

    // Search block to allocate into
    // TODO: It's really ugly that this is an unbounded loop
    while (true) {
        start = block->memory + block->used;

#ifdef DEBUG_FS_ENABLE
        // Include heap canary when calculating (mis)alignment
        start += CANARY_SIZE;
#endif
        padding = get_padding(reinterpret_cast<size_t>(start), align);

        // TODO: search in all blocks for the one with the minimum required padding to fit this allocation?

        if ((size + padding) <= (block->capacity - block->used)) {
            break; // The allocation will fit in this block. Allocate.
        }

        // This block is full

        // Check next block
        if (block->next != nullptr) {
            block = block->next;
            continue;
        }

        // This is the last block and it is full. Allocate a new block.

        // Recalculate size to be allocated:
        // A new block starts aligned but this can result in additional padding being required due to the canary.
        auto alloc_size = size;
#ifdef DEBUG_FS_ENABLE
        alloc_size += get_padding(CANARY_SIZE, align);
#endif

        if (alloc_size <= this->block_capacity) {
            block->next = this->alloc_next_block(this->block_capacity);
            if (block->next == nullptr) {
                return nullptr; // OOM
            }

            // Advance into new block.
            // This skips one useless iteration.
            block = block->next;
            continue;
        }

#ifdef DEBUG_FS_ENABLE
        // Undo including heap canary when passing through to allocation function.
        size -= CANARY_SIZE;
#endif

        // alloc_size is more than what fits into one block.
        // Because we never free anyway, we can just allocate this directly (i.e. without BlockHeader etc.)
        return aligned_alloc_fn(align, size + get_padding(size, align));
    }

#ifdef DEBUG_FS_ENABLE
    // Undo including heap canary
    size -= CANARY_SIZE;
    start -= CANARY_SIZE;

    // Write heap canary. Write to stack array and then memcpy to not access IRAM misaligned.
    uint8_t canary[CANARY_SIZE];
    uint8_t *canary_ptr = canary;
    // 4 bits of payload per canary byte
    for (int i = MAX_ALLOC_BITS - 4; i >= 0; i -= 4) {
        *canary_ptr = 0xC0 | ((size >> i) & 0x0F);
        ++canary_ptr;
    }

    // TODO this only works with MAX_ALIGNMENT up to 16
    static_assert(MAX_ALIGNMENT <= 16);
    *canary_ptr = 0xC0 | ((align - 1) & 0x0F);
    ++canary_ptr;

    memcpy(start, canary, CANARY_SIZE);
    start += CANARY_SIZE;
    block->used += CANARY_SIZE;
#endif

    // Write padding
    // TODO 4 byte padding counter only works with MAX_ALIGNMENT up to 16
    uint8_t pad[MAX_ALIGNMENT];

    for (size_t i = 0; i < padding; ++i) {
        pad[i] = 0xB0 | static_cast<uint8_t>(padding - i);
    }
    memcpy(start, pad, padding);
    start += padding;
    block->used += padding;

    // *Finally* allocate the requested size.
    block->used += size;
    this->padding_overhead += padding;
    ++allocs;

    return start;
}

void *LinearAllocator::alloc(size_t size)
{
    // Have to check size == 0 here to prevent undefined left shift.
    if (size == 0) {
        return nullptr;
    }

    size_t align = 1 << std::countr_zero(size);
    align = std::min(align, alignof(max_align_t));
    return this->aligned_alloc(align, size);
}

#ifdef DEBUG_FS_ENABLE
bool LinearAllocator::check_integrity() const
{
    size_t allocs_seen = 0;
    size_t padding_seen = 0;

    BlockHeader *block = this->head;
    while (block != nullptr) {
        size_t offset = 0;

        while (offset < block->used) {
            ++allocs_seen;

            // Check canary
            uint8_t canary[CANARY_SIZE];
            memcpy(canary, block->memory + offset, CANARY_SIZE);

            size_t next_alloc_size = 0;

            for (size_t i = 0; i < MAX_ALLOC_BITS / 4; ++i) {
                if ((canary[i] & 0xF0) != 0xC0) {
                    logger.printfln("Expected canary byte at offset %zu but found byte 0x%02x", offset + i, canary[i]);
                    return false;
                }
                next_alloc_size |= static_cast<size_t>((canary[i] & 0x0F) << (MAX_ALLOC_BITS - 4 - i * 4));
            }
            size_t next_padding = (canary[CANARY_SIZE - 1] & 0x0F) + 1;
            offset += CANARY_SIZE;

            auto padding = get_padding(offset, next_padding);

            if (next_alloc_size + padding > block->capacity - offset) {
                logger.printfln("Canary at offset %zu encoded allocation size %zu and padding %zu but block only has %zu bytes left! Was the canary overwritten?", offset - CANARY_SIZE, next_alloc_size, next_padding, block->capacity - offset);
                return false;
            }

            // Check padding
            uint8_t pad[LinearAllocator::MAX_ALIGNMENT];
            memcpy(pad, block->memory + offset, padding);
            uint8_t *pad_ptr = pad;

            while (padding > 0) {
                auto expected_padding_byte = (0xB0 | (padding & 0x0F));
                if (*pad_ptr != expected_padding_byte) {
                    logger.printfln("Expected padding byte 0x%02zx at offset %zu but found byte 0x%02x", expected_padding_byte, offset, *pad_ptr);
                    return false;
                }
                ++pad_ptr;
                ++padding_seen;
                --padding;
                ++offset;
            }

            // Skip allocated block
            offset += next_alloc_size;
        }


        block = block->next;
    }

    // Check bookkeeping
    if (allocs_seen != allocs) {
        logger.printfln("Seen %zu allocations but %zu allocations were tracked", allocs_seen, allocs);
        return false;
    }

    if (padding_seen != padding_overhead) {
        logger.printfln("Seen %zu padding bytes but %zu were tracked", padding_seen, padding_overhead);
        return false;
    }

    return true;
}
#endif

void LinearAllocator::print_statistics() {
    logger.printfln("allocs %zu padding overhead %zu blocks %zu", allocs, padding_overhead, allocated_blocks);
    BlockHeader *ptr = this->head;
    size_t i = 0;
    while (ptr != nullptr) {
        logger.printfln("    Block %zu", i);
        logger.printfln("        capacity %zu used %zu", ptr->capacity, ptr->used);
        ++i;
        ptr = ptr->next;
    }
}
