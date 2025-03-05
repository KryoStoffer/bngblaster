/*
 * BNG Blaster (BBL) - BGP Definitions
 *
 * Christian Giese, MARCH 2022
 *
 * Copyright (C) 2020-2025, RtBrick, Inc.
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef __BBL_BGP_DEF_H__
#define __BBL_BGP_DEF_H__

/* DEFINITIONS ... */

#define BGP_PORT                    179
#define BGP_MIN_MESSAGE_SIZE        19U
#define BGP_MAX_MESSAGE_SIZE        4096U
#define BGP_BUF_SIZE                256*1024
#define BGP_DEFAULT_AS              65000
#define BGP_DEFAULT_HOLD_TIME       90
#define BGP_DEFAULT_TEARDOWN_TIME   5

#define BGP_MSG_OPEN                1
#define BGP_MSG_UPDATE              2
#define BGP_MSG_NOTIFICATION        3
#define BGP_MSG_KEEPALIVE           4

#define BGP_CAPABILITY              2
#define BGP_CAPABILITY_4_BYTE_AS    65

#define BGP_IPV4_UC                 0x00000001
#define BGP_IPv6_UC                 0x00000002
#define BGP_IPv4_MC                 0x00000004
#define BGP_IPv6_MC                 0x00000008
#define BGP_IPv4_LU                 0x00000010
#define BGP_IPv6_LU                 0x00000020
#define BGP_IPv4_VPN_UC             0x00000040
#define BGP_IPv6_VPN_UC             0x00000080
#define BGP_IPv4_VPN_MC             0x00000100
#define BGP_IPv6_VPN_MC             0x00000200
#define BGP_IPv4_FLOW               0x00000400
#define BGP_IPv6_FLOW               0x00000800
#define BGP_EVPN                    0x00001000

typedef enum bgp_state_ {
    BGP_CLOSED,
    BGP_IDLE,
    BGP_CONNECT,
    BGP_ACTIVE,
    BGP_OPENSENT,
    BGP_OPENCONFIRM,
    BGP_ESTABLISHED,
    BGP_CLOSING,
} bgp_state_t;

/*
 * BGP RAW Update File
 */
typedef struct bgp_raw_update_ {
    const char *file;

    uint8_t *buf;
    uint32_t len;
    uint32_t updates;

    /* Pointer to next instance */
    struct bgp_raw_update_ *next;
} bgp_raw_update_s;

/*
 * BGP Configuration
 */
typedef struct bgp_config_ {
    uint8_t  af; /* address family */
    uint32_t ipv4_local_address;
    uint32_t ipv4_peer_address;

    ipv6addr_t ipv6_local_address;
    ipv6addr_t ipv6_peer_address;

    uint32_t id;
    uint32_t local_as;
    uint32_t peer_as;
    uint16_t hold_time;
    uint16_t teardown_time;
    uint8_t  tos; /* IPv4 TOS or IPv6 TC */
    uint8_t  ttl;

    uint32_t family;
    uint32_t extended_nexthop;

    bool reconnect;
    bool start_traffic;

    char *network_interface;
    char *raw_update_file;

    /* Pointer to next instance */
    struct bgp_config_ *next;
} bgp_config_s;

/*
 * BGP Session
 */
typedef struct bgp_session_ {
    uint8_t  af; /* address family */
    uint32_t ipv4_local_address;
    uint32_t ipv4_peer_address;

    ipv6addr_t *ipv6_local_address;
    ipv6addr_t *ipv6_peer_address;

    char *local_address_str;
    char *peer_address_str;

    bgp_config_s *config;
    bbl_network_interface_s *interface;
    bbl_tcp_ctx_s *tcpc;
    bbl_tcp_ctx_s *listen_tcpc;

    struct timer_ *connect_timer;
    struct timer_ *keepalive_timer;
    struct timer_ *hold_timer;
    struct timer_ *close_timer;

    struct timer_ *update_timer;
    struct timer_ *teardown_timer;

    io_buffer_t read_buf;
    io_buffer_t write_buf;

    bgp_state_t state;

    struct {
        uint32_t as;
        uint32_t id;
        uint16_t hold_time;
    } peer;

    struct {
        uint32_t message_rx;
        uint32_t message_tx;
        uint32_t keepalive_rx;
        uint32_t keepalive_tx;
        uint32_t update_rx;
        uint32_t update_tx;
    } stats;

    bgp_raw_update_s *raw_update_start;
    bgp_raw_update_s *raw_update;
    bool raw_update_sending;

    struct timespec established_timestamp;
    struct timespec update_start_timestamp;
    struct timespec update_stop_timestamp;
    struct timespec update_duration;

    bool teardown;
    uint8_t error_code;
    uint8_t error_subcode;
    
    struct bgp_session_ *next; /* pointer to next instance */
} bgp_session_s;

#endif