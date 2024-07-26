#include "current_allocator.h"
#include <memory>

#include "tools.h"
#include "event_log_prefix.h"
#include "modules/cm_networking/cm_networking_defs.h"

#include <string.h>

//#include "tests.h"


#pragma region Copied over from cm_networking/cm_protocol.cpp

#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <errno.h>

static int create_socket(uint16_t port, bool blocking)
{
    int sock;
    struct sockaddr_in dest_addr;

    dest_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port);

    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock < 0) {
        logger.printfln("Unable to create socket: errno %d", errno);
        return -1;
    }

    int err = bind(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err < 0) {
        logger.printfln("Socket unable to bind: errno %d", errno);
        return -1;
    }

    if (blocking) {
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 100 * 1000;
        err = setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
        if (err < 0) {
            logger.printfln("Failed to set SO_RCVTIMEO opion: errno %d", errno);
            return -1;
        }
        return sock;
    }

    int flags = fcntl(sock, F_GETFL, 0);
    if (flags < 0) {
        logger.printfln("Failed to get flags from socket: errno %d", errno);
        return -1;
    }

    err = fcntl(sock, F_SETFL, flags | O_NONBLOCK);
    if (err < 0) {
        logger.printfln("Failed to set O_NONBLOCK flag: errno %d", errno);
        return -1;
    }

    return sock;
}

static bool seq_num_invalid(uint16_t received_sn, uint16_t last_seen_sn)
{
    return received_sn <= last_seen_sn && last_seen_sn - received_sn < 5;
}

#pragma endregion

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc99-designator"
static struct sockaddr_in dest_addrs[] = {
    {
        .sin_family = AF_INET,
        .sin_port = htons(CHARGE_MANAGEMENT_PORT),
        .sin_addr.s_addr = inet_addr("192.168.1.115")
    },
    {
        .sin_family = AF_INET,
        .sin_port = htons(CHARGE_MANAGEMENT_PORT),
        .sin_addr.s_addr = inet_addr("192.168.1.129")
    }
};
#pragma clang diagnostic pop

static int manager_sock;

static const char *get_charger_name(uint8_t idx) {
    return idx == 0 ? "emu1" : "unknown charger";
};

static void start_manager() {
    manager_sock = create_socket(CHARGE_MANAGER_PORT, true);
}


void receive_packets(
    size_t charger_count,
    const char *const *hosts,
    std::function<void(uint8_t /* client_id */, cm_state_v1 *, cm_state_v2 *, cm_state_v3 *)> manager_callback,
    std::function<void(uint8_t, uint8_t)> manager_error_callback
    )
{

    static uint16_t last_seen_seq_num[MAX_CONTROLLED_CHARGERS];
    static bool initialized = false;
    if (!initialized) {
        memset(last_seen_seq_num, 255, sizeof(last_seen_seq_num));
        initialized = true; // FIXME: delayed initialization doesn't show in frontend
    }

    // Try to receive up to four packets in one go to catch up on the backlog.
    // Don't receive every available packet to smooth out bursts of packets.
    static_assert(MAX_CONTROLLED_CHARGERS <= 32);
    for (int poll_ctr = 0; poll_ctr < 4; ++poll_ctr) {
        struct cm_state_packet state_pkt;
        struct sockaddr_in source_addr;
        socklen_t socklen = sizeof(source_addr);

        int len = recvfrom(manager_sock, &state_pkt, sizeof(state_pkt), 0, (sockaddr *)&source_addr, &socklen);

        if (len < 0) {
            if (errno != EAGAIN && errno != EWOULDBLOCK)
                logger.printfln("recvfrom failed: %s", strerror(errno));
            return;
        }

        int charger_idx = -1;
        for(int idx = 0; idx < charger_count; ++idx)
            if (source_addr.sin_family == dest_addrs[idx].sin_family &&
                source_addr.sin_port == dest_addrs[idx].sin_port &&
                source_addr.sin_addr.s_addr == dest_addrs[idx].sin_addr.s_addr) {
                    charger_idx = idx;
                    break;
            }

        // Don't log in the first 20 seconds after startup: We are probably still resolving hostnames.
        if (charger_idx == -1) {
            if (deadline_elapsed(20000))
                logger.printfln("Received packet from unknown %s. Is the config complete?", inet_ntoa(source_addr.sin_addr));
            return;
        }

        /*String validation_error = validate_state_packet_header(&state_pkt, len);
        if (!validation_error.isEmpty()) {
            logger.printfln("Received state packet from %s (%s) (%i bytes) failed validation: %s",
                            charge_manager.get_charger_name(charger_idx),
                            inet_ntoa(source_addr.sin_addr),
                            len,
                            validation_error.c_str());
            manager_error_callback(charger_idx, CM_NETWORKING_ERROR_INVALID_HEADER);
            return;
        }*/

        if (seq_num_invalid(state_pkt.header.seq_num, last_seen_seq_num[charger_idx])) {
            logger.printfln("Received stale (out of order?) state packet from %s (%s). Last seen seq_num is %u, Received seq_num is %u",
                            get_charger_name(charger_idx),
                            inet_ntoa(source_addr.sin_addr),
                            last_seen_seq_num[charger_idx],
                            state_pkt.header.seq_num);
            return;
        }

        last_seen_seq_num[charger_idx] = state_pkt.header.seq_num;

        if (!CM_STATE_FLAGS_MANAGED_IS_SET(state_pkt.v1.state_flags)) {
            manager_error_callback(charger_idx, CM_NETWORKING_ERROR_NOT_MANAGED);
            logger.printfln("%s (%s) reports managed is not activated!",
                get_charger_name(charger_idx),
                inet_ntoa(source_addr.sin_addr));
            return;
        }

        manager_callback(charger_idx, &state_pkt.v1, state_pkt.header.version >= 2 ? &state_pkt.v2 : nullptr, state_pkt.header.version >= 3 ? &state_pkt.v3 : nullptr);
    }
}

bool send_manager_update(uint8_t client_id, uint16_t allocated_current, bool cp_disconnect_requested, int8_t allocated_phases)
{
    static uint16_t next_seq_num = 1;

    if (manager_sock < 0)
        return true;

    struct cm_command_packet command_pkt;
    memset(&command_pkt, 0, sizeof(command_pkt));
    command_pkt.header.magic = CM_PACKET_MAGIC;
    command_pkt.header.length = CM_COMMAND_PACKET_LENGTH;
    command_pkt.header.seq_num = next_seq_num;
    ++next_seq_num;
    command_pkt.header.version = CM_COMMAND_VERSION;

    command_pkt.v1.allocated_current = allocated_current;
    command_pkt.v1.command_flags = cp_disconnect_requested << CM_COMMAND_FLAGS_CPDISC_BIT_POS;

    command_pkt.v2.allocated_phases = allocated_phases;

    int err = sendto(manager_sock, &command_pkt, sizeof(command_pkt), MSG_DONTWAIT, (sockaddr *)&dest_addrs[client_id], sizeof(dest_addrs[client_id]));

    if (err < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            // Intentionally don't increment here, we want to resend to this charger next.
            return false;
        if (errno == ENOMEM) {
            // Ignore ENOMEM for now. Usually indicates that we don't have a network connection yet.
            return true;
        }

        logger.printfln("Failed to send command: %s (%d)", strerror(errno), errno);
        return true;
    }
    if (err != CM_COMMAND_PACKET_LENGTH) {
        logger.printfln("Failed to send command: sendto truncated packet (of %u bytes) to %d bytes.", CM_COMMAND_PACKET_LENGTH, err);
        return true;
    }
    return true;
}


void send_to_clients(size_t charger_count, ChargerAllocationState *charger_allocation_state) {
    static int i = 0;

    if (i >= charger_count)
        i = 0;

    auto &charger_alloc = charger_allocation_state[i];
    if(send_manager_update(i, charger_alloc.allocated_current, charger_alloc.cp_disconnect, charger_alloc.allocated_phases))
        ++i;
}

#define CHARGER_COUNT (sizeof(dest_addrs)/sizeof(dest_addrs[0]))

int main(int argc, char **argv) {
    /*if (argc == 2 && strcmp(argv[1], "--run-tests") == 0) {
        run_tests();
        return 0;
    }*/


    start_manager();

    CurrentAllocatorConfig cfg {
        .allocation_interval = 1000_s,
        .global_hysteresis = 3_m,
        .wakeup_time = 3_m,
        .plug_in_time = 3_m,
        .minimum_active_time = 15_m,
        .allocated_energy_rotation_threshold = 5,

        .minimum_current_3p = 9200,
        .minimum_current_1p = 6000,
        .enable_current_factor = 1.5f,
        .distribution_log = std::unique_ptr<char[]>(new char[DISTRIBUTION_LOG_LEN]()),
        .distribution_log_len = DISTRIBUTION_LOG_LEN,
        .charger_count = CHARGER_COUNT,
        .requested_current_margin = 3000,
        .requested_current_threshold = 60,

    };

    bool seen_all_chargers = true;
    CurrentLimits limits {
        .raw = {96000, 32000, 32000, 32000},
        .min = {96000, 32000, 32000, 32000},
        .spread = {96000, 32000, 32000, 32000},
        .max_pv = 96000,
    };

    bool cp_disconnect_requested = false;

    ChargerState charger_state[CHARGER_COUNT]{};
    charger_state[0].phase_rotation = PhaseRotation::L123;

    const char * const hosts[CHARGER_COUNT] = {
        "warp3-29fk.localdomain",
        "warp2-22oH.localdomain"
    };

    auto clear_dns_cache_entry = [](uint8_t idx){};

    CurrentAllocatorState ca_state;

    ChargerAllocationState charger_allocation_state[CHARGER_COUNT]{};

    uint32_t allocated_current = 0;

    uint32_t last_alloc = millis();
    uint32_t last_send = millis();

    while (true) {
        receive_packets(
            cfg.charger_count,
            hosts,
            [&cfg,
            &charger_state,
            &charger_allocation_state,
            hosts
            ] (uint8_t client_id, cm_state_v1 *v1, cm_state_v2 *v2, cm_state_v3 *v3) mutable {
                update_from_client_packet(
                    client_id,
                    v1,
                    v2,
                    v3,
                    &cfg,
                    charger_state,
                    charger_allocation_state,
                    hosts,
                    get_charger_name);
            },
            [](uint8_t, uint8_t){logger.printfln("packet receive error");});

        if (deadline_elapsed(last_alloc + 5000)) {
            CurrentLimits limits_post_alloc = limits;
            allocate_current(&cfg,
                            seen_all_chargers,
                            &limits_post_alloc,
                            cp_disconnect_requested,
                            charger_state,
                            hosts,
                            get_charger_name,
                            clear_dns_cache_entry,
                            &ca_state,
                            charger_allocation_state,
                            &allocated_current);
            last_alloc = millis();
        }

        if (deadline_elapsed(last_send + 1000 / cfg.charger_count)) {
            send_to_clients(cfg.charger_count, charger_allocation_state);
            last_send = millis();
        }

    }
}
