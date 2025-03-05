/*
 * BNG Blaster (BBL) - IS-IS Adjacency
 * 
 * Christian Giese, February 2022
 *
 * Copyright (C) 2020-2025, RtBrick, Inc.
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "isis.h"

/**
 * isis_adjacency_init
 * 
 * This function inits the IS-IS adjacency. 
 *
 * @param interface network interface
 * @param config network interface configuration
 * @param instance IS-IS instance
 */
bool 
isis_adjacency_init(bbl_network_interface_s *interface,
                    bbl_network_config_s *interface_config,
                    isis_instance_s *instance)
{
    isis_config_s *config = instance->config;
    isis_adjacency_s *adjacency = NULL;
    isis_adjacency_p2p_s *adjacency_p2p = NULL;
    
    uint8_t level;

    LOG(ISIS, "Add network interface %s to IS-IS instance %u\n", 
        interface->name, interface_config->isis_instance_id);

    /* Init P2P adjacency. */
    if(interface_config->isis_p2p) {
        adjacency_p2p = calloc(1, sizeof(isis_adjacency_p2p_s));
        if(!adjacency_p2p) return false;
        interface->isis_adjacency_p2p = adjacency_p2p;
        adjacency_p2p->interface = interface;
        adjacency_p2p->instance  = instance; 
        adjacency_p2p->peer = calloc(1, sizeof(isis_peer_s));
        if(!adjacency_p2p->peer) return false;
        adjacency_p2p->peer->adjacency_p2p = adjacency_p2p;
        adjacency_p2p->level = interface_config->isis_level;
        adjacency_p2p->state = ISIS_P2P_ADJACENCY_STATE_DOWN;
        interface->send_requests |= BBL_IF_SEND_ISIS_P2P_HELLO;
    } else {
        instance->next_pseudo_node_id++;
    }

    /* Init adjacency levels. */
    for(int i=0; i<ISIS_LEVELS; i++) {
        level = i+1;
        if(!(interface_config->isis_level & level)) {
            continue;
        }
        adjacency = calloc(1, sizeof(isis_adjacency_s));
        if(!adjacency) return false;
        interface->isis_adjacency[i] = adjacency;
        adjacency->interface = interface;
        adjacency->instance = instance;
        if(interface_config->isis_p2p) {
            if(!adjacency_p2p) return false;
            adjacency->p2p = true;
            adjacency->peer = adjacency_p2p->peer;
        } else {
            adjacency->pseudo_node_id = instance->next_pseudo_node_id;
        }
        if(instance->level[i].adjacency) {
            adjacency->next = instance->level[i].adjacency;
        }
        instance->level[i].adjacency = adjacency;

        adjacency->flood_tree = hb_tree_new((dict_compare_func)isis_lsp_id_compare);
        adjacency->psnp_tree = hb_tree_new((dict_compare_func)isis_lsp_id_compare);
        adjacency->levels = interface_config->isis_level;
        adjacency->level = level;
        adjacency->window_size = config->lsp_tx_window_size;
        if(level == ISIS_LEVEL_1) {
            adjacency->priority = interface_config->isis_l1_priority;
            adjacency->metric = interface_config->isis_l1_metric;
            if(!interface_config->isis_p2p) interface->send_requests |= BBL_IF_SEND_ISIS_L1_HELLO;
        } else {
            adjacency->priority = interface_config->isis_l2_priority;
            adjacency->metric = interface_config->isis_l2_metric;
            if(!interface_config->isis_p2p) interface->send_requests |= BBL_IF_SEND_ISIS_L2_HELLO;
        }
	if(interface_config->isis_adjacency_sid > 0) {
	    adjacency->adjacency_sid = interface_config->isis_adjacency_sid;
	}
    }
    return true;
}

/**
 * isis_adjacency_up
 *
 * @param adjacency IS-IS adjacency
 */
void 
isis_adjacency_up(isis_adjacency_s *adjacency)
{
    isis_config_s *config = adjacency->instance->config;

    if(adjacency->state == ISIS_ADJACENCY_STATE_UP) {
        return;
    }

    LOG(ISIS, "ISIS %s adjacency UP to %s on interface %s \n", 
        isis_level_string(adjacency->level), 
        isis_system_id_to_str(adjacency->peer->system_id),
        adjacency->interface->name);

    adjacency->state = ISIS_ADJACENCY_STATE_UP;

    timer_add_periodic(&g_ctx->timer_root, &adjacency->timer_csnp, 
        "ISIS CSNP", config->csnp_interval, 0, adjacency, &isis_csnp_job);

    timer_add(&g_ctx->timer_root, &adjacency->timer_csnp_next, 
        "ISIS CSNP", 0, 10 * MSEC, adjacency, &isis_csnp_job);

    if(adjacency->p2p) {
        timer_add_periodic(&g_ctx->timer_root, &adjacency->timer_tx, 
            "ISIS TX", 0, config->lsp_tx_interval * MSEC, adjacency, &isis_lsp_tx_p2p_job);

        timer_add_periodic(&g_ctx->timer_root, &adjacency->timer_retry, 
            "ISIS RETRY", config->lsp_retry_interval, 0, adjacency, &isis_lsp_retry_job);
    } else {
        timer_add_periodic(&g_ctx->timer_root, &adjacency->timer_tx, 
            "ISIS TX", 0, config->lsp_tx_interval * MSEC, adjacency, &isis_lsp_tx_job);
    }

    g_ctx->routing_sessions++;
}

/**
 * isis_adjacency_down
 *
 * @param adjacency IS-IS adjacency
 */
void
isis_adjacency_down(isis_adjacency_s *adjacency, const char *reason)
{
    if(adjacency->state == ISIS_ADJACENCY_STATE_DOWN) {
        return;
    }
    adjacency->state = ISIS_ADJACENCY_STATE_DOWN;

    LOG(ISIS, "ISIS %s adjacency DOWN to %s on interface %s (%s)\n", 
        isis_level_string(adjacency->level), 
        isis_system_id_to_str(adjacency->peer->system_id),
        adjacency->interface->name, reason);

    timer_del(adjacency->timer_tx);
    timer_del(adjacency->timer_retry);
    timer_del(adjacency->timer_csnp);
    timer_del(adjacency->timer_csnp_next);

    if(g_ctx->routing_sessions) g_ctx->routing_sessions--;
}
