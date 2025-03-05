/*
 * BNG Blaster (BBL) - IO RX Threads
 *
 * Christian Giese, July 2022
 *
 * Copyright (C) 2020-2025, RtBrick, Inc.
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef __BBL_IO_THREAD_H__
#define __BBL_IO_THREAD_H__

bool
io_thread_init(io_handle_s *io);

void
io_thread_start_all();

void
io_thread_stop_all();

io_result_t
io_thread_rx_handler(io_thread_s *thread, io_handle_s *io);

#endif