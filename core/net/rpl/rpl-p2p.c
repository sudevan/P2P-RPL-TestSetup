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

#define DEBUG DEBUG_NONE
#include "net/uip-debug.h"

#include "net/rpl/rpl.h"
#include "net/rpl/rpl-p2p.h"

#include "net/uip.h"
#include "net/uip-ds6.h"
#include "lib/list.h"
#include "lib/memb.h"
#include "sys/ctimer.h"

#include <limits.h>
#include <string.h>

/************************************************************************/
int rpl_p2p_event_dro;
#if WITH_RPL_P2P_MEASUREMENT
int rpl_p2p_event_mo;
#endif /* WITH_RPL_P2P_MEASUREMENT */
/************************************************************************/
static rpl_dag_p2p_t dag_p2p_table[RPL_MAX_DAG_P2P_ENTRIES];
MEMB(src_route_addr_memb, rpl_src_route_addr_t, RPL_MAX_SRC_ROUTE_ADDR_ENTRIES);
static uint8_t instance = 0;
rpl_dag_t *latest_p2p_dag = NULL;
/************************************************************************/
static void free_addr_list(list_t addr_list);
static rpl_src_route_addr_t * add_src_route_addr(list_t addr_list, rpl_cmpr_addr_t *addr);
/************************************************************************/
const rpl_dodag_config_t P2P_DEFAULT_DODAG_CONFIG = {
  DEFAULT_P2P_DIO_INTERVAL_DOUBLINGS,
  DEFAULT_P2P_DIO_INTERVAL_MIN,
  DEFAULT_P2P_DIO_REDUNDANCY,
  0,   /* Max rank increase */
  DEFAULT_MIN_HOPRANKINC,
  1,   /* OCP */
  RPL_DEFAULT_LIFETIME,
  RPL_DEFAULT_LIFETIME_UNIT
};
/************************************************************************/
static void
free_addr_list(list_t addr_list)
{
  rpl_src_route_addr_t *a;

  /* Free memory allocated by source route */
  for(a = list_pop(addr_list); a != NULL; a = list_pop(addr_list)) {
    memb_free(&src_route_addr_memb, a);
  }
}
/************************************************************************/
static rpl_src_route_addr_t *
add_src_route_addr(list_t addr_list, rpl_cmpr_addr_t *addr)
{
  rpl_src_route_addr_t *a;

  a = memb_alloc(&src_route_addr_memb);
  if (a == NULL) {
    RPL_STAT(rpl_stats.mem_overflows++);
    return NULL;
  }
  memcpy(&a->addr, addr, sizeof(rpl_cmpr_addr_t));
  list_add(addr_list, a);

  return a;
}
/*---------------------------------------------------------------------------*/
uint8_t
rpl_p2p_add_partial_route(rpl_dag_t *dag, rpl_dio_t *dio)
{
  uint8_t i, j;

  for(i = 0; i < RPL_P2P_MAX_PARTIAL_ROUTES; i++) {
    if(list_length(dag->p2p->partial_routes[i]) == 0) {
      /* Copy the addresses from the packet buffer into static memory */
      for(j = 0; j < dio->rdo->addr_len; j += 16 - dio->rdo->compr) {
        if(add_src_route_addr(dag->p2p->partial_routes[i], (rpl_cmpr_addr_t *)(dio->rdo->addr_start + j)) == NULL) {
          free_addr_list(dag->p2p->partial_routes[i]);
          return 0;
        }
      }
      /* If the address vector is empty we add the DODAGID as special value in 
       * order to being able to tell an unused partial address list apart from a
       * one hop route. */
      if(dio->rdo->addr_len == 0) {
        if(add_src_route_addr(dag->p2p->partial_routes[i], (rpl_cmpr_addr_t *)(((uint8_t *)&dag->dag_id) + dio->rdo->compr)) == NULL) {
          free_addr_list(dag->p2p->partial_routes[i]);
          return 0;
        }
      }
      /* We do not add our compressed global address to the source route option yet,
       * since this would unnecessarily block memory. The address is added when 
       * sending the DIO. */
      return 1;
    }
  }
  return 0;
}
/************************************************************************/
uip_ipaddr_t*
rpl_get_own_addr(uint8_t global)
{
  int i;

  for (i = 0; i < UIP_DS6_ADDR_NB; i++) {
    if (uip_ds6_if.addr_list[i].isused &&
       (uip_ds6_if.addr_list[i].state == ADDR_TENTATIVE ||
        uip_ds6_if.addr_list[i].state == ADDR_PREFERRED)) {
      if (global && !uip_is_addr_link_local(&uip_ds6_if.addr_list[i].ipaddr)) {
        return &uip_ds6_if.addr_list[i].ipaddr;
      } else if(!global && uip_is_addr_link_local(&uip_ds6_if.addr_list[i].ipaddr)) {
        return &uip_ds6_if.addr_list[i].ipaddr;
      }
    }
  }
  return NULL;
}
/************************************************************************/
rpl_dag_p2p_t *
rpl_p2p_alloc_dag(rpl_dag_t *dag)
{
  rpl_dag_p2p_t *p2p;
  rpl_dag_p2p_t *end;
  uint8_t i;

  for (p2p = &dag_p2p_table[0], end = p2p + RPL_MAX_DAG_P2P_ENTRIES; p2p < end; p2p++) {
    if (p2p->dag == NULL || p2p->dag->used == 0) {
      memset(p2p, 0, sizeof(*p2p));
      for(i = 0; i < RPL_P2P_MAX_PARTIAL_ROUTES; i++) {
        p2p->partial_routes[i] = &p2p->partial_routes_lists[i];
        p2p->partial_routes_lists[i] = NULL;
        list_init(p2p->partial_routes[i]);
      }
      p2p->dag = dag;
      PRINTF("RPL: Allocated P2P at %p with DAG at %p\n", p2p, dag);
      latest_p2p_dag = dag;
      return p2p;
    }
  }
  ANNOTATE("#A err=Store0\n");
  return NULL;
}
/************************************************************************/
void
rpl_p2p_free_dag(rpl_dag_p2p_t *p2p)
{
  uint8_t i;

  for(i = 0; i < RPL_P2P_MAX_PARTIAL_ROUTES; i++) {
    free_addr_list(p2p->partial_routes[i]);
  }
  /* Free the DAG */
  p2p->dag->p2p = NULL;
  rpl_free_dag(p2p->dag);
  p2p->dag = NULL;
}
/************************************************************************/
void
rpl_p2p_purge_dags(void)
{
  rpl_dag_p2p_t *p2p;
  rpl_dag_p2p_t *end;

  for (p2p = &dag_p2p_table[0], end = p2p + RPL_MAX_DAG_P2P_ENTRIES; p2p < end; p2p++) {
    if (p2p->dag != NULL && p2p->dag->used) {
#if 0
      printf("RPL: Lifetime of P2P DAG ");
      print6addr(&p2p->dag->dag_id);
      printf(" is %d sec\n", p2p->lifetime);
#endif
      if (p2p->lifetime <= 1 && p2p->expired) {
        PRINTF("RPL: Freeing expired P2P DAG ");
        PRINT6ADDR(&p2p->dag->dag_id);
        PRINTF("\n");
        rpl_p2p_free_dag(p2p);
      }
      else if (p2p->lifetime <= 1 && !p2p->expired) {
        PRINTF("RPL: Lifetime of P2P DAG ");
        PRINT6ADDR(&p2p->dag->dag_id);
        PRINTF(" expired\n");
        /* Stop sending DIOs for this DAG but still keep it in memory for some
         * time in order to ignore DIOs from neighbors whose lifetime is not yet
         * expired.
         * XXX This timeout has to be chosen carefully because if we receive a
         * DIO for this DAG after we deleted it, the DAG object will be created
         * again and new DIOs are emmitted, which will keep the DAG alive on the
         * other nodes and so on.
         * A save value should be the minimum lifetime (which is set upon receiving a
         * DIO) plus some fixed propagation time. */
        p2p->lifetime = rpl_p2p_lt2sec(p2p->rdo_lifetime) + 10;
        p2p->expired = 1;
        ctimer_stop(&p2p->dag->dio_timer);
        /* Parent memory is shared betwee DAGs. Since we only keep the DAG to
         * detect and ignore corresponding DIOs, we can safely free the parents. */
        rpl_remove_parents(p2p->dag, 0);
#if (DEBUG) & DEBUG_ANNOTATE
        rpl_repaint_links();
#endif /* (DEBUG) & DEBUG_ANNOTATE */
#ifdef SENSLAB
        printf("P2P DAG ");
        print6addr(&p2p->dag->dag_id);
        printf(" 0x%02x EXPIRED\n", p2p->dag->instance_id);
#endif /* SENSLAB */
      }
      else {
        p2p->lifetime--;
      }
    }
  }
}
/************************************************************************/
uint16_t
rpl_p2p_lt2sec(uint8_t lifetime)
{
  switch (lifetime) {
    case RPL_P2P_DAG_LIFETIME_1_SEC:
      return 1;
    case RPL_P2P_DAG_LIFETIME_4_SEC:
      return 4;
    case RPL_P2P_DAG_LIFETIME_16_SEC:
      return 16;
    case RPL_P2P_DAG_LIFETIME_64_SEC:
      return 64;
    default:
      return 0;
  }
}
/************************************************************************/
uint8_t
rpl_discover_route(uip_ipaddr_t *target)
{

  /* create temporary DAG */
  rpl_dag_t *dag;
  uint8_t instance_id;

  /* Increase the local RPLInstanceID each time we issue a new request. */
  instance = (instance + 1) % 0x40;
  instance_id = RPL_LOCAL_DEFAULT_INSTANCE | instance;

  dag = rpl_get_dag(instance_id, rpl_get_own_addr(1));
  if(dag != NULL) {
    PRINTF("RPL: Dropping a local DAG when starting route discovery");
    rpl_free_dag(dag);
  }
  dag = rpl_alloc_dag(instance_id);
  if(dag == NULL) {
    PRINTF("RPL: Failed to allocate a DAG\n");
#ifdef SENSLAB
    puts("ERR MEM dag");
#endif /* SENSLAB */
    return 0;
  }

  dag->p2p = rpl_p2p_alloc_dag(dag);
  if (dag->p2p == NULL) {
    PRINTF("RPL: Failed to allocate P2P extension for DAG\n");
#ifdef SENSLAB
    puts("ERR MEM p2p");
#endif /* SENSLAB */
    rpl_free_dag(dag);
    return 0;
  }

  dag->used = 1;
  dag->joined = 1;
  dag->version = 0;
  dag->preferred_parent = NULL;
  /* set P2P specific DAG configuration options */
  dag->grounded = 0;
  dag->mop = RPL_MOP_P2P_ROUTE_DISCOVERY;
  dag->dtsn_out = 0;
  dag->preference = 0;
  memcpy(&dag->dag_id, rpl_get_own_addr(1), sizeof(dag->dag_id));
  rpl_set_dodag_config(dag, &P2P_DEFAULT_DODAG_CONFIG);
  /* The ROOT_RANK macro relies on the configuration option being set! */
  dag->rank = ROOT_RANK(dag);
  dag->of->update_metric_container(dag);

  /* additional attributes needed for the P2P routes */
  dag->p2p->process = PROCESS_CURRENT();
  dag->p2p->both_directions = RPL_P2P_DEFAULT_BOTH_DIRECTIONS;
  dag->p2p->hop_by_hop = RPL_P2P_DEFAULT_HOP_BY_HOP;
  dag->p2p->num_routes = RPL_P2P_DEFAULT_NUM_ROUTES;
  /* Lifetime as specified in RDO */
  dag->p2p->rdo_lifetime = RPL_P2P_DEFAULT_DAG_LIFETIME;
  dag->p2p->max_rank = RPL_P2P_DEFAULT_MAX_RANK;
  /* Remaining lifetime of the DAG in seconds */
  dag->p2p->lifetime = rpl_p2p_lt2sec(dag->p2p->rdo_lifetime);
  dag->p2p->expired = 0;
  /* Compression of addresses in RDO target and address vector */
  dag->p2p->compr = RPL_P2P_RDO_COMPR;
  /* Compress target */
  memcpy(&dag->p2p->target, ((uint8_t *)target) + dag->p2p->compr, 16 - dag->p2p->compr);

  /* Route discovery DIO may travel as long as this constranint is met */
  dag->p2p->constr.type = dag->mc.type;
  dag->p2p->constr.flags = RPL_DAG_MC_FLAG_C;
  dag->p2p->constr.aggr = 0;
  /* Equals MAX_PATH_COST in OF ETX i.e. no constraint */
  dag->p2p->constr.obj.etx = 100 * RPL_DAG_MC_ETX_DIVISOR;

  ANNOTATE("#A p2p=?-%u\n", target->u8[15]);
  PRINTF("RPL: Initialized DAG with instance ID 0x%02x for route-discovery to ", dag->instance_id);
  PRINT6ADDR(target);
  PRINTF("\n");
#if (DEBUG) & DEBUG_ANNOTATE
  rpl_repaint_links();
#endif /* (DEBUG) & DEBUG_ANNOTATE */
#ifdef SENSLAB
  printf("P2P DAG ");
  print6addr(&dag->dag_id);
  printf(" 0x%02x DISCOVER ", dag->instance_id);
  print6addr(target);
  putchar('\n');
#endif /* SENSLAB */

  /* send DIO */
  rpl_reset_dio_timer(dag, 1);

  return 1;
}
/************************************************************************/
uint8_t
rpl_p2p_meets_constraints(rpl_metric_container_t *metric, rpl_metric_container_t *constr)
{
  if (constr == NULL) {
    /* no constraint, whatever metric is fine */
    return 1;
  }
  if (metric->type != constr->type) {
    PRINTF("RPL: Unable to check constraint, metric not matching\n");
    return 0;
  }
  if (metric->type == RPL_DAG_MC_ETX) {
    PRINTF("RPL: metric: %d, P2P constraint: %d\n", metric->obj.etx/RPL_DAG_MC_ETX_DIVISOR, constr->obj.etx/RPL_DAG_MC_ETX_DIVISOR);
    return (metric->obj.etx <= constr->obj.etx);
  }
  if (metric->type == RPL_DAG_MC_ENERGY) {
    /* TODO implement some policy here */
  }
  return 1;
}
#if (DEBUG) & DEBUG_ANNOTATE
/************************************************************************/
void
rpl_annotate_rdo(uint8_t *rdo, uip_ipaddr_t *dodag_id)
{
  ANNOTATE("#A p2p=");
  rpl_print_rdo(rdo, dodag_id);
  putchar('\n');
}
#endif /* (DEBUG) & DEBUG_ANNOTATE */
#if (DEBUG) & DEBUG_ANNOTATE || defined(SENSLAB)
/************************************************************************/
void
rpl_print_rdo(uint8_t *rdo, uip_ipaddr_t *dodag_id)
{
  uint8_t first, last;
  uint8_t *a;
  int a_add;

  uint8_t compr = RPL_BITMASK_READ(P2P_RDO_C, *(rdo + 2));
  uint8_t *a_first = rdo + 4 + 16 - compr;
  uint8_t *a_last = rdo + 2  + *(rdo + 1) - 16 + compr;

  if(uip_ds6_is_my_addr(dodag_id)) {
    /* We are the origin, traverse forward */
    first = dodag_id->u8[15];
    last = *(rdo + 4 + 16 - compr - 1);
    a = a_first;
    a_add = 16 - compr;
  } else {
    /* We are an intermediate node or the target, traverse backward. */
    first = rpl_get_own_addr(1)->u8[15];
    last = dodag_id->u8[15];
    a = a_last;
    a_add = -(16 - compr);
  }
  printf("%u-", first);
  for(; a >= a_first && a <= a_last; a += a_add) {
    printf("%u-", *(a + 16 - compr - 1));
  }
  printf("%u", last);
}
#endif /* (DEBUG) & DEBUG_ANNOTATE || defined(SENSLAB) */
/************************************************************************/
void
rpl_p2p_update_dag(rpl_dag_t *dag, rpl_dio_t *dio)
{
  uint8_t i;

  PRINTF("RPL: Updating P2P DAG\n");
  /*uint8_t offset = 4 + 16 - dio->rdo->compr;
  print_buffer(dio->rdo->addr_start - offset, dio->rdo->addr_len + offset);*/

  /* The metric options have already been fully created upon parsing the DIO, so
   * just copy them. */
  if(dio->constr != NULL) {
    memcpy(&dag->p2p->constr, dio->constr, sizeof(rpl_metric_container_t));
  } else {
    dag->p2p->constr.type = RPL_DAG_MC_NONE;
  }

  /* Copy values from the route discovery option */
  dag->p2p->both_directions = dio->rdo->both_directions;
  dag->p2p->num_routes = dio->rdo->num_routes;
  dag->p2p->compr = dio->rdo->compr;
  dag->p2p->hop_by_hop = dio->rdo->hop_by_hop;
  dag->p2p->rdo_lifetime = dio->rdo->lifetime;
  dag->p2p->max_rank = dio->rdo->max_rank;
  /* Copy target */
  memcpy(&dag->p2p->target, dio->rdo->target, 16 - dio->rdo->compr);

  /* Delete all current partial routes and free memory */
  for(i = 0; i < RPL_P2P_MAX_PARTIAL_ROUTES; i++) {
    free_addr_list(dag->p2p->partial_routes[i]);
  }
  /* Store route advertised in the DIO */
  printf("P2P: Partial route ");
  rpl_print_rdo(dio->rdo->start, &dio->dag_id);
  if(rpl_p2p_add_partial_route(dag, dio)) {
    printf(" cached\n");
  } else {
    printf(" NOT cached\n");
  }
}
/************************************************************************/
static void
send_dros(void *data)
{
  rpl_dag_t *dag = (rpl_dag_t *)data;
  uip_ds6_src_route_t *src_route;
  rpl_src_route_addr_t *a;
  uip_ipaddr_t addr;
  uint8_t i, j;
  list_t addr_list;
  void *empty_list = NULL;


  /* Send as many DROs as routes were requested, if so many are available */
  for(i = 0, j = 0; i < RPL_P2P_MAX_PARTIAL_ROUTES && j <= dag->p2p->num_routes; i++) {
    addr_list = dag->p2p->partial_routes[i];
    /* Ignore empty address lists */
    if(list_length(addr_list) == 0) {
      continue;
    }
    if(list_length(addr_list) == 1 &&
       memcmp(&((rpl_src_route_addr_t *)list_head(addr_list))->addr, ((uint8_t *)&dag->dag_id) + dag->p2p->compr, 16 - dag->p2p->compr) == 0) {
      /* Treat as empty address list */
      addr_list = &empty_list;
    }

    /* Store the backward route if necessary */
    if(j == 0 && dag->p2p->both_directions && RPL_P2P_STORE_BACKWARD_ROUTES) {
      src_route = uip_ds6_src_route_add(&dag->dag_id);
      if(src_route == NULL) {
        PRINTF("RPL: Unable to store source route\n");
        ANNOTATE("#A err=Store1\n");
#ifdef SENSLAB
        puts("ERR MEM srcrt");
#endif /* SENSLAB */
      } else {
        src_route->state.lifetime = (unsigned long)(dag->config.default_lifetime * dag->config.lifetime_unit);
        for(a = list_head(addr_list); a != NULL; a = list_item_next(a)) {
          /* Decompress address */
          memcpy(&addr, &dag->dag_id, dag->p2p->compr);
          memcpy(((uint8_t *)&addr) + dag->p2p->compr, &a->addr, 16 - dag->p2p->compr);
          /* XXX This is a hack! we need link local addresses in the source route,
           * since neighbor discovery is not implemented or does not work. */
          uip_create_linklocal_prefix(&addr);
          /* Add intermediate hops */
          if(uip_ds6_src_route_hop_add(src_route, &addr, 1) == NULL) {
            uip_ds6_src_route_rm(src_route);
            src_route = NULL;
            PRINTF("RPL: Unable to store all addresses of source route\n");
            ANNOTATE("#A err=Store2\n");
#ifdef SENSLAB
            puts("ERR MEM srcrt entry");
#endif /* SENSLAB */
            break;
          }
        }
        if(src_route != NULL) {
          /* Add the link local address of the origin at the very end. */
          memcpy(&addr, &dag->dag_id, 16);
          /* XXX This is a hack! we need link local addresses in the source route,
           * since neighbor discovery is not implemented or does not work. */
          uip_create_linklocal_prefix(&addr);
          if(uip_ds6_src_route_hop_add(src_route, &addr, 0) == NULL) {
            uip_ds6_src_route_rm(src_route);
            PRINTF("RPL: Unable to store last address of source route\n");
            ANNOTATE("#A err=Store2\n");
  #ifdef SENSLAB
            puts("ERR MEM srcrt entry");
  #endif /* SENSLAB */
          }
        }
      }
    }
    /* Send DRO for this route */
    dro_output(dag, addr_list);
    j++;
  }
}
/************************************************************************/
void
rpl_p2p_process_target(rpl_dag_t *dag, rpl_dio_t *dio)
{
  if(!rpl_p2p_is_target(dag->p2p)) {
    PRINTF("RPL: We are not the target!\n");
    return;
  }
  /* Stop the DIO timer. */
  ctimer_stop(&dag->dio_timer);

  if(dag->p2p->dro_timer.f != NULL) {
    PRINTF("RPL: DRO timer already started!\n");
    return;
  }
  /* Start the DRO timer. */
  ctimer_set(&dag->p2p->dro_timer, RPL_P2P_DRO_WAIT_TIME, send_dros, dag);  

  PRINTF("----> RPL: Route discovery reached target <----\n");
#if (DEBUG) & DEBUG_ANNOTATE
  rpl_annotate_rdo(dio->rdo->start, &dio->dag_id);
#endif /* (DEBUG) & DEBUG_ANNOTATE */
#ifdef SENSLAB
  printf("P2P DAG ");
  print6addr(&dio->dag_id);
  printf(" 0x%02x REACHED ", dio->instance_id);
  rpl_print_rdo(dio->rdo->start, &dio->dag_id);
  putchar('\n');
#endif /* SENSLAB */
}
/************************************************************************/
uip_ds6_src_route_t *
rpl_p2p_add_src_route(uip_ipaddr_t *dest, uint8_t *addr_start, uint8_t addr_len, uint8_t compr, uint8_t reverse)
{
  uip_ds6_src_route_t *src_route;
  uint8_t i;
  uip_ipaddr_t addr;

  src_route = uip_ds6_src_route_add(dest);
  if(src_route == NULL) {
    PRINTF("RPL: Unable to store source route\n");
    ANNOTATE("#A err=Store1\n");
#ifdef SENSLAB
    puts("ERR MEM srcrt");
#endif /* SENSLAB */
    return NULL;
  }
  /* TODO take lifetime from DAG configuration */
  src_route->state.lifetime = (unsigned long)(RPL_DEFAULT_LIFETIME * RPL_DEFAULT_LIFETIME_UNIT);
  for(i = 0; i < addr_len; i += 16 - compr) {
    /* Decompress address */
    memcpy(&addr, dest, compr);
    memcpy(((uint8_t *)&addr) + compr, addr_start + i, 16 - compr);
    /* XXX This is a hack! we need link local addresses in the source route,
     * since neighbor discovery is not implemented or does not work. */
    uip_create_linklocal_prefix(&addr);
    /* Add intermediate hops in correct order (depending on whether this is
     * called on the origin after receiving a DRO or on the target after receiving
     * a DIO) */
    if(uip_ds6_src_route_hop_add(src_route, &addr, reverse) == NULL) {
      uip_ds6_src_route_rm(src_route);
      PRINTF("RPL: Unable to store all addresses of source route\n");
      ANNOTATE("#A err=Store2\n");
#ifdef SENSLAB
      puts("ERR MEM srcrt entry");
#endif /* SENSLAB */
      return NULL;
    }
  }
  /* Add the link local address of the target at the very end. */
  /* XXX This is a hack! we need link local addresses in the source route,
   * since neighbor discovery is not implemented or does not work. */
  memcpy(&addr, dest, 16);
  uip_create_linklocal_prefix(&addr);
  if(uip_ds6_src_route_hop_add(src_route, &addr, 0) == NULL) {
    uip_ds6_src_route_rm(src_route);
    PRINTF("RPL: Unable to store last address of source route\n");
    ANNOTATE("#A err=Store2\n");
#ifdef SENSLAB
    puts("ERR MEM srcrt entry");
#endif /* SENSLAB */
    return NULL;
  }
  return src_route;
}
/************************************************************************/
uip_ds6_route_t *
rpl_p2p_add_hbh_route(uint8_t instance, uip_ipaddr_t *dodagid, uip_ipaddr_t *target, uip_ipaddr_t *nexthop)
{
  uip_ds6_route_t *hbhrt;
  rpl_dag_t *dag;

  dag = rpl_get_dag(instance, dodagid);
  hbhrt = rpl_add_route(dag, target, 128, nexthop);
  if(hbhrt != NULL) {
    hbhrt->state.instance = instance;
    memcpy(&hbhrt->state.dodagid, dodagid, 16);
  }
  return hbhrt;
}
/************************************************************************/
uint8_t
rpl_p2p_route_differs(rpl_dag_t *dag, rpl_dio_t *dio)
{
  uint8_t i, j;
  rpl_src_route_addr_t *a;

  /* DAG and DIO must belong to the same discovery, i.e. share the same origin
   * and target. */
  for(i = 0; i < RPL_P2P_MAX_PARTIAL_ROUTES; i++) {
    /* Ignore empty address lists */
    if(list_length(dag->p2p->partial_routes[i]) == 0) {
      continue;
    }
    /* In the special case where the address vector is empty, a matching route 
     * contains the DODAGID as only hop. */
    if(dio->rdo->addr_len == 0) {
      if(list_length(dag->p2p->partial_routes[i]) != 1 ||
         memcmp(&((rpl_src_route_addr_t *)list_head(dag->p2p->partial_routes[i]))->addr, ((uint8_t *)&dag->dag_id) + dio->rdo->compr, 16 - dio->rdo->compr) != 0) {
        continue;
      } else {
        return 0;
      }
    }
    /* Route differs if length differs */
    if(list_length(dag->p2p->partial_routes[i]) != dio->rdo->addr_len) {
      continue;
    }
    /* Compare intermediate hops */
    for(j = 0, a = list_head(dag->p2p->partial_routes[i]);
        j < dio->rdo->addr_len && a != NULL;
        j += 16 - dio->rdo->compr, a = list_item_next(a)) {
      if(memcmp(dio->rdo->addr_start + j, &a->addr, 16 - dio->rdo->compr) != 0) {
        break;
      }
    }
    if(a == NULL) {
      return 0;
    }
  }
  return 1;
}
