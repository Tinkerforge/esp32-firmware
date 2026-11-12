/* esp32-firmware
 * Copyright (C) 2025 Mattias Schäffersmann <mattias@tinkerforge.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "config/slot_allocator.h"

#include "event_log_prefix.h"
#include "main_dependencies.h"
#include "tools/malloc.h"

#include "gcc_warnings.h"

[[gnu::section(".iram.data")]] static Superblock<Config::ConfUint>   first_superblock_ConfUint;
[[gnu::section(".iram.data")]] static Superblock<Config::ConfInt>    first_superblock_ConfInt;
[[gnu::section(".iram.data")]] static Superblock<Config::ConfUint32> first_superblock_ConfUint32;
[[gnu::section(".iram.data")]] static Superblock<Config::ConfInt32>  first_superblock_ConfInt32;
[[gnu::section(".iram.data")]] static Superblock<Config::ConfFloat>  first_superblock_ConfFloat;
[[gnu::section(".iram.data")]] static Superblock<Config::ConfString> first_superblock_ConfString;
[[gnu::section(".iram.data")]] static Superblock<Config::ConfArray>  first_superblock_ConfArray;
[[gnu::section(".iram.data")]] static Superblock<Config::ConfObject> first_superblock_ConfObject;
[[gnu::section(".iram.data")]] static Superblock<Config::ConfUnion>  first_superblock_ConfUnion;
[[gnu::section(".iram.data")]] static Superblock<Config::ConfUint53> first_superblock_ConfUint53;
[[gnu::section(".iram.data")]] static Superblock<Config::ConfInt52>  first_superblock_ConfInt52;
[[gnu::section(".iram.data")]] static Superblock<Config::ConfTuple>  first_superblock_ConfTuple;
[[gnu::section(".iram.data")]] static Superblock<Config>             first_superblock_Config;

template<> Superblock<Config::ConfUint>   *RootBlock<Config::ConfUint>::first_superblock   = &first_superblock_ConfUint;
template<> Superblock<Config::ConfInt>    *RootBlock<Config::ConfInt>::first_superblock    = &first_superblock_ConfInt;
template<> Superblock<Config::ConfUint32> *RootBlock<Config::ConfUint32>::first_superblock = &first_superblock_ConfUint32;
template<> Superblock<Config::ConfInt32>  *RootBlock<Config::ConfInt32>::first_superblock  = &first_superblock_ConfInt32;
template<> Superblock<Config::ConfFloat>  *RootBlock<Config::ConfFloat>::first_superblock  = &first_superblock_ConfFloat;
template<> Superblock<Config::ConfString> *RootBlock<Config::ConfString>::first_superblock = &first_superblock_ConfString;
template<> Superblock<Config::ConfArray>  *RootBlock<Config::ConfArray>::first_superblock  = &first_superblock_ConfArray;
template<> Superblock<Config::ConfObject> *RootBlock<Config::ConfObject>::first_superblock = &first_superblock_ConfObject;
template<> Superblock<Config::ConfUnion>  *RootBlock<Config::ConfUnion>::first_superblock  = &first_superblock_ConfUnion;
template<> Superblock<Config::ConfUint53> *RootBlock<Config::ConfUint53>::first_superblock = &first_superblock_ConfUint53;
template<> Superblock<Config::ConfInt52>  *RootBlock<Config::ConfInt52>::first_superblock  = &first_superblock_ConfInt52;
template<> Superblock<Config::ConfTuple>  *RootBlock<Config::ConfTuple>::first_superblock  = &first_superblock_ConfTuple;
template<> Superblock<Config>             *RootBlock<Config>::first_superblock             = &first_superblock_Config;

template<typename ConfT> uint16_t RootBlock<ConfT>::first_free_slot  = 0;

#if MODULE_DEBUG_AVAILABLE()
template<typename ConfT> uint16_t RootBlock<ConfT>::allocated_blocks = 0;
template<typename ConfT> uint16_t RootBlock<ConfT>::used_slots       = 0;
template<typename ConfT> uint16_t RootBlock<ConfT>::last_used_slot   = 0;
template<typename ConfT> uint16_t RootBlock<ConfT>::slots_hwm        = 0;

#ifdef DEBUG_FS_ENABLE
template<typename ConfT> size_t RootBlock<ConfT>::allocs             = 0;
template<typename ConfT> size_t RootBlock<ConfT>::frees              = 0;
#endif
#endif

template<typename ConfigT>
uint16_t nextSlots(uint16_t batch_size)
{
    ASSERT_MAIN_THREAD();
    assert(batch_size > 0 && batch_size <= SlotConfig<ConfigT>::slots_per_block);

    Superblock<ConfigT> *superblock = RootBlock<ConfigT>::first_superblock;
    size_t superblock_offset = 0;

    size_t block_i = RootBlock<ConfigT>::first_free_slot / SlotConfig<ConfigT>::slots_per_block;
    size_t slot_i  = RootBlock<ConfigT>::first_free_slot % SlotConfig<ConfigT>::slots_per_block;

    bool first_free_set = false;

    while (block_i >= SlotConfig<ConfigT>::blocks_per_superblock) {
        if (superblock->next_superblock == nullptr) {
            // When the first free slot points into the next unallocated block,
            // don't decrease block_i so that the following block iteration is skipped
            // when looking for a free slot and a new superblock gets allocated first.

            if (block_i != SlotConfig<ConfigT>::blocks_per_superblock || slot_i != 0) {
                esp_system_abort("Unexpected block_i or slot_i: not first in next block");
            }

            break;
        }

        block_i -= SlotConfig<ConfigT>::blocks_per_superblock;
        superblock = superblock->next_superblock;
        superblock_offset++;
    }

    while (true) {
        const size_t superblock_slot_index_offset = superblock_offset * SlotConfig<ConfigT>::slots_per_superblock;

        for (; block_i < SlotConfig<ConfigT>::blocks_per_superblock; block_i++) {
            typename ConfigT::Slot *block = superblock->blocks[block_i];

            if (block == nullptr) {
                block = ConfigT::allocSlotBuf(SlotConfig<ConfigT>::slots_per_block);

                if (block == nullptr) {
                    esp_system_abort("Couldn't allocate slot buffer");
                }

                superblock->blocks[block_i] = block;

#if MODULE_DEBUG_AVAILABLE()
                RootBlock<ConfigT>::allocated_blocks++;
#endif
            }

            const size_t block_slot_index_offset = superblock_slot_index_offset + block_i * SlotConfig<ConfigT>::slots_per_block;

            for (; slot_i <= SlotConfig<ConfigT>::slots_per_block - batch_size; slot_i++) {
                const uint16_t idx = static_cast<uint16_t>(block_slot_index_offset + slot_i);

                if (ConfigT::slotEmpty(block + slot_i, idx)) {
                    uint16_t last_idx;

                    if (batch_size == 1) {
                        RootBlock<ConfigT>::first_free_slot = idx + 1;
                        last_idx = idx;
                    } else {
                        uint16_t free_found = 1;

                        for (; free_found < batch_size; free_found++) {
                            if (!ConfigT::slotEmpty(block + slot_i + free_found, idx + free_found)) {
                                break;
                            }
                        }

                        if (free_found < batch_size) {
                            if (!first_free_set) {
                                RootBlock<ConfigT>::first_free_slot = idx;
                                first_free_set = true;
                            }

                            slot_i += free_found;
                            continue;
                        }

                        if (!first_free_set) {
                            RootBlock<ConfigT>::first_free_slot = idx + batch_size;
                        }

                        last_idx = static_cast<uint16_t>(idx + batch_size - 1);
                    }

#if !MODULE_DEBUG_AVAILABLE()
                    (void) last_idx; //Avoid unused warning
#else
                    RootBlock<ConfigT>::used_slots += batch_size;
#ifdef DEBUG_FS_ENABLE
                    RootBlock<ConfigT>::allocs += batch_size;
#endif

                    if (last_idx > RootBlock<ConfigT>::last_used_slot) {
                        RootBlock<ConfigT>::last_used_slot = last_idx;

                        if (last_idx > RootBlock<ConfigT>::slots_hwm) {
                            RootBlock<ConfigT>::slots_hwm = last_idx;
                        }
                    }
#endif
                    return idx;
                }
            }

            if (slot_i < SlotConfig<ConfigT>::slots_per_block && !first_free_set) {
                do {
                    const size_t idx = block_slot_index_offset + slot_i;

                    if (ConfigT::slotEmpty(block + slot_i, idx)) {
                        RootBlock<ConfigT>::first_free_slot = static_cast<uint16_t>(idx);
                        first_free_set = true;
                        break;
                    }

                    slot_i++;
                } while (slot_i < SlotConfig<ConfigT>::slots_per_block);
            }

            slot_i = 0;
        }

        if (superblock->next_superblock == nullptr) {
            Superblock<ConfigT> *new_superblock = static_cast<decltype(superblock)>(calloc_32bit_addressed(1, sizeof(*superblock)));

            if (new_superblock == nullptr) {
                esp_system_abort("Couldn't allocate new superblock");
            }

            superblock->next_superblock = new_superblock;
            logger.printfln("Allocated new superblock at %p for %s", static_cast<void *>(new_superblock), ConfigT::variantName);
        }
        superblock = superblock->next_superblock;
        superblock_offset++;

        block_i = 0;
    }
}

template uint16_t nextSlots<Config::ConfUint  >(uint16_t batch_size);
template uint16_t nextSlots<Config::ConfInt   >(uint16_t batch_size);
template uint16_t nextSlots<Config::ConfUint32>(uint16_t batch_size);
template uint16_t nextSlots<Config::ConfInt32 >(uint16_t batch_size);
template uint16_t nextSlots<Config::ConfFloat >(uint16_t batch_size);
template uint16_t nextSlots<Config::ConfString>(uint16_t batch_size);
template uint16_t nextSlots<Config::ConfArray >(uint16_t batch_size);
template uint16_t nextSlots<Config::ConfObject>(uint16_t batch_size);
template uint16_t nextSlots<Config::ConfUnion >(uint16_t batch_size);
template uint16_t nextSlots<Config::ConfUint53>(uint16_t batch_size);
template uint16_t nextSlots<Config::ConfInt52 >(uint16_t batch_size);
template uint16_t nextSlots<Config::ConfTuple >(uint16_t batch_size);
template uint16_t nextSlots<Config            >(uint16_t batch_size);

template<typename ConfigT>
typename ConfigT::Slot *get_slot(uint16_t idx)
{
    Superblock<ConfigT> *superblock = RootBlock<ConfigT>::first_superblock;

    size_t block_idx = idx / SlotConfig<ConfigT>::slots_per_block;
    size_t slot_idx  = idx % SlotConfig<ConfigT>::slots_per_block;

    while (block_idx >= SlotConfig<ConfigT>::blocks_per_superblock) {
        block_idx -= SlotConfig<ConfigT>::blocks_per_superblock;
        superblock = superblock->next_superblock;
    }

    return superblock->blocks[block_idx] + slot_idx;
}

template Config::ConfUint::Slot   *get_slot<Config::ConfUint>(uint16_t idx);
template Config::ConfInt::Slot    *get_slot<Config::ConfInt>(uint16_t idx);
template Config::ConfUint32::Slot *get_slot<Config::ConfUint32>(uint16_t idx);
template Config::ConfInt32::Slot  *get_slot<Config::ConfInt32>(uint16_t idx);
template Config::ConfFloat::Slot  *get_slot<Config::ConfFloat>(uint16_t idx);
template Config::ConfString::Slot *get_slot<Config::ConfString>(uint16_t idx);
template Config::ConfArray::Slot  *get_slot<Config::ConfArray>(uint16_t idx);
template Config::ConfObject::Slot *get_slot<Config::ConfObject>(uint16_t idx);
template Config::ConfUnion::Slot  *get_slot<Config::ConfUnion>(uint16_t idx);
template Config::ConfUint53::Slot *get_slot<Config::ConfUint53>(uint16_t idx);
template Config::ConfInt52::Slot  *get_slot<Config::ConfInt52>(uint16_t idx);
template Config::ConfTuple::Slot  *get_slot<Config::ConfTuple>(uint16_t idx);
template Config::Slot             *get_slot<Config>(uint16_t idx);

#if MODULE_DEBUG_AVAILABLE() && defined(DEBUG_FS_ENABLE)
[[gnu::noinline]]
[[gnu::noreturn]]
void abort_on_notify_free_error(const char *variant_name, uint16_t index, uint16_t batch_size)
{
    char msg[96];
    snprintf(msg, std::size(msg), "Notify free for slot that is not free: %s @ %hu in batch of %hu", variant_name, index, batch_size);
    esp_system_abort(msg);
}
#endif

#ifdef DEBUG_FS_ENABLE
template<typename ConfigT>
static bool check_slot_accounting()
{
    bool success = true;
    size_t last_idx = RootBlock<ConfigT>::first_free_slot;
    Superblock<ConfigT> *superblock = RootBlock<ConfigT>::first_superblock;
    size_t superblock_idx = 0;
    size_t block_count = 0;
    size_t last_used_slot = 0;

#if MODULE_DEBUG_AVAILABLE()
    size_t used_slots = 0;

    while (superblock) {
        for (size_t block_i = 0; block_i < SlotConfig<ConfigT>::blocks_per_superblock; block_i++) {
            typename ConfigT::Slot *block = superblock->blocks[block_i];

            if (block == nullptr) {
                break;
            }

            block_count++;

            for (size_t slot_i = 0; slot_i < SlotConfig<ConfigT>::slots_per_block; slot_i++) {
                const size_t abs_slot_idx = superblock_idx * SlotConfig<ConfigT>::slots_per_superblock + block_i * SlotConfig<ConfigT>::slots_per_block + slot_i;
                if (!ConfigT::slotEmpty(block + slot_i, abs_slot_idx)) {
                    ConfigT::slotDebugHook(block + slot_i, abs_slot_idx);
                    used_slots++;
                    last_used_slot = abs_slot_idx;
                }
            }
        }

        superblock = superblock->next_superblock;
        superblock_idx++;
    }

    if (used_slots != RootBlock<ConfigT>::used_slots) {
        logger.printfln("used_slots mismatch for %s. Counted %zu, expected %hu. allocs %zu  frees %zu  diff %zu",
            ConfigT::variantName, used_slots, RootBlock<ConfigT>::used_slots, RootBlock<ConfigT>::allocs, RootBlock<ConfigT>::frees, RootBlock<ConfigT>::allocs - RootBlock<ConfigT>::frees);
        success = false;
    }

    if (block_count != RootBlock<ConfigT>::allocated_blocks) {
        logger.printfln("allocated_blocks mismatch for %s. Counted %zu, expected %hu.", ConfigT::variantName, block_count, RootBlock<ConfigT>::allocated_blocks);
        success = false;
    }

    if (last_used_slot != RootBlock<ConfigT>::last_used_slot) {
        logger.printfln("last_used_slot mismatch for %s. Found %zu, expected %hu.", ConfigT::variantName, last_used_slot, RootBlock<ConfigT>::last_used_slot);
        success = false;
    }

#endif

    if (RootBlock<ConfigT>::frees > RootBlock<ConfigT>::allocs) {
        logger.printfln("alloc/free mismatch for %s. %zu frees > %zu allocs.", ConfigT::variantName, RootBlock<ConfigT>::frees, RootBlock<ConfigT>::allocs);
        success = false;
    }

    superblock = RootBlock<ConfigT>::first_superblock;
    superblock_idx = 0;

    while (true) {
        if (superblock == nullptr) {
            if (last_idx == 0) {
                // Points to first slot in unallocated superblock, which is ok.
                return success;
            }

            logger.printfln("First free slot %zu points into an unallocated superblock for %s", last_idx, ConfigT::variantName);
            success = false;
            return success;
        }

        for (size_t block_i = 0; block_i < SlotConfig<ConfigT>::blocks_per_superblock; block_i++) {
            typename ConfigT::Slot *block = superblock->blocks[block_i];

            if (block == nullptr) {
                if (last_idx == 0) {
                    // Points to first slot in unallocated block, which is ok.
                    return success;
                }

                logger.printfln("First free slot %zu points into an unallocated block %zu for %s", last_idx, block_i, ConfigT::variantName);
                success = false;
                return success;
            }

            for (size_t slot_i = 0; slot_i < SlotConfig<ConfigT>::slots_per_block; slot_i++) {
                if (slot_i >= last_idx) {
                    return success;
                }

                const size_t abs_slot_idx = superblock_idx * SlotConfig<ConfigT>::slots_per_superblock + block_i * SlotConfig<ConfigT>::slots_per_block + slot_i;
                if (ConfigT::slotEmpty(block + slot_i, abs_slot_idx)) {
                    if (slot_i != last_idx) {
                        logger.printfln("First free slot mismatch. Expected %zu but found %zu in block %zu for %s.", last_idx, slot_i, block_i, ConfigT::variantName);
                        success = false;
                    }

                    return success;
                }
            }

            last_idx -= SlotConfig<ConfigT>::slots_per_block;
        }

        superblock = superblock->next_superblock;
        superblock_idx++;
    }

    return success;
}
#endif

#ifdef DEBUG_FS_ENABLE
template<> Superblock<SelfTest> *RootBlock<SelfTest>::first_superblock = nullptr;
template uint16_t nextSlots<SelfTest>(uint16_t batch_size);
template SelfTest::Slot *get_slot<SelfTest>(uint16_t idx);

SelfTest::Slot *SelfTest::allocSlotBuf(size_t elements)
{
    const size_t buf_size = sizeof(SelfTest::Slot) * elements;
    auto result = heap_caps_calloc_prefer(buf_size, 2, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT, MALLOC_CAP_SPIRAM);
    return static_cast<SelfTest::Slot *>(result);
}

uint16_t SelfTest::allocSlot(uint16_t batch_size)
{
    const uint16_t idx = nextSlots<SelfTest>(batch_size);

    for (uint16_t i = 0; i < batch_size; i++) {
        SelfTest::Slot *slot = get_slot<SelfTest>(idx + i);
        assert(!slot->inuse);
        slot->inuse = true;
    }

    return idx;
}

void SelfTest::freeSlot(uint16_t idx, uint16_t batch_size)
{
    for (uint16_t i = 0; i < batch_size; i++) {
        SelfTest::Slot *slot = get_slot<SelfTest>(idx + i);
        assert(slot->inuse);
        slot->inuse = false;
    }

    notify_free_slots<SelfTest>(idx, batch_size);
}

static void clear_dummy_superblock(Superblock<SelfTest> *superblock, size_t superblock_index)
{
    if (superblock->next_superblock != nullptr) {
        clear_dummy_superblock(superblock->next_superblock, superblock_index + 1);
        free(superblock->next_superblock);
        superblock->next_superblock = nullptr;
    }

    for (size_t i = 0; i < SlotConfig<SelfTest>::blocks_per_superblock; i++) {
        SelfTest::Slot *block = superblock->blocks[i];

        if (block == nullptr) {
            continue;
        }

        for (size_t j = 0; j < SlotConfig<SelfTest>::slots_per_block; j++) {
            if (block[j].inuse) {
                dbg("super %zu -> block %zu -> slot %zu still in use", superblock_index, i, j);
            }
        }

        free(block);
        superblock->blocks[i] = nullptr;
    }
}

static void check_dummy_slots()
{
    RootBlock<SelfTest>::first_superblock = static_cast<decltype(RootBlock<SelfTest>::first_superblock)>(calloc_32bit_addressed(1, sizeof(*RootBlock<SelfTest>::first_superblock)));

    uint16_t idx;

    // Test allocating full blocks and full superblock.
    // Test single tail allocations.

    for (uint16_t i = 0; i < 8; i++) {
        idx = SelfTest::allocSlot(1);

        if (idx != i) {
            esp_system_abortf<64>("Alloc index failed: expected %hu, got %hu", i, idx);
        }

        if (!check_slot_accounting<SelfTest>()) {
            esp_system_abortf<64>("Single alloc failed at index %zu", i);
        }
    }

    // xxxx xxxx

    // Test the self-test.

    get_slot<SelfTest>(2)->inuse = false;

    if (check_slot_accounting<SelfTest>()) {
        esp_system_abort("check_slot_accounting passed even though it should have failed");
    }

    logger.printfln("Error test ok, ignore previous errors");

    // xx|x xxxx

    // Restore consistent state after test self-test.

    get_slot<SelfTest>(2)->inuse = true;

    if (!check_slot_accounting<SelfTest>()) {
        esp_system_abort("check_slot_accounting failed after its own self-test");
    }

    // xxxx xxxx

    // Test single middle free.

    SelfTest::freeSlot(2, 1);

    if (!check_slot_accounting<SelfTest>()) {
        esp_system_abort("Single free failed at index 2");
    }

    // xx.x xxxx

    // Test single middle allocation.

    idx = SelfTest::allocSlot(1);

    if (idx != 2) {
        esp_system_abortf<64>("Single alloc index failed: expected 2, got %hu", idx);
    }

    if (!check_slot_accounting<SelfTest>()) {
        esp_system_abort("Single alloc failed at index 2");
    }

    // xxxx xxxx

    // Test single tail allocation into new superblock.

    idx = SelfTest::allocSlot(1);

    if (idx != 8) {
        esp_system_abortf<64>("Single alloc index failed: expected 8, got %hu", idx);
    }

    if (!check_slot_accounting<SelfTest>()) {
        esp_system_abort("Single alloc failed at index 8");
    }

    // xxxx xxxx  x...

    // Test notify_free_slots first_free_slot updates.
    // Test single head frees until empty.

    for (uint16_t i = 0; i < 9; i++) {
        SelfTest::freeSlot(i, 1);

        if (!check_slot_accounting<SelfTest>()) {
            esp_system_abortf<64>("Single free failed at index %zu", i);
        }
    }

    // .... .... ....

    // Fill again.

    for (uint16_t i = 0; i < 9; i++) {
        idx = SelfTest::allocSlot(1);

        if (idx != i) {
            esp_system_abortf<64>("Alloc index failed: expected %hu, got %hu", i, idx);
        }

        if (!check_slot_accounting<SelfTest>()) {
            esp_system_abortf<64>("Single alloc failed at index %zu", i);
        }
    }

    // xxxx xxxx  x...

    // Test single tail frees until empty.

    for (uint16_t i = 9; i > 0; i--) {
        SelfTest::freeSlot(i - 1, 1);

        if (!check_slot_accounting<SelfTest>()) {
            esp_system_abortf<64>("Single free failed at index %hu", i - 1);
        }
    }

    // .... ....  ....

    // Test batch allocation at first free slot and tail allocation.

    for (uint16_t i = 0; i < 8; i += 4) {
        idx = SelfTest::allocSlot(4);

        if (idx != i) {
            esp_system_abortf<64>("Block alloc 4 at wrong index: expected %hu, got %hu", i, idx);
        }

        if (!check_slot_accounting<SelfTest>()) {
            esp_system_abortf<64>("Block alloc failed at index %hu", i);
        }
    }

    // xxxx xxxx  ....

    // Test batch middle free.

    SelfTest::freeSlot(2, 2);

    if (!check_slot_accounting<SelfTest>()) {
        esp_system_abort("Block free failed at index 2");
    }

    // xx.. xxxx  ....

    // Test batch allocation that doesn't fit into the first free slot.
    // Test tail batch allocation.

    idx = SelfTest::allocSlot(4);

    if (idx != 8) {
        esp_system_abortf<64>("Block alloc 4 at wrong index: expected 8, got %hu", idx);
    }

    if (!check_slot_accounting<SelfTest>()) {
        esp_system_abort("Block alloc failed at index 8");
    }

    // xx.. xxxx  xxxx

    // Test batch middle alloc.

    idx = SelfTest::allocSlot(2);

    if (idx != 2) {
        esp_system_abortf<64>("Block alloc 2 at wrong index: expected 2, got %hu", idx);
    }

    if (!check_slot_accounting<SelfTest>()) {
        esp_system_abort("Block alloc failed at index 2");
    }

    // xxxx xxxx  xxxx

    // Test freeing head batch until empty.

    for (uint16_t i = 0; i < 12; i += 2) {
        SelfTest::freeSlot(i, 2);

        if (!check_slot_accounting<SelfTest>()) {
            esp_system_abortf<64>("Block free failed at index %hu", i);
        }
    }

    // .... ....  ....

    // Fill again.

    for (uint16_t i = 0; i < 8; i += 2) {
        idx = SelfTest::allocSlot(2);

        if (idx != i) {
            esp_system_abortf<64>("Block alloc 2 at wrong index: expected %hu, got %hu", i, idx);
        }

        if (!check_slot_accounting<SelfTest>()) {
            esp_system_abortf<64>("Block alloc failed at index %hu", i);
        }
    }

    // xxxx xxxx  ....

    // Test freeing tail batch until empty.

    for (uint16_t i = 8; i > 0; i -= 2) {
        SelfTest::freeSlot(i - 2, 2);

        if (!check_slot_accounting<SelfTest>()) {
            esp_system_abortf<64>("Block free failed at index %hu", i - 2);
        }
    }

    // .... ....  ....

    logger.printfln("Slot allocator self-test passed");

    clear_dummy_superblock(RootBlock<SelfTest>::first_superblock, 0);
    free(RootBlock<SelfTest>::first_superblock);
    RootBlock<SelfTest>::first_superblock = nullptr;
}
#endif

void config_post_setup()
{
#ifdef DEBUG_FS_ENABLE
    check_dummy_slots();

    task_scheduler.scheduleUncancelable([]() {
        bool accounting_ok = true;
        accounting_ok &= check_slot_accounting<Config::ConfUint>();
        accounting_ok &= check_slot_accounting<Config::ConfInt>();
        accounting_ok &= check_slot_accounting<Config::ConfUint32>();
        accounting_ok &= check_slot_accounting<Config::ConfInt32>();
        accounting_ok &= check_slot_accounting<Config::ConfFloat>();
        accounting_ok &= check_slot_accounting<Config::ConfString>();
        accounting_ok &= check_slot_accounting<Config::ConfArray>();
        accounting_ok &= check_slot_accounting<Config::ConfObject>();
        accounting_ok &= check_slot_accounting<Config::ConfUnion>();
        accounting_ok &= check_slot_accounting<Config::ConfUint53>();
        accounting_ok &= check_slot_accounting<Config::ConfInt52>();
        accounting_ok &= check_slot_accounting<Config::ConfTuple>();
        accounting_ok &= check_slot_accounting<Config>();
        if (!accounting_ok)
            debug.report_slot_accounting_error();
        slotDebugHookDone();
    }, 0_ms, 10_s);
#endif
}
