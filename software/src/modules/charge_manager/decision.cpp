#include "charge_manager_private.h"

ChargerDecision ChargerDecision::None0() { return ChargerDecision{._empty = 0, .tag = AllocatorDecision::None0}; }
ChargerDecision ChargerDecision::WaitingForRotation0() { return ChargerDecision{._empty = 0, .tag = AllocatorDecision::WaitingForRotation0}; }
ChargerDecision ChargerDecision::ShuttingDownUnknown0() { return ChargerDecision{._empty = 0, .tag = AllocatorDecision::ShuttingDownUnknown0}; }
ChargerDecision ChargerDecision::ShuttingDownNotActive0() { return ChargerDecision{._empty = 0, .tag = AllocatorDecision::ShuttingDownNotActive0}; }
ChargerDecision ChargerDecision::ShuttingDownRotatedForB10() { return ChargerDecision{._empty = 0, .tag = AllocatorDecision::ShuttingDownRotatedForB10}; }
ChargerDecision ChargerDecision::ShuttingDownRotatedForHigherPrio0() { return ChargerDecision{._empty = 0, .tag = AllocatorDecision::ShuttingDownRotatedForHigherPrio0}; }
ChargerDecision ChargerDecision::ShuttingDownOffOrError0() { return ChargerDecision{._empty = 0, .tag = AllocatorDecision::ShuttingDownOffOrError0}; }
ChargerDecision ChargerDecision::WelcomeChargeUntil2(micros_t timestamp) { return ChargerDecision{.welcome_charge_until = {.timestamp = timestamp}, .tag = AllocatorDecision::WelcomeChargeUntil2}; }
ChargerDecision ChargerDecision::ShuttingDownPhaseOverload2(uint32_t overload_ma, uint8_t phase) { return ChargerDecision{.shutting_down_phase_overload = {.overload_ma = overload_ma, .phase = phase }, .tag = AllocatorDecision::ShuttingDownPhaseOverload2}; }
ChargerDecision ChargerDecision::CantActivatePhaseMinimum3(uint32_t required_ma, uint32_t min_ma, uint8_t phase) { return ChargerDecision{.cant_activate_phase_minimum = {.required_ma = required_ma, .min_ma = min_ma, .phase = phase}, .tag = AllocatorDecision::CantActivatePhaseMinimum3}; }
ChargerDecision ChargerDecision::Activating1(uint8_t phase_alloc) { return ChargerDecision{.activating = {.phase_alloc = phase_alloc}, .tag = AllocatorDecision::Activating1}; }
ChargerDecision ChargerDecision::PhaseSwitching0() { return ChargerDecision{._empty = 0, .tag = AllocatorDecision::PhaseSwitching0}; }
ChargerDecision ChargerDecision::PhaseSwitchingBlockedUntil2(micros_t timestamp) { return ChargerDecision{.phase_switching_blocked_until = {.timestamp = timestamp}, .tag = AllocatorDecision::PhaseSwitchingBlockedUntil2}; }
ChargerDecision ChargerDecision::WakingUp0() { return ChargerDecision{._empty = 0, .tag = AllocatorDecision::WakingUp0}; }

const ConfUnionPrototype<AllocatorDecision> *ChargerDecision::getUnionPrototypes() {
    static const ConfUnionPrototype<AllocatorDecision> alloc_desc_union[ALLOCATOR_DECISION_COUNT] = {
        {AllocatorDecision::None0, *Config::Null()},
        {AllocatorDecision::WaitingForRotation0, *Config::Null()},
        {AllocatorDecision::ShuttingDownUnknown0, *Config::Null()},
        {AllocatorDecision::ShuttingDownNotActive0, *Config::Null()},
        {AllocatorDecision::ShuttingDownRotatedForB10, *Config::Null()},
        {AllocatorDecision::ShuttingDownRotatedForHigherPrio0, *Config::Null()},
        {AllocatorDecision::ShuttingDownOffOrError0, *Config::Null()},
        {AllocatorDecision::WelcomeChargeUntil2, Config::Int52(0)}, // timestamp
        {AllocatorDecision::ShuttingDownPhaseOverload2, Config::Array({Config::Uint32(0), Config::Uint32(0)}, Config::get_prototype_uint32_0(), 2, 2, Config::type_id<Config::ConfUint>())}, // phase, overload_mA
        {AllocatorDecision::CantActivatePhaseMinimum3, Config::Array({Config::Uint32(0), Config::Uint32(0), Config::Uint32(0)}, Config::get_prototype_uint32_0(), 3, 3, Config::type_id<Config::ConfUint>())}, //phase, required_mA, min_mA
        {AllocatorDecision::Activating1, Config::Uint8(0)}, // phase_alloc; // TODO add limit. requires config changes currently not merged to master
        {AllocatorDecision::PhaseSwitching0, *Config::Null()},
        {AllocatorDecision::PhaseSwitchingBlockedUntil2, Config::Int52(0)}, // timestamp
        {AllocatorDecision::WakingUp0, *Config::Null()},
    };
    return alloc_desc_union;
}

size_t ChargerDecision::getUnionPrototypeCount() { return ALLOCATOR_DECISION_COUNT; }

void ChargerDecision::writeToConfig(Config *target) {
    target->changeUnionVariant(this->tag);

    switch (this->tag) {
        case AllocatorDecision::WelcomeChargeUntil2:
            target->get()->updateInt52(this->welcome_charge_until.timestamp.to<seconds_t>().as<int64_t>());
            break;
        case AllocatorDecision::ShuttingDownPhaseOverload2:
            target->get()->get(0)->updateUint(this->shutting_down_phase_overload.phase);
            target->get()->get(1)->updateUint(this->shutting_down_phase_overload.overload_ma);
            break;
        case AllocatorDecision::CantActivatePhaseMinimum3:
            target->get()->get(0)->updateUint(this->cant_activate_phase_minimum.phase);
            target->get()->get(1)->updateUint(this->cant_activate_phase_minimum.required_ma);
            target->get()->get(2)->updateUint(this->cant_activate_phase_minimum.min_ma);
            break;
        case AllocatorDecision::Activating1:
            target->get()->updateUint(this->activating.phase_alloc);
            break;
        case AllocatorDecision::PhaseSwitchingBlockedUntil2:
            target->get()->updateInt52(this->phase_switching_blocked_until.timestamp.to<seconds_t>().as<int64_t>());
            break;

        case AllocatorDecision::None0:
        case AllocatorDecision::WaitingForRotation0:
        case AllocatorDecision::ShuttingDownUnknown0:
        case AllocatorDecision::ShuttingDownNotActive0:
        case AllocatorDecision::ShuttingDownRotatedForB10:
        case AllocatorDecision::ShuttingDownRotatedForHigherPrio0:
        case AllocatorDecision::ShuttingDownOffOrError0:
        case AllocatorDecision::PhaseSwitching0:
        case AllocatorDecision::WakingUp0:
            break;
    }
}


GlobalDecision GlobalDecision::None0() { return GlobalDecision{._empty = 0, .tag = GlobalAllocatorDecision::None0}; }
GlobalDecision GlobalDecision::NextRotationAt2(micros_t timestamp) { return GlobalDecision{.next_rotation_at = {.timestamp = timestamp}, .tag = GlobalAllocatorDecision::NextRotationAt2}; }
GlobalDecision GlobalDecision::PVExcessOverloadedHysteresisBlocksUntil3(uint32_t overload_ma, micros_t timestamp) { return GlobalDecision{.pv_excess_overloaded_hysteresis_blocks_until = {.timestamp = timestamp, .overload_ma = overload_ma}, .tag = GlobalAllocatorDecision::PVExcessOverloadedHysteresisBlocksUntil3}; }
GlobalDecision GlobalDecision::HysteresisElapsesAt2(micros_t timestamp) { return GlobalDecision{.hysteresis_elapses_at = {.timestamp = timestamp}, .tag = GlobalAllocatorDecision::HysteresisElapsesAt2}; }

const ConfUnionPrototype<GlobalAllocatorDecision> *GlobalDecision::getUnionPrototypes() {
    static const ConfUnionPrototype<GlobalAllocatorDecision> global_alloc_desc_union[GLOBAL_ALLOCATOR_DECISION_COUNT] = {
        {GlobalAllocatorDecision::None0, *Config::Null()},
        {GlobalAllocatorDecision::NextRotationAt2, Config::Int52(0)}, // timestamp
        {GlobalAllocatorDecision::PVExcessOverloadedHysteresisBlocksUntil3, Config::Array({Config::Int52(0), Config::Int52(0)}, Config::get_prototype_int52_0(), 2, 2, Config::type_id<Config::ConfInt52>())}, // overload_ma, timestamp
        {GlobalAllocatorDecision::HysteresisElapsesAt2, Config::Int52(0)}, // timestamp
    };
    return global_alloc_desc_union;
}

size_t GlobalDecision::getUnionPrototypeCount() { return GLOBAL_ALLOCATOR_DECISION_COUNT; }

void GlobalDecision::writeToConfig(Config *target){
    target->changeUnionVariant(this->tag);

    switch (this->tag) {
        case GlobalAllocatorDecision::NextRotationAt2:
            target->get()->updateInt52(this->next_rotation_at.timestamp.to<seconds_t>().as<int64_t>());
            break;
        case GlobalAllocatorDecision::PVExcessOverloadedHysteresisBlocksUntil3:
            target->get()->get(0)->updateInt52(this->pv_excess_overloaded_hysteresis_blocks_until.overload_ma);
            target->get()->get(1)->updateInt52(this->pv_excess_overloaded_hysteresis_blocks_until.timestamp.to<seconds_t>().as<int64_t>());
            break;
        case GlobalAllocatorDecision::HysteresisElapsesAt2:
            target->get()->updateInt52(this->hysteresis_elapses_at.timestamp.to<seconds_t>().as<int64_t>());
            break;

        case GlobalAllocatorDecision::None0:
            break;
    }
}
