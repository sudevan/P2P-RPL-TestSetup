/**
 * \addtogroup uip6
 * @{
 */
/*
 * Copyright (c) 2009, Swedish Institute of Computer Science.
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
 *
 * This file is part of the Contiki operating system.
 */
/**
 * \file
 *         ContikiRPL, an implementation of IETF ROLL RPL.
 *
 * \author Joakim Eriksson <joakime@sics.se>, Nicolas Tsiftes <nvt@sics.se>
 * Contributor: Matthias Philipp <matthias-philipp@gmx.de>
 */

#if WITH_RPL_P2P
#include "net/rpl/rpl-p2p.h"
#include "sys/process.h"
#endif /* WITH_RPL_P2P */

#include "net/uip.h"
#include "net/tcpip.h"
#include "net/uip-ds6.h"
#include "net/rpl/rpl-private.h"
#include "net/neighbor-info.h"

#define DEBUG DEBUG_NONE
#include "net/uip-debug.h"

#include <limits.h>
#include <string.h>

#if RPL_CONF_STATS
rpl_stats_t rpl_stats;
#endif

/************************************************************************/
extern uip_ds6_route_t uip_ds6_routing_table[UIP_DS6_ROUTE_NB];
extern struct rpl_dag dag_table[RPL_MAX_DAG_ENTRIES];
#if WITH_RPL_P2P
extern uip_ds6_src_route_t uip_ds6_src_routing_table[UIP_DS6_SRC_ROUTE_NB];
#endif /* WITH_RPL_P2P */
/************************************************************************/
void
rpl_purge_routes(void)
{
  int i;

  for(i = 0; i < UIP_DS6_ROUTE_NB; i++) {
    if(uip_ds6_routing_table[i].isused) {
      if(uip_ds6_routing_table[i].state.lifetime <= 1) {
        uip_ds6_route_rm(&uip_ds6_routing_table[i]);
      } else {
        uip_ds6_routing_table[i].state.lifetime--;
      }
    }
  }
#if WITH_RPL_P2P
  /* Do the same for source routes */
  for (i = 0; i < UIP_DS6_SRC_ROUTE_NB; i++) {
    if (uip_ds6_src_routing_table[i].isused) {
      if (uip_ds6_src_routing_table[i].state.lifetime <= 1) {
        PRINTF("RPL: purging source route to ");
        PRINT6ADDR(&uip_ds6_src_routing_table[i].ipaddr);
        PRINTF("\n");
        uip_ds6_src_route_rm(&uip_ds6_src_routing_table[i]);
      }
      else {
        uip_ds6_src_routing_table[i].state.lifetime--;
      }
    }
  }
#endif /* WITH_RPL_P2P */
}
/************************************************************************/
void
rpl_remove_routes(rpl_dag_t *dag)
{
  int i;

  for(i = 0; i < UIP_DS6_ROUTE_NB; i++) {
    if(uip_ds6_routing_table[i].state.dag == dag) {
      uip_ds6_route_rm(&uip_ds6_routing_table[i]);
    }
  }
}
/************************************************************************/
uip_ds6_route_t *
rpl_add_route(rpl_dag_t *dag, uip_ipaddr_t *prefix, int prefix_len,
              uip_ipaddr_t *next_hop)
{
  uip_ds6_route_t *rep;
  uint8_t i, i_min;

  rep = uip_ds6_route_lookup(prefix);
  if(rep == NULL) {
    if((rep = uip_ds6_route_add(prefix, prefix_len, next_hop, 0)) == NULL) {
      /* No space for more route entries, remove the one that is to expire next. */
      for(i = 0, i_min = 0; i < UIP_DS6_ROUTE_NB; i++) {
        if(uip_ds6_routing_table[i].state.lifetime < uip_ds6_routing_table[i_min].state.lifetime) {
          i_min = i;
        }
      }
      PRINTF("RPL: No space for new route entry, removing old route to ");
      PRINT6ADDR(&uip_ds6_routing_table[i_min].ipaddr);
      PRINTF("\n");
      uip_ds6_route_rm(&uip_ds6_routing_table[i_min]);
      /* Try again to add the route */
      if((rep = uip_ds6_route_add(prefix, prefix_len, next_hop, 0)) == NULL) {
        PRINTF("RPL: No space for more route entries\n");
        return NULL;
      }
    }
  } else {
    PRINTF("RPL: Updated the next hop for prefix ");
    PRINT6ADDR(prefix);
    PRINTF(" to ");
    PRINT6ADDR(next_hop);
    PRINTF("\n");
    uip_ipaddr_copy(&rep->nexthop, next_hop);
  }
  rep->state.dag = dag;
  if(dag == NULL) {
    rep->state.lifetime = RPL_DEFAULT_LIFETIME * RPL_DEFAULT_LIFETIME_UNIT;
  } else {
    rep->state.lifetime = RPL_LIFETIME(dag, dag->config.default_lifetime);
  }
  rep->state.learned_from = RPL_ROUTE_FROM_INTERNAL;

  PRINTF("RPL: Added a route to ");
  PRINT6ADDR(prefix);
  PRINTF("/%d via ", prefix_len);
  PRINT6ADDR(next_hop);
  PRINTF("\n");

  return rep;
}
/************************************************************************/
static void
rpl_link_neighbor_callback(const rimeaddr_t *addr, int known, int etx)
{
  uip_ipaddr_t ipaddr;
  rpl_dag_t *dag;
  rpl_parent_t *parent;

  uip_ip6addr(&ipaddr, 0xfe80, 0, 0, 0, 0, 0, 0, 0);
  uip_ds6_set_addr_iid(&ipaddr, (uip_lladdr_t *)addr);
  PRINTF("RPL: Neighbor ");
  PRINT6ADDR(&ipaddr);
  PRINTF(" is %sknown. ETX = %u\n", known ? "" : "no longer ", NEIGHBOR_INFO_FIX2ETX(etx));

  /* Update all the DAGs */
  for(dag = &dag_table[0]; dag <= &dag_table[RPL_MAX_DAG_ENTRIES - 1]; dag++) {
    if (!dag->joined
#if WITH_RPL_P2P
        || (dag->mop == RPL_MOP_P2P_ROUTE_DISCOVERY && dag->p2p->expired)
#endif /* WITH_RPL_P2P */
        ) {
      continue;
    }

    parent = rpl_find_parent(dag, &ipaddr);
    if(parent == NULL) {
      if(!known) {
        PRINTF("RPL: Deleting routes installed by DAOs received from ");
        PRINT6ADDR(&ipaddr);
        PRINTF("\n");
        uip_ds6_route_rm_by_nexthop(&ipaddr);
      }
      continue;
    }

    /* Trigger DAG rank recalculation. */
    parent->updated = 1;

    parent->link_metric = etx;

    if(dag->of->parent_state_callback != NULL) {
      dag->of->parent_state_callback(parent, known, etx);
    }

    if(!known) {
      PRINTF("RPL: Removing parent ");
      PRINT6ADDR(&parent->addr);
      PRINTF(" because of bad connectivity (ETX %d)\n", etx);
      parent->rank = INFINITE_RANK;
    }
  }
}
/************************************************************************/
void
rpl_ipv6_neighbor_callback(uip_ds6_nbr_t *nbr)
{
  rpl_dag_t *dag;
  rpl_parent_t *p;

  /* Local DAGs by the P2P extension are only updated by DIOs */
  dag = rpl_get_dag(RPL_ANY_INSTANCE, NULL);
  if(dag == NULL) {
    return;
  }

  /* if this is our default route then clean the dag->def_route state */
  if(dag->def_route != NULL &&
     uip_ipaddr_cmp(&dag->def_route->ipaddr, &nbr->ipaddr)) {
    dag->def_route = NULL;
  }

  if(!nbr->isused) {
    PRINTF("RPL: Removing neighbor ");
    PRINT6ADDR(&nbr->ipaddr);
    PRINTF("\n");
    p = rpl_find_parent(dag, &nbr->ipaddr);
    if(p != NULL) {
      p->rank = INFINITE_RANK;
      /* Trigger DAG rank recalculation. */
      p->updated = 1;
    }
  }
}
/************************************************************************/
void
rpl_init(void)
{
  uip_ipaddr_t rplmaddr;
  PRINTF("RPL started\n");

  rpl_reset_periodic_timer();
  neighbor_info_subscribe(rpl_link_neighbor_callback);

  /* add rpl multicast address */
  uip_create_linklocal_rplnodes_mcast(&rplmaddr);
  uip_ds6_maddr_add(&rplmaddr);

#if RPL_CONF_STATS
  memset(&rpl_stats, 0, sizeof(rpl_stats));
#endif
#if WITH_RPL_P2P
  rpl_p2p_event_dro = process_alloc_event();
#if WITH_RPL_P2P_MEASUREMENT
  rpl_p2p_event_mo = process_alloc_event();
#endif /* WITH_RPL_P2P_MEASUREMENT */
#endif /* WITH_RPL_P2P */
}
/************************************************************************/
