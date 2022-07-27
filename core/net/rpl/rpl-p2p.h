/**
 * \addtogroup uip6
 * @{
 */
/*
 * Copyright (c) 2011, Institut National de Recherche en Informatique et en
 * Automatique (INRIA)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
/**
 * \file
 *         RPL Point to Point (P2P) extension.
 *
 * Reactive Discovery of Point-to-Point Routes in Low Power and Lossy Networks
 * draft-ietf-roll-p2p-rpl-07
 *
 * \author Matthias Philipp <matthias-philipp@gmx.de>
 * Contributor: Emmanuel Baccelli <emmanuel.baccelli@inria.fr>
 */

#ifndef RPL_P2P_H
#define RPL_P2P_H

#include "net/uip.h"
#include "net/rpl/rpl-private.h"

#include "lib/list.h"

/* D flag is always zero in control messages (see draft-ietf-roll-rpl-14) */
#define RPL_LOCAL_DEFAULT_INSTANCE      0x80 /* local ID 0 */

/* Compression as used in route discoveries originated by this node. */
#define RPL_P2P_RDO_COMPR               15
/* Minimum RDO compression value that can be handled by this node. This value
 * affects the size of the memory being reserved for addresses in the RDO. If
 * all nodes use the same compr value this can be safely set to
 * RPL_P2P_RDO_COMPR. For full interoperability this should be set to zero. This
 * reserves 16 byte per address in the SRO and is a waste of memory in case
 * compression is used. */
#define RPL_P2P_RDO_COMPR_MIN           RPL_P2P_RDO_COMPR

/* We use one global DAG for routes to the sink, the remaining DAGs can be used
 * by the P2P extension */
#define RPL_MAX_DAG_P2P_ENTRIES         RPL_MAX_DAG_ENTRIES - 1
/* This is probably a waste of memory on most nodes and to few for others. Real
 * malloc() would be very helpful in this case. */
#define RPL_MAX_SRC_ROUTE_ADDR_ENTRIES  20

/* We use different Trickle timer parameters for P2P RPL */
#define DEFAULT_P2P_DIO_INTERVAL_DOUBLINGS 16
#define DEFAULT_P2P_DIO_INTERVAL_MIN       6
#define DEFAULT_P2P_DIO_REDUNDANCY         1

#define RPL_DRO_ACK_WAIT_TIME           (CLOCK_SECOND * 10)
#define RPL_DRO_CACHE_ENTRY_SIZE        64
#define RPL_MAX_DRO_CACHE_ENTRIES       2
#define RPL_MAX_DRO_REXMITS             2
#define RPL_P2P_MO_DEFAULT_VECTOR_SIZE  10
#define RPL_P2P_MO_MAX_REQUESTS         3
#define RPL_P2P_MAX_PARTIAL_ROUTES      3

#define RPL_MOP_P2P_ROUTE_DISCOVERY     4    /* DAG Mode of Operation */
#define RPL_DIO_SUBOPT_ROUTE_DISCOVERY  10   /* Route discovery option */
#define RPL_CODE_DRO                    0x04 /* Discovery reply object */
#define RPL_CODE_DRO_ACK                0x05 /* Discovery reply object Acknowledgement */
#define RPL_CODE_MO                     0x06 /* Measurement object */

/* Bitmasks of the route discovery option */
#define RPL_P2P_RDO_D_FLAG              0x80 /* Optimize route in both directions */
#define RPL_P2P_RDO_H_FLAG              0x40 /* Hop-by-hop routes desired */
#define RPL_P2P_RDO_N_MASK              0x30 /* Number of routes */
#define RPL_P2P_RDO_N_SHIFT             4
#define RPL_P2P_RDO_C_MASK              0x0f /* Compression */
#define RPL_P2P_RDO_C_SHIFT             0

#define RPL_P2P_RDO_L_MASK              0xc0 /* Temporary DAG lifetime */
#define RPL_P2P_RDO_L_SHIFT             6
#define RPL_P2P_RDO_R_MASK              0x3f /* Rem */
#define RPL_P2P_RDO_R_SHIFT             0

/* Bitmasks of the discovery reply object */
#define RPL_P2P_DRO_Q_MASK              0xC0 /* Sequence Number */
#define RPL_P2P_DRO_Q_SHIFT             6
#define RPL_P2P_DRO_S_FLAG              0x20 /* Stop bit */
#define RPL_P2P_DRO_A_FLAG              0x10 /* Acknowledgement */

/* Bitmasks of the measurement object */
#define RPL_P2P_MO_C_MASK               0xf0 /* Compression */
#define RPL_P2P_MO_C_SHIFT              4
#define RPL_P2P_MO_T_FLAG               0x08 /* Type */
#define RPL_P2P_MO_H_FLAG               0x04 /* Hop-by-hop */
#define RPL_P2P_MO_A_FLAG               0x02 /* Accumulate route */
#define RPL_P2P_MO_R_FLAG               0x01 /* Reverse */

#define RPL_P2P_MO_B_FLAG               0x80 /* Back request */
#define RPL_P2P_MO_I_FLAG               0x40 /* Intermediate reply */
#define RPL_P2P_MO_S_MASK               0x3f /* Sequence */
#define RPL_P2P_MO_S_SHIFT              0

#define RPL_P2P_MO_N_MASK               0xf0 /* Num */
#define RPL_P2P_MO_N_SHIFT              4
#define RPL_P2P_MO_I_MASK               0x0f /* Index */
#define RPL_P2P_MO_I_SHIFT              0

/* Lifetime field in the route discovery option */
#define RPL_P2P_DAG_LIFETIME_1_SEC      0
#define RPL_P2P_DAG_LIFETIME_4_SEC      1
#define RPL_P2P_DAG_LIFETIME_16_SEC     2
#define RPL_P2P_DAG_LIFETIME_64_SEC     3

/* Direction of the requested routes */
#define RPL_P2P_DEFAULT_BOTH_DIRECTIONS 1
/* Request hop-by-hop or source routes */
#define RPL_P2P_DEFAULT_HOP_BY_HOP      0
/* Number of requested routes */
#define RPL_P2P_DEFAULT_NUM_ROUTES      0
/* Lifetime of the temporary DAG */
#define RPL_P2P_DEFAULT_DAG_LIFETIME    RPL_P2P_DAG_LIFETIME_16_SEC
/* Hop limit for P2P DIOs */
#define RPL_P2P_DEFAULT_MAX_RANK        0
/* Whether the target should store backward source route */
#define RPL_P2P_STORE_BACKWARD_ROUTES   1
/* Whether to set the STOP bit when the number of requested routes has been reached */
#define RPL_P2P_SET_STOP_BIT            1
/* Time to wait for additional/alternative/better routes before sending DRO(s) */
#define RPL_P2P_DRO_WAIT_TIME           (CLOCK_SECOND * 3)

/* conveniently read and write bitmasks */
#define RPL_BITMASK_WRITE(name, b, v)   (b |= ((v << RPL_ ## name ## _SHIFT) & RPL_ ## name ## _MASK))
#define RPL_BITMASK_READ(name, b)       ((b & RPL_ ## name ## _MASK) >> RPL_ ## name ## _SHIFT)
/* conveniently set and get flags */
#define RPL_FLAG_SET(name, b)           (b |= RPL_ ## name ## _FLAG)
#define RPL_FLAG_CLR(name, b)           (b &= ~(RPL_ ## name ## _FLAG))
#define RPL_FLAG_GET(name, b)           (b & RPL_ ## name ## _FLAG)
/* Check whether this node is the target */
#define rpl_p2p_is_target(p2p)          (memcmp(((uint8_t *)rpl_get_own_addr(1)) + (p2p)->compr, &(p2p)->target, 16 - (p2p)->compr) == 0)

extern int rpl_p2p_event_dro;
#if WITH_RPL_P2P_MEASUREMENT
extern int rpl_p2p_event_mo;

typedef struct rpl_mo_result {
  uip_ipaddr_t target;
  uint16_t path_metric;
} rpl_mo_result_t;
#endif /* WITH_RPL_P2P_MEASUREMENT */

typedef union rpl_cmpr_addr_t {
  uint8_t u8[16 - RPL_P2P_RDO_COMPR];
} rpl_cmpr_addr_t;

typedef struct rpl_src_route_addr {
  struct rpl_src_route_addr *next;
  rpl_cmpr_addr_t addr;
} rpl_src_route_addr_t;

typedef struct rpl_dio_rdo {
  uint8_t *start;
  uint8_t flags1;
  uint8_t both_directions;
  uint8_t num_routes;
  uint8_t compr;
  uint8_t hop_by_hop;
  uint8_t lifetime;
  uint8_t max_rank;
  uint8_t *target;
  uint8_t *addr_start;
  uint8_t addr_len;
} rpl_dio_rdo_t;

typedef struct rpl_dag_p2p {
  struct rpl_dag *dag;
  struct  process *process;
  uint8_t both_directions;
  uint8_t hop_by_hop;
  uint8_t num_routes;
  uint8_t rdo_lifetime;
  uint8_t max_rank;
  uint16_t lifetime;
  uint8_t expired;
  uint8_t compr;
  uint8_t dro_seq;
  struct ctimer dro_timer;
  rpl_cmpr_addr_t target;
  rpl_metric_container_t constr;
  void *partial_routes_lists[RPL_P2P_MAX_PARTIAL_ROUTES];
  list_t partial_routes[RPL_P2P_MAX_PARTIAL_ROUTES];
} rpl_dag_p2p_t;

typedef struct rpl_dro_cache_entry {
  struct rpl_dro_cache *next;
  uint8_t rexmits;
  struct ctimer timer;
  uint8_t length;
  uint8_t buffer[RPL_DRO_CACHE_ENTRY_SIZE];
} rpl_dro_cache_entry_t;

uint8_t rpl_discover_route(uip_ipaddr_t *target);

struct rpl_dag_p2p *rpl_p2p_alloc_dag(rpl_dag_t *dag);
void rpl_p2p_free_dag_p2p(rpl_dag_p2p_t *p2p);
void rpl_p2p_purge_dags(void);
uint8_t rpl_p2p_meets_constraints(rpl_metric_container_t *metric, rpl_metric_container_t *constr);
void rpl_p2p_update_dag(struct rpl_dag *dag, struct rpl_dio *dio);
void rpl_p2p_process_target(struct rpl_dag *dag, struct rpl_dio *dio);
uint16_t rpl_p2p_lt2sec(uint8_t lifetime);
uint8_t rpl_p2p_add_partial_route(struct rpl_dag *dag, struct rpl_dio *dio);
uint8_t rpl_p2p_route_differs(struct rpl_dag *dag, struct rpl_dio *dio);
uip_ds6_src_route_t *rpl_p2p_add_src_route(uip_ipaddr_t *dest, uint8_t *addr_start, uint8_t addr_len, uint8_t compr, uint8_t reverse);
uip_ds6_route_t *rpl_p2p_add_hbh_route(uint8_t instance, uip_ipaddr_t *dodagid, uip_ipaddr_t *target, uip_ipaddr_t *nexthop);

void dro_output(rpl_dag_t *dag, list_t addr_list);
#if WITH_RPL_P2P_MEASUREMENT
void mo_output(uip_ipaddr_t *target);
#endif /* WITH_RPL_P2P_MEASUREMENT */

#if (DEBUG) & DEBUG_ANNOTATE
void rpl_annotate_rdo(uint8_t *rdo, uip_ipaddr_t *dodag_id);
void rpl_repaint_links(void);
#endif /* (DEBUG) & DEBUG_ANNOTATE */
#if (DEBUG) & DEBUG_ANNOTATE || defined(SENSLAB)
void rpl_print_rdo(uint8_t *rdo, uip_ipaddr_t *dodag_id);
#endif /* (DEBUG) & DEBUG_ANNOTATE || defined(SENSLAB) */

#endif /* RPL_P2P_H */
