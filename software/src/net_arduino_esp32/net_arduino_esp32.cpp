/*
 * Copyright (C) 2020 Erik Fleckstein <erik@tinkerforge.com>
 *
 * Redistribution and use in source and binary forms of this file,
 * with or without modification, are permitted. See the Creative
 * Commons Zero (CC0 1.0) License for more details.
 */

#include "net_arduino_esp32.h"

#include <Arduino.h>
#include <lwip/err.h>
#include <lwip/sockets.h>
#include <lwip/sys.h>
#include <lwip/netdb.h>
#include <string.h>
#include <esp_wifi.h>
#include <esp_random.h>

#include "../bindings/macros.h"
#include "../bindings/hal_common.h"
#include "../bindings/errors.h"
#include "hmac.h"

static void remove_open_request(TF_Net *net, size_t idx) {
    memmove(net->open_requests + idx, net->open_requests + idx + 1, sizeof(TF_Request) * (net->open_request_count - idx - 1));
    --net->open_request_count;
}

static void add_open_request(TF_Net *net, uint32_t client_id, TF_TFPHeader *header) {
    if (net->open_request_count == sizeof(net->open_requests) / sizeof(net->open_requests[0])) {
        // The open_request array is full. Drop the oldest request.
        // Note that this does not mean, that a later response to the dropped request will be
        // dropped too: As only the mapping to where the request came from is missing, the response
        // will be broadcast to all connected clients.
        remove_open_request(net, 0);
    }

    net->open_requests[net->open_request_count].client_id = client_id;
    net->open_requests[net->open_request_count].uid_num = header->uid_num;
    net->open_requests[net->open_request_count].fid = header->fid;
    net->open_requests[net->open_request_count].seq_num = header->seq_num;

    ++net->open_request_count;
}

static int set_tcp_options(int fd, const char **op) {
    int one = 1;

    *op = "enable TCP_NODELAY";

    if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one)) < 0) {
        return -1;
    }

    *op = "enable O_NONBLOCK";
    int flags = fcntl(fd, F_GETFL, 0);

    if (flags < 0 || fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) {
        return -1;
    }

    return 0;
}

static int build_server_socket(const sockaddr_in *addr) {
    const char *op = "";
    int one = 1;

    op = "open socket";
    int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (fd < 0) {
        goto error;
    }

    if (set_tcp_options(fd, &op) < 0) {
        goto close_and_error;
    }

    op = "enable SO_REUSEADDR";

    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) < 0) {
        goto close_and_error;
    }

    op = "bind socket";

    if (bind(fd, (const sockaddr *)addr, sizeof(sockaddr_in)) < 0) {
        goto close_and_error;
    }

    op = "listen on socket";

    if (listen(fd, 16) < 0) {
        goto close_and_error;
    }

    return fd;

close_and_error:
    close(fd);

error:
    tf_hal_log_error("Failed to %s: %s\n", op, strerror(errno));

    return -1;
}

static int server_accept(int server_fd) {
    struct sockaddr_in client_addr;
    socklen_t client_addr_size = sizeof(client_addr);
    const char *op = "accept";
    int client_fd = accept(server_fd, (struct sockaddr *) &client_addr, &client_addr_size);

    if (client_fd < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
            return -2;
        }

        return -1;
    }

    if (set_tcp_options(client_fd, &op) < 0) {
        close(client_fd);
        tf_hal_log_info("Failed to %s: %s\n", op, strerror(errno));
        return -2;
    }

    return client_fd;
}

static uint32_t tf_net_current_time_us(TF_Net *net) {
    return micros();
}

static uint32_t tf_net_current_time_ms(TF_Net *net) {
    return millis();
}

static void remove_client(TF_Net *net, uint32_t client_idx) {
    TF_NetClient *client = &net->clients[client_idx];

    for (int i = 0; i < net->open_request_count; ++i) {
        if (net->open_requests[i].client_id == client->id) {
            remove_open_request(net, i);
            --i;
        }
    }

    close(client->fd);

    memmove(net->clients + client_idx, net->clients + (client_idx + 1), sizeof(TF_NetClient) * (net->clients_used - client_idx - 1));
    --net->clients_used;
    net->clients[net->clients_used].id = 0;
    net->clients[net->clients_used].fd = 0;
    net->clients[net->clients_used].read_buf_used = 0;
    net->clients[net->clients_used].send_buf_used = 0;
    // TODO clear read_buf?;

    tf_hal_log_info("disconnect; clients used: %d\n", net->clients_used);
}

static int read_packets(TF_Net *net) {
    for (size_t i = 0; i < net->clients_used; ++i) {
        TF_NetClient *client = &net->clients[i];
        uint8_t *buf = client->read_buf;
        uint8_t to_write = sizeof(client->read_buf) / sizeof(client->read_buf[0]) - client->read_buf_used;

        if (to_write == 0) {
            continue;
        }

        ssize_t written = recv(client->fd, buf + client->read_buf_used, to_write, 0);

        if (written == 0) {
            remove_client(net, i);
            --i;

            continue;
        }

        if (written < 0) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                remove_client(net, i);
                --i;
            }

            continue;
        }

        client->read_buf_used += written;
        client->last_recv_ms = tf_net_current_time_ms(net);
    }

    return 0;
}

static int send_from_buf(TF_Net *net, int client_idx) {
    TF_NetClient *client = &net->clients[client_idx];
    int written = send(client->fd, client->send_buf, client->send_buf_used, 0);

    if (written < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return 0;
        }

        return -1;
    }

    client->last_send_us = tf_net_current_time_us(net);

    size_t send_buf_size = sizeof(client->send_buf) / sizeof(client->send_buf[0]);

    memmove(client->send_buf, client->send_buf + written, send_buf_size - written * sizeof(client->send_buf[0]));
    client->send_buf_used -= written;

    return written;
}

static bool deadline_elapsed(uint32_t now, uint32_t deadline_us) {
    return ((uint32_t)(now - deadline_us)) < (UINT32_MAX / 2);
}

static int add_to_send_buf(TF_Net *net, int client_idx, uint8_t *buf, uint8_t len) {
    TF_NetClient *client = &net->clients[client_idx];
    size_t send_buf_size = sizeof(client->send_buf) / sizeof(client->send_buf[0]);
    uint32_t deadline_us = tf_net_current_time_us(net) + 10000;

    // Packet will not fit into send buf. Try to make room by sending older packets.
    while (len > send_buf_size - client->send_buf_used) {
        if (deadline_elapsed(tf_net_current_time_us(net), deadline_us)) {
            ++client->sends_without_progress;

            if (client->sends_without_progress < 10) {
                return 0;
            }

            return -1;
        }

        int err = send_from_buf(net, client_idx);

        if (err < 0) {
            return err;
        }
    }

    client->sends_without_progress = 0;

    memcpy(client->send_buf + client->send_buf_used, buf, len);
    client->send_buf_used += len;

    return len;
}

static void flush_send_buffers(TF_Net *net) {
    for (size_t i = 0; i < net->clients_used; ++i) {
        TF_NetClient *client = &net->clients[i];

        if (client->send_buf_used == 0) {
            continue;
        }

        if (deadline_elapsed(tf_net_current_time_us(net), client->last_send_us + net->send_buf_timeout_us) || client->send_buf_used > 1300) {
            if (send_from_buf(net, i) < 0) {
                remove_client(net, i);
                --i;
            }
        }
    }
}

static int accept_connections(TF_Net *net) {
    TF_NetClient *clients = net->clients;
    size_t max_clients = sizeof(net->clients) / sizeof(net->clients[0]);

    if (net->server_fd == -1) {
        if (net->clients_used != max_clients) {
            net->server_fd = build_server_socket(&net->listen_addr);
        }

        return 0;
    }

    static uint32_t client_id = 0;
    int client_fd = server_accept(net->server_fd);

    if (client_fd == -1) {
        // TODO: this is fatal as no connections can be accepted anymore. We should close all connections here
        // and then reopen the server socket
        close(net->server_fd);
        return -1;
    } else if (client_fd == -2) {
        return 0;
    }

    size_t insert_idx = net->clients_used;
    clients[insert_idx].fd = client_fd;
    clients[insert_idx].id = ++client_id;
    clients[insert_idx].last_recv_ms = tf_net_current_time_ms(net);
    clients[insert_idx].last_send_us = tf_net_current_time_us(net);

    clients[insert_idx].auth_state = net->auth_secret == NULL ? TF_CLIENT_AUTHENTICATION_STATE_DISABLED : TF_CLIENT_AUTHENTICATION_STATE_ENABLED;
    clients[insert_idx].auth_nonce = net->next_auth_nonce++;

    memset(clients[insert_idx].read_buf, 0, sizeof(clients[insert_idx].read_buf) / sizeof(clients[insert_idx].read_buf[0]));
    clients[insert_idx].read_buf_used = 0;

    memset(clients[insert_idx].send_buf, 0, sizeof(clients[insert_idx].send_buf) / sizeof(clients[insert_idx].send_buf[0]));
    clients[insert_idx].send_buf_used = 0;
    ++net->clients_used;

    printf("clients used: %d\n", net->clients_used);

    if (net->clients_used == max_clients) {
        // We already have the maximum amount of clients, don't accept more.
        close(net->server_fd);
        net->server_fd = -1;
    }

    return 0;
}

/*static void remove_dead_clients(TF_Net *net) {
    for (size_t i = 0; i < net->clients_used; ++i) {
        if (deadline_elapsed(tf_net_current_time_ms(net), net->clients[i].last_recv_ms + net->recv_timeout_ms)) {
            remove_client(net, i);
        }
    }
}*/

static void drop_packet(TF_NetClient *client, uint8_t len) {
    len = MIN(len, client->read_buf_used);
    memmove(client->read_buf, client->read_buf + len, client->read_buf_used - len);
    client->read_buf_used -= len;
    client->available_packet_valid = false;
}

static void broadcast(TF_Net *net, TF_TFPHeader *header, uint8_t *buf) {
    for (size_t i = 0; i < net->clients_used; ++i) {
        TF_NetClient *client = &net->clients[i];

        if (   client->auth_state != TF_CLIENT_AUTHENTICATION_STATE_DISABLED
           && client->auth_state != TF_CLIENT_AUTHENTICATION_STATE_DONE) {
            tf_hal_log_info("Client not authenticated. Dropping response");
            continue;
        }

        if (add_to_send_buf(net, i, buf, header->length) < 0) {
            remove_client(net, i);
            --i;
            continue;
        }
    }
}

static void unicast(TF_Net *net, TF_TFPHeader *header, uint8_t *buf, size_t client_idx) {
    if (add_to_send_buf(net, client_idx, buf, header->length) < 0) {
        remove_client(net, client_idx);
        return;
    }

    // TODO: if send_from_buf can't send anything here we will send again in 20 ms. maybe set last_send to a lower value to force more sends.
    if (send_from_buf(net, client_idx) < 0) {
        remove_client(net, client_idx);
        return;
    }
}

static void handle_get_authentication_nonce_request(TF_Net *net, uint8_t client_id, TF_TFPHeader *header, uint8_t *buf) {
    TF_NetClient *client = &net->clients[client_id];

    if (client->auth_state == TF_CLIENT_AUTHENTICATION_STATE_DISABLED) {
        tf_hal_log_info("Client tries to authenticate, but authentication is disabled. Disconnecting client.\n");
        remove_client(net, client_id);
        return;
    }

    if (client->auth_state == TF_CLIENT_AUTHENTICATION_STATE_DONE) {
        tf_hal_log_info("Already authenticated client tries to authenticate again.");
        client->auth_state = TF_CLIENT_AUTHENTICATION_STATE_ENABLED;
    }

    if (client->auth_state != TF_CLIENT_AUTHENTICATION_STATE_ENABLED) {
        tf_hal_log_info("Client performed invalid authentication sequence (%d -> %d). Disconnecting client.\n", client->auth_state, TF_CLIENT_AUTHENTICATION_STATE_NONCE_SENT);
        remove_client(net, client_id);
        return;
    }

    client->auth_state = TF_CLIENT_AUTHENTICATION_STATE_NONCE_SENT;

    if (header->response_expected) {
        header->length = TF_GET_AUTHENTICATION_NONCE_RESPONSE_LEN;
        tf_tfp_header_write(header, buf);
        memcpy(buf + TF_TFP_HEADER_LENGTH, &client->auth_nonce, sizeof(client->auth_nonce));
        unicast(net, header, buf, client_id);
    }
}

static void handle_authenticate_request(TF_Net *net, uint8_t client_id, TF_TFPHeader *header, uint8_t *buf) {
    TF_NetClient *client = &net->clients[client_id];

    if (client->auth_state == TF_CLIENT_AUTHENTICATION_STATE_DISABLED) {
        tf_hal_log_info("Client tries to authenticate, but authentication is disabled. Disconnecting client.\n");
        remove_client(net, client_id);
        return;
    }

    if (client->auth_state != TF_CLIENT_AUTHENTICATION_STATE_NONCE_SENT) {
        tf_hal_log_info("Client performed invalid authentication sequence (%d -> %d). Disconnecting client.\n", client->auth_state, TF_CLIENT_AUTHENTICATION_STATE_NONCE_SENT);
        remove_client(net, client_id);
        return;
    }

    uint8_t *buf_read_ptr = buf;
    buf_read_ptr += TF_TFP_HEADER_LENGTH;

    uint32_t nonces[2];
    uint8_t digest[TF_SHA1_DIGEST_LENGTH];
    memcpy(&nonces[0], &client->auth_nonce, sizeof(uint32_t));
    memcpy(&nonces[1], buf_read_ptr, sizeof(uint32_t));
    buf_read_ptr += sizeof(uint32_t);

    tf_hmac_sha1((const uint8_t *)net->auth_secret, strlen(net->auth_secret),
                 (const uint8_t *)nonces, sizeof(nonces), digest);

    if (memcmp(buf_read_ptr, digest, TF_SHA1_DIGEST_LENGTH) != 0) {
        tf_hal_log_info("Authenticate request from client did not contain the expected data. Disconnecting client\n");
        remove_client(net, client_id);
        return;
    }

    tf_hal_log_info("Client authenticated.\n");
    client->auth_state = TF_CLIENT_AUTHENTICATION_STATE_DONE;

    if (header->response_expected) {
        header->length = TF_AUTHENTICATE_RESPONSE_LEN;
        tf_tfp_header_write(header, buf);
        unicast(net, header, buf, client_id);
    }
}

static void handle_brickd_packet(TF_Net *net, uint8_t packet_id, TF_TFPHeader *header, uint8_t *buf) {
    if (header->fid == TF_FUNCTION_GET_AUTHENTICATION_NONCE) {
        if (header->length != TF_GET_AUTHENTICATION_NONCE_REQUEST_LEN) {
            tf_hal_log_info("Received authentication-nonce request with wrong length. Disconnecting client.\n");
            remove_client(net, packet_id);
            return;
        }

        handle_get_authentication_nonce_request(net, packet_id, header, buf);
        return;
    }

    if (header->fid == TF_FUNCTION_AUTHENTICATE) {
        if (header->length != TF_AUTHENTICATE_REQUEST_LEN) {
            tf_hal_log_info("Received authenticate request with wrong length. Disconnecting client.\n");
            remove_client(net, packet_id);
            return;
        }

        handle_authenticate_request(net, packet_id, header, buf);
        return;
    }

    header->error_code = 2;
    tf_tfp_header_write(header, buf);
    unicast(net, header, buf, packet_id);
}

static void reassemble_packets(TF_Net *net) {
    for (int i = 0; i < net->clients_used; ++i) {
        TF_NetClient *client = &net->clients[i];

        if (client->available_packet_valid) {
            continue;
        }

        uint8_t used = client->read_buf_used;

        if (used < 8) {
            continue;
        }

        uint8_t *buf = client->read_buf;
        TF_TFPHeader header;

        if (!tf_tfp_header_peek_plain(&header, buf) || header.fid == 0) {
            tf_hal_log_debug("invalid request, closing connection\n");
            remove_client(net, i);
            continue;
        }

        if (used < header.length) {
            continue;
        }

        // Handle Brick Daemon packets without copy
        if (header.uid_num == 1) {
            uint8_t len = header.length;
            handle_brickd_packet(net, i, &header, buf);
            drop_packet(client, len);
            continue;
        } else if (   client->auth_state != TF_CLIENT_AUTHENTICATION_STATE_DISABLED
                   && client->auth_state != TF_CLIENT_AUTHENTICATION_STATE_DONE) {
            tf_hal_log_info("Client not authenticated. Dropping request\n");
            drop_packet(client, header.length);
            continue;
        }

        client->available_packet = header;
        client->available_packet_valid = true;
    }
}

int tf_net_create(TF_Net *net, const char* listen_addr, uint16_t port, const char* auth_secret) {
    memset(net, 0, sizeof(TF_Net));
    net->send_buf_timeout_us = 20000;
    net->recv_timeout_ms = 60000;
    net->auth_secret = auth_secret;

    net->listen_addr.sin_family = AF_INET;
    net->listen_addr.sin_port = port == 0 ? htons(4223) : htons(port);

    if (listen_addr == nullptr) {
        net->listen_addr.sin_addr.s_addr = INADDR_ANY;
    } else if(ipaddr_aton(listen_addr, (ip_addr_t *)&net->listen_addr.sin_addr) == 0) {
        return TF_E_INVALID_ADDRESS;
    }

    net->server_fd = build_server_socket(&net->listen_addr);

    if (auth_secret == nullptr)
        return TF_E_OK;

    wifi_mode_t _wifi_mode;
    if (esp_wifi_get_mode(&_wifi_mode) == ESP_ERR_WIFI_NOT_INIT) {
        tf_hal_log_error("tf_net_create: Authentication requested but wifi is not initialized. Server nonce could be pseudo-random!");
    }

    net->next_auth_nonce = esp_random();

    return TF_E_OK;
}

int tf_net_destroy(TF_Net *net) {
    if (net->server_fd >= 0) {
        close(net->server_fd);
    }

    for (int i = 0; i < net->clients_used; ++i) {
        remove_client(net, i);
    }

    return 0;
}

int tf_net_tick(TF_Net *net) {
    flush_send_buffers(net);
    read_packets(net);
    reassemble_packets(net);
    accept_connections(net);
    // TODO: reenable this when we have a useful heartbeat
    //remove_dead_clients(net);

    return 0;
}

bool tf_net_get_available_tfp_header(TF_Net *net, TF_TFPHeader *header, int *packet_id) {
    for (int i = (*packet_id) + 1; i < net->clients_used; ++i) {
        TF_NetClient *client = &net->clients[i];

        if (!client->available_packet_valid) {
            continue;
        }

        *header = client->available_packet;
        *packet_id = i;

        return true;
    }

    return false;
}

int tf_net_get_packet(TF_Net *net, uint8_t packet_id, uint8_t *buf) {
    TF_NetClient *client = &net->clients[packet_id];

    if (!client->available_packet_valid) {
        return -1;
    }

    memcpy(buf, client->read_buf, client->available_packet.length);

    // If we output a packet that is a request, keep track over the request to be able to unicast the response.
    if (client->available_packet.uid_num != 0 && client->available_packet.response_expected) {
        add_open_request(net, client->id, &client->available_packet);
    }

    return 0;
}

int tf_net_drop_packet(TF_Net *net, uint8_t packet_id) {
    TF_NetClient *client = &net->clients[packet_id];

    if (!client->available_packet_valid) {
        return -1;
    }

    drop_packet(client, client->available_packet.length);

    return 0;
}

void tf_net_send_packet(TF_Net *net, TF_TFPHeader *header, uint8_t *buf) {
    if (header->seq_num == 0) {
        broadcast(net, header, buf);
    } else {
        TF_Request *request = NULL;
        size_t request_idx = 0;

        for (; request_idx < net->open_request_count; ++request_idx) {
            TF_Request *req = &net->open_requests[request_idx];

            if (req->fid == header->fid && req->seq_num == header->seq_num && req->uid_num == header->uid_num) {
                request = req;
                break;
            }
        }

        if (request == NULL) {
            // We don't know where the request came from anymore.
            broadcast(net, header, buf);
            return;
        }

        uint32_t client_id = request->client_id;
        remove_open_request(net, request_idx);

        TF_NetClient *client = NULL;
        size_t client_idx = 0;

        for (; client_idx < net->clients_used; ++client_idx) {
            if (net->clients[client_idx].id == client_id) {
                client = &net->clients[client_idx];
                break;
            }
        }

        if (client == NULL) {
            // The client for this response is gone. Drop the response.
            return;
        }

        if (client->auth_state != TF_CLIENT_AUTHENTICATION_STATE_DISABLED &&
            client->auth_state != TF_CLIENT_AUTHENTICATION_STATE_DONE) {
            tf_hal_log_info("Client not authenticated. Dropping response");
            return;
        }

        unicast(net, header, buf, client_idx);
    }
}
