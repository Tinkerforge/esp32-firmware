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

#pragma once

#include <stdint.h>

#include "config.h"
#include "config/private.h"
#include "main_available.h"

template<typename ConfigT>
struct SlotConfig;

template<> struct SlotConfig<Config::ConfUint> {
    static constexpr const size_t slots_per_superblock  = 2048;
    static constexpr const size_t slots_per_block       =  256;
    static constexpr const size_t blocks_per_superblock = slots_per_superblock / slots_per_block;;

    static_assert(slots_per_superblock % slots_per_block == 0);
    static_assert((slots_per_block & (slots_per_block - 1)) == 0);
};

template<> struct SlotConfig<Config::ConfInt> {
    static constexpr const size_t slots_per_superblock  = 512;
    static constexpr const size_t slots_per_block       =  64;
    static constexpr const size_t blocks_per_superblock = slots_per_superblock / slots_per_block;;

    static_assert(slots_per_superblock % slots_per_block == 0);
    static_assert((slots_per_block & (slots_per_block - 1)) == 0);
};

template<> struct SlotConfig<Config::ConfFloat> {
    static constexpr const size_t slots_per_superblock  = 1024;
    static constexpr const size_t slots_per_block       =  128;
    static constexpr const size_t blocks_per_superblock = slots_per_superblock / slots_per_block;;

    static_assert(slots_per_superblock % slots_per_block == 0);
    static_assert((slots_per_block & (slots_per_block - 1)) == 0);
};

template<> struct SlotConfig<Config::ConfString> {
    static constexpr const size_t slots_per_superblock  = 512;
    static constexpr const size_t slots_per_block       =  64;
    static constexpr const size_t blocks_per_superblock = slots_per_superblock / slots_per_block;;

    static_assert(slots_per_superblock % slots_per_block == 0);
    static_assert((slots_per_block & (slots_per_block - 1)) == 0);
};

template<> struct SlotConfig<Config::ConfArray> {
    static constexpr const size_t slots_per_superblock  = 256;
    static constexpr const size_t slots_per_block       =  32;
    static constexpr const size_t blocks_per_superblock = slots_per_superblock / slots_per_block;;

    static_assert(slots_per_superblock % slots_per_block == 0);
    static_assert((slots_per_block & (slots_per_block - 1)) == 0);
};

template<> struct SlotConfig<Config::ConfObject> {
    static constexpr const size_t slots_per_superblock  = 2048;
    static constexpr const size_t slots_per_block       =  256;
    static constexpr const size_t blocks_per_superblock = slots_per_superblock / slots_per_block;;

    static_assert(slots_per_superblock % slots_per_block == 0);
    static_assert((slots_per_block & (slots_per_block - 1)) == 0);
};

template<> struct SlotConfig<Config::ConfUnion> {
    static constexpr const size_t slots_per_superblock  = 128;
    static constexpr const size_t slots_per_block       =  16;
    static constexpr const size_t blocks_per_superblock = slots_per_superblock / slots_per_block;;

    static_assert(slots_per_superblock % slots_per_block == 0);
    static_assert((slots_per_block & (slots_per_block - 1)) == 0);
};

template<> struct SlotConfig<Config::ConfUint53> {
    static constexpr const size_t slots_per_superblock  = 128;
    static constexpr const size_t slots_per_block       =  16;
    static constexpr const size_t blocks_per_superblock = slots_per_superblock / slots_per_block;;

    static_assert(slots_per_superblock % slots_per_block == 0);
    static_assert((slots_per_block & (slots_per_block - 1)) == 0);
};

template<> struct SlotConfig<Config::ConfInt52> {
    static constexpr const size_t slots_per_superblock  = 128;
    static constexpr const size_t slots_per_block       =  16;
    static constexpr const size_t blocks_per_superblock = slots_per_superblock / slots_per_block;;

    static_assert(slots_per_superblock % slots_per_block == 0);
    static_assert((slots_per_block & (slots_per_block - 1)) == 0);
};

template<typename ConfigT>
struct Superblock {
    typename ConfigT::Slot *blocks[SlotConfig<ConfigT>::blocks_per_superblock];
    Superblock<ConfigT> *next_superblock;
};

template<typename ConfigT>
struct RootBlock {
    static Superblock<ConfigT> *first_superblock;
    static uint16_t allocated_blocks;
    static uint16_t first_free_slot;

#if MODULE_DEBUG_AVAILABLE()
    static uint16_t used_slots;
    static uint16_t last_used_slot;
    static uint16_t slots_hwm;

#ifdef DEBUG_FS_ENABLE
    static size_t allocs;
    static size_t frees;
#endif
#endif
};

template<typename ConfigT>
size_t nextSlot();

extern template size_t nextSlot<Config::ConfUint>();
extern template size_t nextSlot<Config::ConfInt>();
extern template size_t nextSlot<Config::ConfFloat>();
extern template size_t nextSlot<Config::ConfString>();
extern template size_t nextSlot<Config::ConfArray>();
extern template size_t nextSlot<Config::ConfObject>();
extern template size_t nextSlot<Config::ConfUnion>();
extern template size_t nextSlot<Config::ConfInt52>();
extern template size_t nextSlot<Config::ConfUint53>();

#if MODULE_DEBUG_AVAILABLE()
template<typename ConfigT>
size_t find_last_used_slot(Superblock<ConfigT> *superblock, size_t last_slot_to_check)
{
    if (last_slot_to_check >= SlotConfig<ConfigT>::slots_per_superblock) {
        size_t last_slot = find_last_used_slot(superblock->next_superblock, last_slot_to_check - SlotConfig<ConfigT>::slots_per_superblock);

        if (last_slot < std::numeric_limits<size_t>::max()) {
            return last_slot + SlotConfig<ConfigT>::slots_per_superblock;
        }
    }

    size_t last_block_idx = last_slot_to_check / SlotConfig<ConfigT>::slots_per_block;
    size_t last_slot_idx  = last_slot_to_check % SlotConfig<ConfigT>::slots_per_block;

    while (true) {
        typename ConfigT::Slot *block = superblock->blocks[last_block_idx];

        while (true) {
            if (!ConfigT::slotEmpty(block + last_slot_idx)) {
                return last_slot_idx + last_block_idx * SlotConfig<ConfigT>::slots_per_block;
            }

            if (last_slot_idx == 0) {
                break;
            }
            last_slot_idx--;
        }

        if (last_block_idx == 0) {
            break;
        }
        last_block_idx--;
        last_slot_idx = SlotConfig<ConfigT>::slots_per_block - 1;
    }

    return std::numeric_limits<size_t>::max();
}
#endif

template<typename ConfigT>
inline void notify_free_slot(size_t idx)
{
    ASSERT_MAIN_THREAD();

    if (idx < RootBlock<ConfigT>::first_free_slot) {
        RootBlock<ConfigT>::first_free_slot = static_cast<uint16_t>(idx);
    }

#if MODULE_DEBUG_AVAILABLE()
    RootBlock<ConfigT>::used_slots--;
#ifdef DEBUG_FS_ENABLE
    RootBlock<ConfigT>::frees++;
#endif

    if (idx == RootBlock<ConfigT>::last_used_slot && RootBlock<ConfigT>::last_used_slot > 0) {
        RootBlock<ConfigT>::last_used_slot = find_last_used_slot(RootBlock<ConfigT>::first_superblock, RootBlock<ConfigT>::last_used_slot - 1u);
    }
#endif
}

template<typename ConfigT>
typename ConfigT::Slot *get_slot(size_t idx);

extern template Config::ConfUint::Slot   *get_slot<Config::ConfUint>(size_t idx);
extern template Config::ConfInt::Slot    *get_slot<Config::ConfInt>(size_t idx);
extern template Config::ConfFloat::Slot  *get_slot<Config::ConfFloat>(size_t idx);
extern template Config::ConfString::Slot *get_slot<Config::ConfString>(size_t idx);
extern template Config::ConfArray::Slot  *get_slot<Config::ConfArray>(size_t idx);
extern template Config::ConfObject::Slot *get_slot<Config::ConfObject>(size_t idx);
extern template Config::ConfUnion::Slot  *get_slot<Config::ConfUnion>(size_t idx);
extern template Config::ConfUint53::Slot   *get_slot<Config::ConfUint53>(size_t idx);
extern template Config::ConfInt52::Slot    *get_slot<Config::ConfInt52>(size_t idx);

template<typename ConfigT>
size_t get_allocated_slot_memory()
{
    return RootBlock<ConfigT>::allocated_blocks * SlotConfig<ConfigT>::slots_per_block * sizeof(typename ConfigT::Slot);
}

#if MODULE_DEBUG_AVAILABLE()
struct SlotDebugInfo {
    uint16_t first_free_slot;
    uint16_t used_slots;
    uint16_t last_used_slot;
    uint16_t slots_hwm;
    uint16_t allocated_slots;
};

template<typename ConfigT>
void get_slot_debug_info(SlotDebugInfo *slot_info)
{
    slot_info->first_free_slot = RootBlock<ConfigT>::first_free_slot;
    slot_info->used_slots      = RootBlock<ConfigT>::used_slots;
    slot_info->last_used_slot  = RootBlock<ConfigT>::last_used_slot;
    slot_info->slots_hwm       = RootBlock<ConfigT>::slots_hwm;
    slot_info->allocated_slots = RootBlock<ConfigT>::allocated_blocks * SlotConfig<ConfigT>::slots_per_block;
}

#endif

#include "main_available_end.h"
