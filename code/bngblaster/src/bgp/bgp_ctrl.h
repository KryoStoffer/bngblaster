/*
 * BNG Blaster (BBL) - BGP CTRL (Control Commands)
 *
 * Christian Giese, March 2022
 *
 * Copyright (C) 2020-2025, RtBrick, Inc.
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef __BBL_BGP_CTRL_H__
#define __BBL_BGP_CTRL_H__

int
bgp_ctrl_sessions(int fd, uint32_t session_id __attribute__((unused)), json_t *arguments __attribute__((unused)));

int
bgp_ctrl_teardown(int fd, uint32_t session_id __attribute__((unused)), json_t *arguments __attribute__((unused)));

int
bgp_ctrl_raw_update(int fd, uint32_t session_id __attribute__((unused)), json_t *arguments);

int
bgp_ctrl_raw_update_list(int fd, uint32_t session_id __attribute__((unused)), json_t *arguments __attribute__((unused)));

int
bgp_ctrl_disconnect(int fd, uint32_t session_id __attribute__((unused)), json_t *arguments);

#endif