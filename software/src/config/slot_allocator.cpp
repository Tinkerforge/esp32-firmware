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

[[gnu::section(".iram.data")]] static Superblock<Config::ConfUint>   first_superblock_ConfUint;
[[gnu::section(".iram.data")]] static Superblock<Config::ConfInt>    first_superblock_ConfInt;
[[gnu::section(".iram.data")]] static Superblock<Config::ConfFloat>  first_superblock_ConfFloat;
[[gnu::section(".iram.data")]] static Superblock<Config::ConfString> first_superblock_ConfString;
[[gnu::section(".iram.data")]] static Superblock<Config::ConfArray>  first_superblock_ConfArray;
[[gnu::section(".iram.data")]] static Superblock<Config::ConfObject> first_superblock_ConfObject;
[[gnu::section(".iram.data")]] static Superblock<Config::ConfUnion>  first_superblock_ConfUnion;
[[gnu::section(".iram.data")]] static Superblock<Config::ConfUint53> first_superblock_ConfUint53;
[[gnu::section(".iram.data")]] static Superblock<Config::ConfInt52>  first_superblock_ConfInt52;

template<> Superblock<Config::ConfUint>   *RootBlock<Config::ConfUint>::first_superblock   = &first_superblock_ConfUint;
template<> Superblock<Config::ConfInt>    *RootBlock<Config::ConfInt>::first_superblock    = &first_superblock_ConfInt;
template<> Superblock<Config::ConfFloat>  *RootBlock<Config::ConfFloat>::first_superblock  = &first_superblock_ConfFloat;
template<> Superblock<Config::ConfString> *RootBlock<Config::ConfString>::first_superblock = &first_superblock_ConfString;
template<> Superblock<Config::ConfArray>  *RootBlock<Config::ConfArray>::first_superblock  = &first_superblock_ConfArray;
template<> Superblock<Config::ConfObject> *RootBlock<Config::ConfObject>::first_superblock = &first_superblock_ConfObject;
template<> Superblock<Config::ConfUnion>  *RootBlock<Config::ConfUnion>::first_superblock  = &first_superblock_ConfUnion;
template<> Superblock<Config::ConfUint53>   *RootBlock<Config::ConfUint53>::first_superblock   = &first_superblock_ConfUint53;
template<> Superblock<Config::ConfInt52>    *RootBlock<Config::ConfInt52>::first_superblock    = &first_superblock_ConfInt52;

template<typename ConfT> uint16_t RootBlock<ConfT>::first_free_slot  = 0;
template<typename ConfT> uint16_t RootBlock<ConfT>::allocated_blocks = 0;

#if MODULE_DEBUG_AVAILABLE()
template<typename ConfT> uint16_t RootBlock<ConfT>::used_slots       = 0;
template<typename ConfT> uint16_t RootBlock<ConfT>::last_used_slot   = 0;
template<typename ConfT> uint16_t RootBlock<ConfT>::slots_hwm        = 0;

#ifdef DEBUG_FS_ENABLE
template<typename ConfT> size_t RootBlock<ConfT>::allocs             = 0;
template<typename ConfT> size_t RootBlock<ConfT>::frees              = 0;
#endif
#endif

template<typename ConfigT>
size_t nextSlot()
{
    ASSERT_MAIN_THREAD();

    Superblock<ConfigT> *superblock = RootBlock<ConfigT>::first_superblock;
    size_t superblock_offset = 0;

    size_t block_i = RootBlock<ConfigT>::first_free_slot / SlotConfig<ConfigT>::slots_per_block;
    size_t slot_i  = RootBlock<ConfigT>::first_free_slot % SlotConfig<ConfigT>::slots_per_block;

    while (block_i > SlotConfig<ConfigT>::blocks_per_superblock) {
        block_i -= SlotConfig<ConfigT>::blocks_per_superblock;
        superblock = superblock->next_superblock;
        superblock_offset++;
    }

    while (true) {
        for (; block_i < SlotConfig<ConfigT>::blocks_per_superblock; block_i++) {
            typename ConfigT::Slot *block = superblock->blocks[block_i];
            if (!block) {
                block = ConfigT::allocSlotBuf(SlotConfig<ConfigT>::slots_per_block);
                superblock->blocks[block_i] = block;
                RootBlock<ConfigT>::allocated_blocks++;
            }

            for (; slot_i < SlotConfig<ConfigT>::slots_per_block; slot_i++) {
                if (ConfigT::slotEmpty(block + slot_i)) {
                    size_t idx = superblock_offset * SlotConfig<ConfigT>::slots_per_superblock + block_i * SlotConfig<ConfigT>::slots_per_block + slot_i;
                    RootBlock<ConfigT>::first_free_slot = idx + 1;

#if MODULE_DEBUG_AVAILABLE()
                    RootBlock<ConfigT>::used_slots++;
#ifdef DEBUG_FS_ENABLE
                    RootBlock<ConfigT>::allocs++;
#endif

                    if (idx > RootBlock<ConfigT>::last_used_slot) {
                        RootBlock<ConfigT>::last_used_slot = idx;

                        if (idx > RootBlock<ConfigT>::slots_hwm) {
                            RootBlock<ConfigT>::slots_hwm = idx;
                        }
                    }
#endif
                    return idx;
                }
            }
            slot_i = 0;
        }

        if (!superblock->next_superblock) {
            Superblock<ConfigT> *new_superblock = static_cast<decltype(superblock)>(calloc_32bit_addressed(1, sizeof(*superblock)));
            superblock->next_superblock = new_superblock;
            logger.printfln("Allocated new superblock at %p for %s", new_superblock, ConfigT::variantName);
        }
        superblock = superblock->next_superblock;
        superblock_offset++;

        block_i = 0;
    }
}

template size_t nextSlot<Config::ConfUint>();
template size_t nextSlot<Config::ConfInt>();
template size_t nextSlot<Config::ConfFloat>();
template size_t nextSlot<Config::ConfString>();
template size_t nextSlot<Config::ConfArray>();
template size_t nextSlot<Config::ConfObject>();
template size_t nextSlot<Config::ConfUnion>();
template size_t nextSlot<Config::ConfUint53>();
template size_t nextSlot<Config::ConfInt52>();

template<typename ConfigT>
typename ConfigT::Slot *get_slot(size_t idx)
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

template Config::ConfUint::Slot   *get_slot<Config::ConfUint>(size_t idx);
template Config::ConfInt::Slot    *get_slot<Config::ConfInt>(size_t idx);
template Config::ConfFloat::Slot  *get_slot<Config::ConfFloat>(size_t idx);
template Config::ConfString::Slot *get_slot<Config::ConfString>(size_t idx);
template Config::ConfArray::Slot  *get_slot<Config::ConfArray>(size_t idx);
template Config::ConfObject::Slot *get_slot<Config::ConfObject>(size_t idx);
template Config::ConfUnion::Slot  *get_slot<Config::ConfUnion>(size_t idx);
template Config::ConfUint53::Slot   *get_slot<Config::ConfUint53>(size_t idx);
template Config::ConfInt52::Slot    *get_slot<Config::ConfInt52>(size_t idx);

#ifdef DEBUG_FS_ENABLE
template<typename ConfigT>
static void check_slot_accounting()
{
    size_t last_idx = RootBlock<ConfigT>::first_free_slot;
    Superblock<ConfigT> *superblock = RootBlock<ConfigT>::first_superblock;

#if MODULE_DEBUG_AVAILABLE()
    size_t used_slots = 0;

    while (superblock) {
        for (size_t block_i = 0; block_i < SlotConfig<ConfigT>::blocks_per_superblock; block_i++) {
            typename ConfigT::Slot *block = superblock->blocks[block_i];

            if (!block) {
                break;
            }

            for (size_t slot_i = 0; slot_i < SlotConfig<ConfigT>::slots_per_block; slot_i++) {
                if (!ConfigT::slotEmpty(block + slot_i)) {
                    used_slots++;
                }
            }
        }

        superblock = superblock->next_superblock;
    }

    if (used_slots != RootBlock<ConfigT>::used_slots) {
        logger.printfln("used_slots mismatch for %s. Counted %zu, expected %hu. allocs %zu  frees %zu  diff %zu",
            ConfigT::variantName, used_slots, RootBlock<ConfigT>::used_slots, RootBlock<ConfigT>::allocs, RootBlock<ConfigT>::frees, RootBlock<ConfigT>::allocs - RootBlock<ConfigT>::frees);
    }
#endif

    superblock = RootBlock<ConfigT>::first_superblock;

    while (true) {
        if (!superblock) {
            if (last_idx == 0) {
                // Points to first slot in unallocated superblock, which is ok.
                return;
            }

            logger.printfln("First free slot %zu points into an unallocated superblock for %s", last_idx, ConfigT::variantName);
            return;
        }

        for (size_t block_i = 0; block_i < SlotConfig<ConfigT>::blocks_per_superblock; block_i++) {
            typename ConfigT::Slot *block = superblock->blocks[block_i];

            if (!block) {
                if (last_idx == 0) {
                    // Points to first slot in unallocated block, which is ok.
                    return;
                }

                logger.printfln("First free slot %zu points into an unallocated block %zu for %s", last_idx, block_i, ConfigT::variantName);
                return;
            }

            for (size_t slot_i = 0; slot_i < SlotConfig<ConfigT>::slots_per_block; slot_i++) {
                if (slot_i >= last_idx) {
                    return;
                }
                if (ConfigT::slotEmpty(block + slot_i)) {
                    if (slot_i != last_idx) {
                        logger.printfln("First free slot mismatch. Expected %zu but found %zu in block %zu for %s.", last_idx, slot_i, block_i, ConfigT::variantName);
                    }

                    return;
                }
            }

            last_idx -= SlotConfig<ConfigT>::slots_per_block;
        }

        superblock = superblock->next_superblock;
    }

}
#endif

void config_post_setup()
{
#ifdef DEBUG_FS_ENABLE
    task_scheduler.scheduleWithFixedDelay([]() {
        check_slot_accounting<Config::ConfUint>();
        check_slot_accounting<Config::ConfInt>();
        check_slot_accounting<Config::ConfFloat>();
        check_slot_accounting<Config::ConfString>();
        check_slot_accounting<Config::ConfArray>();
        check_slot_accounting<Config::ConfObject>();
        check_slot_accounting<Config::ConfUnion>();
        check_slot_accounting<Config::ConfUint53>();
        check_slot_accounting<Config::ConfInt52>();
    }, 1_min, 1_min);
#endif
}
