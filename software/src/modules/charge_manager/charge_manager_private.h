#include <stdint.h>
#include <memory>

#define DISTRIBUTION_LOG_LEN 2048

#define CHARGE_MANAGER_ERROR_CHARGER_UNREACHABLE 128
#define CHARGE_MANAGER_ERROR_EVSE_UNREACHABLE 129
#define CHARGE_MANAGER_ERROR_EVSE_NONREACTIVE 130
#define CHARGE_MANAGER_CLIENT_ERROR_START 192

// Initialized by charge_manager.cpp; never changed
struct CurrentAllocatorConfig {
    uint32_t minimum_current_3p;
    uint32_t minimum_current_1p;
    std::unique_ptr<char[]> distribution_log;
    size_t distribution_log_len;
    size_t charger_count;
    uint16_t requested_current_margin;
};

// R+W _only_ by current_allocator.cpp
struct CurrentAllocatorState {
    bool last_print_local_log_was_error = false;
};
