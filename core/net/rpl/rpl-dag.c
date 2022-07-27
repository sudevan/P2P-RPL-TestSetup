/**
 * \addtogroup uip6
 * @{
 */
/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
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
 *
 */
/**
 * \file
 *         Logic for Directed Acyclic Graphs in RPL.
 *
 * \author Joakim Eriksson <joakime@sics.se>, Nicolas Tsiftes <nvt@sics.se>
 * Contributor: Matthias Philipp <matthias-philipp@gmx.de>
 */

#include "contiki.h"
#include "net/rpl/rpl-private.h"
#include "net/uip.h"
#include "net/uip-nd6.h"
#include "lib/list.h"
#include "lib/memb.h"
#include "sys/ctimer.h"

#include <limits.h>
#include <string.h>

#define DEBUG DEBUG_NONE
#include "net/uip-debug.h"

#if WITH_RPL_P2P
#include "net/rpl/rpl-p2p.h"
#endif /* WITH_RPL_P2P */

#include "net/neighbor-info.h"

/************************************************************************/
extern rpl_of_t RPL_OF;
static rpl_of_t * const objective_functions[] = {&RPL_OF};
/************************************************************************/
/* RPL definitions. */

#ifndef RPL_CONF_GROUNDED
#define RPL_GROUNDED                    0
#else
#define RPL_GROUNDED                    RPL_CONF_GROUNDED
#endif /* !RPL_CONF_GROUNDED */

#ifndef RPL_CONF_DIO_INTERVAL_MIN
#define RPL_DIO_INTERVAL_MIN            DEFAULT_DIO_INTERVAL_MIN
#else
#define RPL_DIO_INTERVAL_MIN            RPL_CONF_DIO_INTERVAL_MIN
#endif /* !RPL_CONF_DIO_INTERVAL_MIN */

#ifndef RPL_CONF_DIO_INTERVAL_DOUBLINGS
#define RPL_DIO_INTERVAL_DOUBLINGS      DEFAULT_DIO_INTERVAL_DOUBLINGS
#else
#define RPL_DIO_INTERVAL_DOUBLINGS      RPL_CONF_DIO_INTERVAL_DOUBLINGS
#endif /* !RPL_CONF_DIO_INTERVAL_DOUBLINGS */

const rpl_dodag_config_t DEFAULT_DODAG_CONFIG = {
  DEFAULT_DIO_INTERVAL_DOUBLINGS,
  DEFAULT_DIO_INTERVAL_MIN,
  DEFAULT_DIO_REDUNDANCY,
  DEFAULT_MAX_RANKINC,
  DEFAULT_MIN_HOPRANKINC,
  1,
  RPL_DEFAULT_LIFETIME,
  RPL_DEFAULT_LIFETIME_UNIT
};
/************************************************************************/
/* Allocate parents from the same static MEMB chunk to reduce memory waste. */
MEMB(parent_memb, struct rpl_parent, RPL_MAX_PARENTS);

rpl_dag_t dag_table[RPL_MAX_DAG_ENTRIES];

#if (DEBUG) & DEBUG_ANNOTATE
extern rpl_dag_t *latest_p2p_dag;
#endif /* (DEBUG) & DEBUG_ANNOTATE */
/************************************************************************/
/* Remove DAG parents with a rank that is at least the same as minimum_rank. */
void
rpl_remove_parents(rpl_dag_t *dag, rpl_rank_t minimum_rank)
{
  rpl_parent_t *p, *p2;

  PRINTF("RPL: Removing parents (minimum rank %u)\n",
	minimum_rank);

  for(p = list_head(dag->parents); p != NULL; p = p2) {
    p2 = p->next;
    if(p->rank >= minimum_rank) {
      rpl_remove_parent(dag, p);
    }
  }
}
/************************************************************************/
static void
remove_worst_parent(rpl_dag_t *dag, rpl_rank_t min_worst_rank)
{
  rpl_parent_t *p, *worst;

  PRINTF("RPL: Removing the worst parent\n");

  /* Find the parent with the highest rank. */
  worst = NULL;
  for(p = list_head(dag->parents); p != NULL; p = list_item_next(p)) {
    if(p != dag->preferred_parent &&
       (worst == NULL || p->rank > worst->rank)) {
      worst = p;
    }
  }
  /* Remove the neighbor if its rank is worse than the minimum worst
     rank. */
  if(worst != NULL && worst->rank > min_worst_rank) {
    rpl_remove_parent(dag, worst);
  }
}
/************************************************************************/
#if RPL_MOP_DEFAULT == RPL_MOP_STORING_NO_MULTICAST || RPL_MOP_DEFAULT == RPL_MOP_NON_STORING
static int
should_send_dao(rpl_dag_t *dag, rpl_dio_t *dio, rpl_parent_t *p)
{
  /* if MOP is set to no downward routes no DAO should be sent */
  if(dag->mop == RPL_MOP_NO_DOWNWARD_ROUTES) return 0;
  return dio->dtsn != p->dtsn && p == dag->preferred_parent;
}
#endif /* RPL_MOP_DEFAULT == RPL_MOP_STORING_NO_MULTICAST || RPL_MOP_DEFAULT == RPL_MOP_NON_STORING */
/************************************************************************/
static int
acceptable_rank(rpl_dag_t *dag, rpl_rank_t rank)
{
  return rank != INFINITE_RANK &&
    (dag->config.max_rankinc == 0 ||
     DAG_RANK(rank, dag) <= DAG_RANK(dag->min_rank + dag->config.max_rankinc, dag));
}
/************************************************************************/
uint8_t
rpl_set_dodag_config(rpl_dag_t *dag, const rpl_dodag_config_t *dcf)
{
  rpl_of_t *of;
  /* Determine objective function from the objective code point */
  of = rpl_find_of(dcf->ocp);
  if (of == NULL) {
    PRINTF("RPL: Unable to determine OF for OCP %d\n", dcf->ocp);
    return 0;
  }
  dag->of = of;
  /* copy configuration struct */
  memcpy(&dag->config, dcf, sizeof(rpl_dodag_config_t));
  return 1;
}
/************************************************************************/
rpl_dag_t *
rpl_set_root(uip_ipaddr_t *dag_id)
{
  rpl_dag_t *dag;
  int version;

  version = -1;
  dag = rpl_get_dag(RPL_DEFAULT_INSTANCE, NULL);
  if(dag != NULL) {
    PRINTF("RPL: Dropping a joined DAG when setting this node as root");
    version = dag->version;
    rpl_free_dag(dag);
  }

  dag = rpl_alloc_dag(RPL_DEFAULT_INSTANCE);
  if(dag == NULL) {
    PRINTF("RPL: Failed to allocate a DAG\n");
    return NULL;
  }

  dag->joined = 1;
  dag->version = version + 1;
  dag->grounded = RPL_GROUNDED;
  dag->mop = RPL_MOP_DEFAULT;
  dag->preferred_parent = NULL;
  dag->dtsn_out = 1; /* Trigger DAOs from the beginning. */

  rpl_set_dodag_config(dag, &DEFAULT_DODAG_CONFIG);
  memcpy(&dag->dag_id, dag_id, sizeof(dag->dag_id));


  dag->rank = ROOT_RANK(dag);

  dag->of->update_metric_container(dag);

  PRINTF("RPL: Node set to be a DAG root with DAG ID ");
  PRINT6ADDR(&dag->dag_id);
  PRINTF("\n");

#if (DEBUG) & DEBUG_ANNOTATE && WITH_RPL_P2P
  rpl_repaint_links();
#endif /* (DEBUG) & DEBUG_ANNOTATE && WITH_RPL_P2P */
#ifdef SENSLAB
  printf("RPL DAG ");
  print6addr(&dag->dag_id);
  printf(" 0x%02x ROOT\n", dag->instance_id);
#endif /* SENSLAB */

  rpl_reset_dio_timer(dag, 1);

  return dag;
}
/************************************************************************/
int
rpl_set_prefix(rpl_dag_t *dag, uip_ipaddr_t *prefix, int len)
{
  if(len <= 128) {
    memset(&dag->prefix_info.prefix, 0, 16);
    memcpy(&dag->prefix_info.prefix, prefix, (len + 7) / 8);
    dag->prefix_info.length = len;
    dag->prefix_info.flags = UIP_ND6_RA_FLAG_AUTONOMOUS;
    PRINTF("RPL: Prefix set - will announce this in DIOs\n");
    return 1;
  }
  return 0;
}
/************************************************************************/
int
rpl_set_default_route(rpl_dag_t *dag, uip_ipaddr_t *from)
{
#if WITH_RPL_P2P
  /* Don't set any default routes in P2P mode */
  if(dag->mop == RPL_MOP_P2P_ROUTE_DISCOVERY) {
    return 1;
  }
#endif /* WITH_RPL_P2P */

  if(dag->def_route != NULL) {
    PRINTF("RPL: Removing default route through ");
    PRINT6ADDR(&dag->def_route->ipaddr);
    PRINTF("\n");
    uip_ds6_defrt_rm(dag->def_route);
  }

  if(from != NULL) {
    PRINTF("RPL: Adding default route through ");
    PRINT6ADDR(from);
    PRINTF("\n");
    dag->def_route = uip_ds6_defrt_add(from,
                                       RPL_LIFETIME(dag,
                                                    dag->config.default_lifetime));
    if(dag->def_route == NULL) {
      return 0;
    }
  }

  return 1;
}
/************************************************************************/
rpl_dag_t *
rpl_alloc_dag(uint8_t instance_id)
{
  rpl_dag_t *dag;
  rpl_dag_t *end;

  for(dag = &dag_table[0], end = dag + RPL_MAX_DAG_ENTRIES; dag < end; dag++) {
    if(dag->used == 0) {
      memset(dag, 0, sizeof(*dag));
      dag->parents = &dag->parent_list;
      list_init(dag->parents);
      dag->instance_id = instance_id;
      dag->def_route = NULL;
      dag->rank = INFINITE_RANK;
      dag->min_rank = INFINITE_RANK;
      dag->used = 1;
      return dag;
    }
  }

  RPL_STAT(rpl_stats.mem_overflows++);
  return NULL;
}
/************************************************************************/
void
rpl_free_dag(rpl_dag_t *dag)
{
  PRINTF("RPL: Leaving the DAG ");
  PRINT6ADDR(&dag->dag_id);
  PRINTF("\n");

  /* Do NOT remove routes installed by P2P DROs when the temporary P2P DAG 
   * expires. */
  if (dag->mop != RPL_MOP_P2P_ROUTE_DISCOVERY) {
    /* Remove routes installed by DAOs. */
    rpl_remove_routes(dag);
  }

  /* Remove parents and the default route. */
  rpl_remove_parents(dag, 0);
  rpl_set_default_route(dag, NULL);

  ctimer_stop(&dag->dio_timer);
#if RPL_MOP_DEFAULT == RPL_MOP_STORING_NO_MULTICAST || RPL_MOP_DEFAULT == RPL_MOP_NON_STORING
  ctimer_stop(&dag->dao_timer);
#endif /* RPL_MOP_DEFAULT == RPL_MOP_STORING_NO_MULTICAST || RPL_MOP_DEFAULT == RPL_MOP_NON_STORING */

  dag->used = 0;
  dag->joined = 0;
}
/************************************************************************/
rpl_parent_t *
rpl_add_parent(rpl_dag_t *dag, rpl_dio_t *dio, uip_ipaddr_t *addr)
{
  rpl_parent_t *p;
  uip_ds6_nbr_t *nbr;

  p = memb_alloc(&parent_memb);
  if(p == NULL) {
    remove_worst_parent(dag, dio->rank);
    p = memb_alloc(&parent_memb);
    if(p == NULL) {
      RPL_STAT(rpl_stats.mem_overflows++);
#ifdef SENSLAB
      puts("ERR MEM parent");
#endif /* SENSLAB */
      return NULL;
    }
  }

  memcpy(&p->addr, addr, sizeof(p->addr));
  p->dag = dag;
  p->rank = dio->rank;
  /* get the actual value of the link metric for this neighbor */
  nbr = uip_ds6_nbr_lookup(addr);
  if(nbr != NULL) {
    p->link_metric = neighbor_info_get_metric((rimeaddr_t *)&nbr->lladdr);
    PRINTF("ETX to parent candidate %u is %d\n", addr->u8[15], NEIGHBOR_INFO_FIX2ETX(p->link_metric));
  } else {
    printf("ERR parent candidate %u not in neighbor cache!\n", addr->u8[15]);
    p->link_metric = INITIAL_LINK_METRIC;
  }

  p->dtsn = 0;

  memcpy(&p->mc, &dio->mc, sizeof(p->mc));

  list_add(dag->parents, p);

  return p;
}
/************************************************************************/
rpl_parent_t *
rpl_find_parent(rpl_dag_t *dag, uip_ipaddr_t *addr)
{
  rpl_parent_t *p;

  for(p = list_head(dag->parents); p != NULL; p = p->next) {
    if(uip_ipaddr_cmp(&p->addr, addr)) {
      return p;
    }
  }

  return NULL;
}
/************************************************************************/
#if (DEBUG) & DEBUG_ANNOTATE && WITH_RPL_P2P
void
rpl_repaint_links(void)
{
  static uint8_t old_global_bp_id = 0;
  static uint8_t old_local_bp_id = 0;
  uint8_t global_bp_id = 0;
  uint8_t local_bp_id = 0;
  rpl_dag_t *dag;

  /* set correct node color */
  if(latest_p2p_dag != NULL && !latest_p2p_dag->p2p->expired) {
    ANNOTATE("#A ping=\n");
    ANNOTATE("#A join=%u\n", latest_p2p_dag->dag_id.u8[15]);
    if(latest_p2p_dag->rank == ROOT_RANK(latest_p2p_dag)) {
      ANNOTATE("#A color=red\n");
    } else if(latest_p2p_dag->p2p->target.u8[sizeof(rpl_cmpr_addr_t) - 2] == rpl_get_own_addr(1)->u8[15]) {
      ANNOTATE("#A color=green\n");
    } else {
      ANNOTATE("#A color=orange\n");
      ANNOTATE("#A p2p=\n");
    }
  } else {
    ANNOTATE("#A join=\n");
    if((dag = rpl_get_dag(RPL_DEFAULT_INSTANCE, NULL)) != NULL) {
      if(dag->rank == ROOT_RANK(dag)) {
        ANNOTATE("#A color=blue\n");
        ANNOTATE("#A root=%u\n",dag->dag_id.u8[sizeof(dag->dag_id) - 1]);
      } else {
        ANNOTATE("#A color=orange\n");
      }
    } else {
      ANNOTATE("#A color=white\n");
    }
  }

  /* Determine currently best parents */
  dag = rpl_get_dag(RPL_DEFAULT_INSTANCE, NULL);
  if(dag != NULL && dag->preferred_parent != NULL) {
    global_bp_id = dag->preferred_parent->addr.u8[sizeof(uip_ipaddr_t) - 1];
  }
  /* Find the newest P2P DAG that is not yet expired */
  if(latest_p2p_dag != NULL &&
     latest_p2p_dag->joined &&
     !latest_p2p_dag->p2p->expired &&
     latest_p2p_dag->preferred_parent != NULL) {
    local_bp_id = latest_p2p_dag->preferred_parent->addr.u8[sizeof(uip_ipaddr_t) - 1];
  }

  if(old_global_bp_id == global_bp_id && old_local_bp_id == local_bp_id) {
    /* Nothing changed, nothing needs to be repainted */
    return;
  }
  /* Some old links may have to be removed */
  if(old_global_bp_id != 0) {
    ANNOTATE("#L %u 0\n", old_global_bp_id);
  }
  if(old_local_bp_id != 0) {
    ANNOTATE("#L %u 0\n", old_local_bp_id);
  }

  /* Add new links to best parents. */
  if (global_bp_id == local_bp_id) {
    /* Cooja does not support two relations between the same nodes, therefore we
     * define one relation that looks like two. */
    if(global_bp_id > 0) {
      ANNOTATE("#L %u 1 dashed,red,overlay\n", global_bp_id);
    }
  } else {
    if(global_bp_id > 0) {
      ANNOTATE("#L %u 1\n", global_bp_id);
    }
    if(local_bp_id > 0) {
      ANNOTATE("#L %u 1 dashed,red\n", local_bp_id);
    }
  }

  old_global_bp_id = global_bp_id;
  old_local_bp_id = local_bp_id;
}
#endif /* (DEBUG) & DEBUG_ANNOTATE && WITH_RPL_P2P */
/************************************************************************/
rpl_parent_t *
rpl_select_parent(rpl_dag_t *dag)
{
  rpl_parent_t *p;
  rpl_parent_t *best;

  best = NULL;
  for(p = list_head(dag->parents); p != NULL; p = p->next) {
    if(p->rank == INFINITE_RANK) {
      /* ignore this neighbor */
    } else if(best == NULL) {
      best = p;
    } else {
      best = dag->of->best_parent(best, p);
    }
  }

  if(best == NULL) {
    /* need to handle update of best... */
    return NULL;
  }

  if(dag->preferred_parent != best) {
    PRINTF("RPL: Sending a No-Path DAO to old DAO parent\n");
#if RPL_MOP_DEFAULT == RPL_MOP_STORING_NO_MULTICAST || RPL_MOP_DEFAULT == RPL_MOP_NON_STORING
    dao_output(dag->preferred_parent, ZERO_LIFETIME);
#endif /* RPL_MOP_DEFAULT == RPL_MOP_STORING_NO_MULTICAST || RPL_MOP_DEFAULT == RPL_MOP_NON_STORING */

    dag->preferred_parent = best; /* Cache the value. */
    dag->of->update_metric_container(dag);
    rpl_set_default_route(dag, &best->addr);
#if RPL_MOP_DEFAULT == RPL_MOP_STORING_NO_MULTICAST || RPL_MOP_DEFAULT == RPL_MOP_NON_STORING
    /* The DAO parent set changed - schedule a DAO transmission. */
    if(dag->mop != RPL_MOP_NO_DOWNWARD_ROUTES) {
      rpl_schedule_dao(dag);
    }
#endif /* RPL_MOP_DEFAULT == RPL_MOP_STORING_NO_MULTICAST || RPL_MOP_DEFAULT == RPL_MOP_NON_STORING */
    /* Do not reset trickle timer for P2P DAG. There the trickle operation
     * is only based on the route metrics and the DIO might thus still be
     * considered consistent even if the best parent changed. */
#if WITH_RPL_P2P
    if(dag->p2p == NULL) {
#endif /* WITH_RPL_P2P */
      rpl_reset_dio_timer(dag, 1);
#if WITH_RPL_P2P
    }
#endif /* WITH_RPL_P2P */
#if (DEBUG) & DEBUG_ANNOTATE && WITH_RPL_P2P
    rpl_repaint_links();
#endif /* (DEBUG) & DEBUG_ANNOTATE && WITH_RPL_P2P */
#ifdef SENSLAB
    if(dag->instance_id == RPL_DEFAULT_INSTANCE) {
      printf("RPL ");
    } else {
      printf("P2P ");
    }
    printf("DAG ");
    print6addr(&dag->dag_id);
    printf(" 0x%02x BP %u\n", dag->instance_id, best->addr.u8[15]);
#endif /* SENSLAB */
    PRINTF("RPL: New preferred parent, rank changed from %u to %u\n",
	   (unsigned)dag->rank, dag->of->calculate_rank(best, 0));
    RPL_STAT(rpl_stats.parent_switch++);
  }

  /* Update the DAG rank, since link-layer information may have changed
     the local confidence. */
  dag->rank = dag->of->calculate_rank(best, 0);
  if(dag->rank < dag->min_rank) {
    dag->min_rank = dag->rank;
  } else if(!acceptable_rank(dag, best->rank)) {
    /* Send a No-Path DAO to the soon-to-be-removed preferred parent. */
#if RPL_MOP_DEFAULT == RPL_MOP_STORING_NO_MULTICAST || RPL_MOP_DEFAULT == RPL_MOP_NON_STORING
    dao_output(best, ZERO_LIFETIME);
#endif /* RPL_MOP_DEFAULT == RPL_MOP_STORING_NO_MULTICAST || RPL_MOP_DEFAULT == RPL_MOP_NON_STORING */

    rpl_remove_parents(dag, 0);
    return NULL;
  }

  return best;
}
/************************************************************************/
int
rpl_remove_parent(rpl_dag_t *dag, rpl_parent_t *parent)
{
  uip_ds6_defrt_t *defrt;

#if WITH_RPL_P2P
  /* Don't remove any routes in P2P mode */
  if (dag->p2p == NULL) {
#endif /* WITH_RPL_P2P */
    /* Remove uIPv6 routes that have this parent as the next hop. **/
    uip_ds6_route_rm_by_nexthop(&parent->addr);
    defrt = uip_ds6_defrt_lookup(&parent->addr);
    if(defrt != NULL) {
      PRINTF("RPL: Removing default route ");
      PRINT6ADDR(&parent->addr);
      PRINTF("\n");
      uip_ds6_defrt_rm(defrt);
      dag->def_route = NULL;
    }
#if WITH_RPL_P2P
  }
#endif /* WITH_RPL_P2P */

  PRINTF("RPL: Removing parent ");
  PRINT6ADDR(&parent->addr);
  PRINTF("\n");

  if(parent == dag->preferred_parent) {
    dag->preferred_parent = NULL;
  }

  list_remove(dag->parents, parent);
  memb_free(&parent_memb, parent);
  return 0;
}
/************************************************************************/
rpl_dag_t *
rpl_get_dag(int instance_id, uip_ipaddr_t *dodag_id)
{
  int i;
  /* RPL_ANY_INSTANCE denotes any global (i.e. non-P2P) DAG, since we support only
   * one global DAG, this is the same as RPL_DEFAULT_INSTANCE */
  if(instance_id == RPL_ANY_INSTANCE) {
    instance_id = RPL_DEFAULT_INSTANCE;
  }
  /* A DODAG is identified by its RPLInstanceID, DODAGID, and version. But since
   * new version override old ones we never store more than one version for the
   * same DAG */
  for(i = 0; i < RPL_MAX_DAG_ENTRIES; i++) {
    if (dag_table[i].joined && ((
            dag_table[i].instance_id == instance_id &&
            dodag_id == NULL
          ) || (
            dag_table[i].instance_id == instance_id &&
            uip_ipaddr_cmp(&dag_table[i].dag_id, dodag_id)
          )
        )
       ) {
      return &dag_table[i];
    }
  }
  return NULL;
}
/************************************************************************/
rpl_of_t *
rpl_find_of(rpl_ocp_t ocp)
{
  unsigned int i;

  for(i = 0;
      i < sizeof(objective_functions) / sizeof(objective_functions[0]); 
      i++) {
    if(objective_functions[i]->ocp == ocp) {
      return objective_functions[i];
    }
  }

  return NULL;
}
/************************************************************************/
static void
join_dag(uip_ipaddr_t *from, rpl_dio_t *dio)
{
  rpl_dag_t *dag;
  rpl_parent_t *p;

  dag = rpl_alloc_dag(dio->instance_id);
  if(dag == NULL) {
    PRINTF("RPL: Failed to allocate a DAG object!\n");
#ifdef SENSLAB
    puts("ERR MEM dag");
#endif /* SENSLAB */
    return;
  }

  p = rpl_add_parent(dag, dio, from);
  PRINTF("RPL: Adding ");
  PRINT6ADDR(from);
  PRINTF(" as a parent: ");
  if(p == NULL) {
    PRINTF("failed\n");
    return;
  }
  PRINTF("succeeded\n");

  if (dio->dodag_config == NULL) {
    rpl_set_dodag_config(dag, &DEFAULT_DODAG_CONFIG);
  }
  else if (!rpl_set_dodag_config(dag, dio->dodag_config)) {
    PRINTF("RPL: DIO for DAG instance %u does not specify a supported OF\n",
        dio->instance_id);
    return;
  }

  if (dio->prefix_info != NULL) {
    /* Autoconfigure an address if this node does not already have an address
       with this prefix. */
    if (dio->prefix_info->flags & UIP_ND6_RA_FLAG_AUTONOMOUS) {
      uip_ipaddr_t ipaddr;
      if(uip_ds6_list_loop
         ((uip_ds6_element_t *) uip_ds6_if.addr_list, UIP_DS6_ADDR_NB,
         sizeof(uip_ds6_addr_t), &dio->prefix_info->prefix, 64,
         (uip_ds6_element_t **) & ipaddr) != FOUND) {
        /* assume that the prefix ends with zeros! */
        memcpy(&ipaddr, &dio->prefix_info->prefix, 16);
        uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
        PRINTF("RPL: adding global IP address ");
        PRINT6ADDR(&ipaddr);
        PRINTF("\n");
        uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);
      }
    }
    /* copy prefix information into the dag */
    memcpy(&dag->prefix_info, dio->prefix_info, sizeof(rpl_prefix_t));
  }

  dag->joined = 1;
  dag->grounded = dio->grounded;
  dag->mop = dio->mop;
  dag->preference = dio->preference;
  dag->instance_id = dio->instance_id;

  dag->version = dio->version;
  dag->preferred_parent = p;
  dag->of->update_metric_container(dag);

  memcpy(&dag->dag_id, &dio->dag_id, sizeof(dio->dag_id));

  dag->rank = dag->of->calculate_rank(p, dio->rank);
  dag->min_rank = dag->rank; /* So far this is the lowest rank we know of. */

  rpl_reset_dio_timer(dag, 1);
  rpl_set_default_route(dag, from);

#if WITH_RPL_P2P
  if (dio->mop == RPL_MOP_P2P_ROUTE_DISCOVERY) {
    /* Check maximum rank constraint */
    if(dio->rdo->max_rank > 0 && DAG_RANK(dag->rank, dag) > dio->rdo->max_rank) {
      printf("P2P: Maximum rank exceeded, not joining DAG %u (%u) > %u\n", DAG_RANK(dag->rank, dag), dag->rank, dio->rdo->max_rank);
      rpl_free_dag(dag);
      return;
    }
    /* Allocate memory for the P2P extension */
    dag->p2p = rpl_p2p_alloc_dag(dag);
    if (dag->p2p == NULL) {
      PRINTF("RPL: Failed to allocate P2P extension for DAG object!\n");
#ifdef SENSLAB
      puts("ERR MEM p2p");
#endif /* SENSLAB */
      rpl_free_dag(dag);
      return;
    }
    /* Initialize additional attributes for the P2P extension */
    rpl_p2p_update_dag(dag, dio);
    /* Set lifetime in seconds */
    dag->p2p->lifetime = rpl_p2p_lt2sec(dag->p2p->rdo_lifetime);
    /* This has to be called after the DIO timer was reset, because it stops the
     * timer in case this node is the target. */
    rpl_p2p_process_target(dag, dio);
  } else {
    dag->p2p = NULL;
  }
#endif /* WITH_RPL_P2P */

  PRINTF("RPL: Joined DAG with instance ID %u, rank %hu, DAG ID ",
         dio->instance_id, dag->rank);
  PRINT6ADDR(&dag->dag_id);
  PRINTF("\n");

#ifdef SENSLAB
  if(dag->instance_id == RPL_DEFAULT_INSTANCE) {
    printf("RPL ");
  } else {
    printf("P2P ");
  }
  printf("DAG ");
  print6addr(&dag->dag_id);
  printf(" 0x%02x JOIN\n", dag->instance_id);
  if(dag->instance_id == RPL_DEFAULT_INSTANCE) {
    printf("RPL ");
  } else {
    printf("P2P ");
  }
  printf("DAG ");
  print6addr(&dag->dag_id);
  printf(" 0x%02x BP %u\n", dag->instance_id, p->addr.u8[15]);
#endif /* SENSLAB */

#if (DEBUG) & DEBUG_ANNOTATE && WITH_RPL_P2P
  /* Visualize the change in Cooja */
  rpl_repaint_links();
#endif /* (DEBUG) & DEBUG_ANNOTATE && WITH_RPL_P2P */

#if RPL_MOP_DEFAULT == RPL_MOP_STORING_NO_MULTICAST || RPL_MOP_DEFAULT == RPL_MOP_NON_STORING
  if(should_send_dao(dag, dio, p)) {
    rpl_schedule_dao(dag);
  } else {
    PRINTF("RPL: The DIO does not meet the prerequisites for sending a DAO\n");
  }
#endif /* RPL_MOP_DEFAULT == RPL_MOP_STORING_NO_MULTICAST || RPL_MOP_DEFAULT == RPL_MOP_NON_STORING */
}
/************************************************************************/
static void
global_repair(uip_ipaddr_t *from, rpl_dag_t *dag, rpl_dio_t *dio)
{
  rpl_parent_t *p;

  rpl_remove_parents(dag, 0);
  dag->version = dio->version;
  dag->dtsn_out = 1;
  dag->of->reset(dag);
  if((p = rpl_add_parent(dag, dio, from)) == NULL) {
    PRINTF("RPL: Failed to add a parent during the global repair\n");
    dag->rank = INFINITE_RANK;
  } else {
    rpl_set_default_route(dag, from);
    dag->rank = dag->of->calculate_rank(NULL, dio->rank);
    dag->min_rank = dag->rank;
    rpl_reset_dio_timer(dag, 1);
#if RPL_MOP_DEFAULT == RPL_MOP_STORING_NO_MULTICAST || RPL_MOP_DEFAULT == RPL_MOP_NON_STORING
    if(should_send_dao(dag, dio, p)) {
      rpl_schedule_dao(dag);
    }
#endif /* RPL_MOP_DEFAULT == RPL_MOP_STORING_NO_MULTICAST || RPL_MOP_DEFAULT == RPL_MOP_NON_STORING */
  }
  PRINTF("RPL: Participating in a global repair (version=%u, rank=%hu)\n",
         dag->version, dag->rank);

  RPL_STAT(rpl_stats.global_repairs++);
}
/************************************************************************/
int
rpl_repair_dag(rpl_dag_t *dag)
{
  if(dag->rank == ROOT_RANK(dag)) {
    dag->version++;
    dag->dtsn_out = 1;
    rpl_reset_dio_timer(dag, 1);
    return 1;
  }
  return 0;
}
/************************************************************************/
void
rpl_local_repair(rpl_dag_t *dag)
{
  PRINTF("RPL: Starting a local DAG repair\n");

  dag->rank = INFINITE_RANK;
  rpl_remove_parents(dag, 0);
  rpl_reset_dio_timer(dag, 1);

  RPL_STAT(rpl_stats.local_repairs++);
}
/************************************************************************/
void
rpl_recalculate_ranks(void)
{
  rpl_dag_t *dag;
  rpl_parent_t *p;

  /*
   * We recalculate ranks when we receive feedback from the system (i.e. ETX
   * changes) rather than RPL protocol messages. This periodical recalculation
   * is called from a timer in order to keep the stack depth reasonably low.
   *
   * Rank re-caclulation is only done for the global DAG, because the local DAGs
   * from the P2P extension are only updated from incoming DIOs as opposed to
   * system feedback. This is because feedback-triggered recalculation would
   * require storing the whole route discovery DIO information (in particular
   * the source route option) for every neighbor.
   */
  dag = rpl_get_dag(RPL_ANY_INSTANCE, NULL);
  if(dag != NULL) {
    for(p = list_head(dag->parents); p != NULL; p = p->next) {
      if(p->updated) {
	p->updated = 0;
	rpl_process_parent_event(dag, p);
	/*
	 * Stop calculating here because the parent list may have changed.
	 * If more ranks need to be recalculated, it will be taken care of
	 * in subsequent calls to this functions.
	 */
	break;
      }
    }
  }
}
/************************************************************************/
int
rpl_process_parent_event(rpl_dag_t *dag, rpl_parent_t *p)
{
  rpl_rank_t parent_rank;
  rpl_rank_t old_rank;

  /* Update the parent rank. */
  parent_rank = p->rank;
  old_rank = dag->rank;

  if(rpl_select_parent(dag) == NULL) {
    /* No suitable parent; trigger a local repair. */
    PRINTF("RPL: No parents found in a DAG\n");
    rpl_local_repair(dag);
    return 1;
  }

  if(DAG_RANK(old_rank, dag) != DAG_RANK(dag->rank, dag)) {
    if(dag->rank < dag->min_rank) {
      dag->min_rank = dag->rank;
    }
    PRINTF("RPL: Moving in the DAG from rank %hu to %hu\n",
	   DAG_RANK(old_rank, dag), DAG_RANK(dag->rank, dag));
    PRINTF("RPL: The preferred parent is ");
    PRINT6ADDR(&dag->preferred_parent->addr);
    PRINTF(" (rank %u)\n",
           (unsigned)DAG_RANK(dag->preferred_parent->rank, dag));
    /* Do not reset trickle timer for P2P DAG. There the trickle operation
     * is only based on the route metrics and the DIO might thus still be
     * considered consistent even if the rank changed. */
#if WITH_RPL_P2P
    if(dag->p2p == NULL) {
#endif /* WITH_RPL_P2P */
      rpl_reset_dio_timer(dag, 1);
#if WITH_RPL_P2P
    }
#endif /* WITH_RPL_P2P */
  }

  if(parent_rank == INFINITE_RANK ||
     !acceptable_rank(dag, dag->of->calculate_rank(NULL, parent_rank))) {
    /* The candidate parent is no longer valid: the rank increase resulting
       from the choice of it as a parent would be too high. */
    return 0;
  }

  return 1;
}
/************************************************************************/
void
rpl_process_dio(uip_ipaddr_t *from, rpl_dio_t *dio)
{
  rpl_dag_t *dag;
  rpl_parent_t *p;
#if WITH_RPL_P2P
  uip_ds6_nbr_t *nbr;
  rpl_parent_t *p2;
  int adv_route_improved = 0;
#endif /* WITH_RPL_P2P */

#if WITH_RPL_P2P
  if(dio->mop != RPL_MOP_DEFAULT && dio->mop != RPL_MOP_P2P_ROUTE_DISCOVERY) {
#else
  if(dio->mop != RPL_MOP_DEFAULT) {
#endif /* WITH_RPL_P2P */
    PRINTF("RPL: Ignoring a DIO with an unsupported MOP: %d\n", dio->mop);
    return;
  }

#if WITH_RPL_P2P
  if(dio->mop == RPL_MOP_P2P_ROUTE_DISCOVERY && dio->rdo == NULL) {
    PRINTF("RPL: Ignoring a P2P Mode DIO without route discovery option\n");
    return;
  }
  /* Check whether we have bidirectional reachcability with the sender of the DIO.
   * Since the ETX value is based on the LQI of incoming packets we cannot check
   * for that. However, links with ETX <= 3 are ussually bidirectional. */
  nbr = uip_ds6_nbr_lookup(from);
  if(nbr == NULL || neighbor_info_get_metric((rimeaddr_t *)&nbr->lladdr) > NEIGHBOR_INFO_ETX2FIX(3)) {
    PRINTF("RPL: Discarding DIO because of unidirectional reachability\n");
    return;
  }
  /* XXX The draft recommends (SHOULD) the metric container to be updated before
   * the constraint is evaluated, which is not the case in this implementation.
   * The objective function framework in Contiki only allows to update the
   * metric container for the current best parent, and not an arbitrary parent
   * (as the DIO originator). */
  if (dio->mop == RPL_MOP_P2P_ROUTE_DISCOVERY && !rpl_p2p_meets_constraints(&dio->mc, dio->constr)) {
    printf("P2P: Constraints not met, ignoring DIO\n");
    return;
  }
#endif /* WITH_RPL_P2P */

  dag = rpl_get_dag(dio->instance_id, &dio->dag_id);
  if(dag == NULL) {
    /* Join the first possible DAG of this RPL instance. */
    if(dio->rank != INFINITE_RANK) {
      join_dag(from, dio);
    } else {
      PRINTF("RPL: Ignoring DIO from node with infinite rank: ");
      PRINT6ADDR(from);
      PRINTF("\n");
    }
    return;
  }

  if(memcmp(&dag->dag_id, &dio->dag_id, sizeof(dag->dag_id))) {
    PRINTF("RPL: Ignoring DIO for another DAG within our instance\n");
    return;
  }

  if(dio->version > dag->version) {
    if(dag->rank == ROOT_RANK(dag)) {
      PRINTF("RPL: Root received inconsistent DIO version number\n");
      dag->version = dio->version + 1;
      rpl_reset_dio_timer(dag, 1);
    } else {
      global_repair(from, dag, dio);
    }
    return;
  } else if(dio->version < dag->version) {
    /* Inconsistency detected - someone is still on old version */
    PRINTF("RPL: old version received => inconsistency detected\n");
    rpl_reset_dio_timer(dag, 1);
    return;
  }

  if(dio->rank == INFINITE_RANK) {
    rpl_reset_dio_timer(dag, 1);
  } else if(dio->rank < ROOT_RANK(dag)) {
    PRINTF("RPL: Ignoring DIO with too low rank: %u\n",
           (unsigned)dio->rank);
    return;
  }

  if(dag->rank == ROOT_RANK(dag)) {
    if(dio->rank != INFINITE_RANK) {
      dag->dio_counter++;
    }
    return;
  }

#if WITH_RPL_P2P
  if (dag->mop == RPL_MOP_P2P_ROUTE_DISCOVERY) {
    if(dag->p2p->expired) {
      PRINTF("RPL: Ignoring DIO for already expired P2P DAG\n");
      return;
    }
    /* Check maximum rank constraint */
    if(dio->rdo->max_rank > 0 && DAG_RANK(dio->rank, dag) > dio->rdo->max_rank) {
      printf("P2P: Ignoring DIO whith too high rank %u (%u) > %u\n", DAG_RANK(dio->rank, dag), dag->rank, dio->rdo->max_rank);
      return;
    }
  }
#endif /* WITH_RPL_P2P */

  /*
   * At this point, we know that this DIO pertains to a DAG that
   * we are already part of. We consider the sender of the DIO to be
   * a candidate parent, and let rpl_process_parent_event decide
   * whether to keep it in the set.
   */

  p = rpl_find_parent(dag, from);
  if(p == NULL) {
    if(RPL_PARENT_COUNT(dag) == RPL_MAX_PARENTS) {
      /* Make room for a new parent. */
      remove_worst_parent(dag, dio->rank);
    }
    
    /* Add the DIO sender as a candidate parent. */
    p = rpl_add_parent(dag, dio, from);
    if(p == NULL) {
      PRINTF("RPL: Failed to add a new parent (");
      PRINT6ADDR(from);
      PRINTF(")\n");
      return;
    }

    PRINTF("RPL: New candidate parent with rank %u: ", (unsigned)p->rank);
    PRINT6ADDR(from);
    PRINTF("\n");
  } else if(DAG_RANK(p->rank, dag) == DAG_RANK(dio->rank, dag)) {
    /* Do not increase DIO counter for P2P DAG. There the trickle operation
     * is only based on the route metrics and the DIO might thus still be
     * considered inconsistent even if the rank did not change. */
#if WITH_RPL_P2P
    if(dag->p2p == NULL) {
#endif /* WITH_RPL_P2P */
      PRINTF("RPL: Received consistent DIO\n");
      dag->dio_counter++;
#if WITH_RPL_P2P
    }
#endif /* WITH_RPL_P2P */
  }
  
#if WITH_RPL_P2P
  if (dio->mop == RPL_MOP_P2P_ROUTE_DISCOVERY) {
    /* Make sure that the neighbor set only contains the current preferred parent
     * and the originator of this DIO. Thereby we can be sure that, if the preferred  
     * parent was changed in rpl_process_parent_event() it was changed to the DIO 
     * originator. Otherwise rpl_process_parent_event() could select any neighbor
     * in the parent set and we would therefore need to store the partial route 
     * for each element in the parent set. */
    for(p2 = list_head(dag->parents); p2 != NULL; p2 = list_item_next(p2)) {
      if(p2 != p && p2 != dag->preferred_parent) {
        rpl_remove_parent(dag, p2);
      }
    }
    /* Check whether this DIO will lead to an improvement in the route that we advertise */
    adv_route_improved = (dag->of->p2p_route_cmp(dag, dio, p) > 0);
  }
#endif /* WITH_RPL_P2P */

  /* We have allocated a candidate parent; process the DIO further. */

  memcpy(&p->mc, &dio->mc, sizeof(p->mc));    
  p->rank = dio->rank;
  if(rpl_process_parent_event(dag, p) == 0) {
    /* The candidate parent no longer exists. */
    return;
  }
  
#if WITH_RPL_P2P
  if (dio->mop == RPL_MOP_P2P_ROUTE_DISCOVERY) {
    /* Trickle operation is slightly different in P2P mode. The decision whether
      * this DIO is consistent or not only relies on the metric value. */
    if(adv_route_improved) {
      /* DIO lead to metric improvement and is thus considered inconsistent. */
      printf("P2P: Advertised route improved, rank: %d, bp: %u\n", dag->rank, dag->preferred_parent->addr.u8[15]);
      rpl_p2p_update_dag(dag, dio);
      rpl_reset_dio_timer(dag, 1);
    } else if (dag->of->p2p_route_cmp(dag, dio, NULL) >= 0) {
      /* DIO advertises better or same route than we do, but did not lead to an
        * improvement in the value that we advertise. It is thus considered
        * consistent. */
      /* Sometimes we receive a second DIO before the DIO timer fired even 
        * once, although it was reset to 0. (This probably only occurs in
        * simulations.) Therefore only increase the redundancy counter if we 
        * sent at least one DIO after the last timer reset. */
      PRINTF("originator: %u, preferred parent: %u\n", p->addr.u8[15], dag->preferred_parent->addr.u8[15]);
      //if(dag->dio_intcurrent > dag->config.dio_intmin) {
      //if(1) {
      if(p != dag->preferred_parent) {
        dag->dio_counter++;
      } 
      if (dag->of->p2p_route_cmp(dag, dio, p) >= 0) {
        /* No change in the path metric but probably a different path.
          * Add this partial route to the list in order to increase diversity. */
        if(rpl_p2p_route_differs(dag, dio)) {
          printf("P2P: Alternative partial route ");
          rpl_print_rdo(dio->rdo->start, &dio->dag_id);
          /* Store alternative partial route */
          if(rpl_p2p_add_partial_route(dag, dio)) {
            printf(" cached\n");
          } else {
            printf(" NOT cached\n");
          }
        }
      }
    } else {
      /* No improvement and DIO advertises worse metrics, thus no impact on
        * trickle. */
    }
    /* Stop the DIO timer and start the DRO timer if we are the target. */
    rpl_p2p_process_target(dag, dio);
  }
#endif /* WITH_RPL_P2P */

#if RPL_MOP_DEFAULT == RPL_MOP_STORING_NO_MULTICAST || RPL_MOP_DEFAULT == RPL_MOP_NON_STORING
  if(should_send_dao(dag, dio, p)) {
    rpl_schedule_dao(dag);
  }
#endif /* RPL_MOP_DEFAULT == RPL_MOP_STORING_NO_MULTICAST || RPL_MOP_DEFAULT == RPL_MOP_NON_STORING */
  
  p->dtsn = dio->dtsn;
}
/************************************************************************/

