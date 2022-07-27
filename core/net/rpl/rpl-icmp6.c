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
 *         ICMP6 I/O for RPL control messages.
 *
 * \author Joakim Eriksson <joakime@sics.se>, Nicolas Tsiftes <nvt@sics.se>
 * Contributors: Niclas Finne <nfi@sics.se>, Joel Hoglund <joel@sics.se>,
 *               Mathieu Pouillot <m.pouillot@watteco.com>, Matthias Philipp
 *               <matthias-philipp@gmx.de>
 */

#include "net/tcpip.h"
#include "net/uip.h"
#include "net/uip-ds6.h"
#include "net/uip-nd6.h"
#include "net/uip-icmp6.h"
#include "net/rpl/rpl-private.h"
#include "net/packetbuf.h"

#include <limits.h>
#include <string.h>

#define DEBUG DEBUG_NONE
#include "net/uip-debug.h"

#if WITH_RPL_P2P
#include "net/rpl/rpl-p2p.h"
#include "lib/memb.h"
#include "lib/list.h"
#include "sys/ctimer.h"
#include "sys/process.h"
#include "lib/random.h"
#if WITH_RPL_P2P_MEASUREMENT
#include "net/neighbor-info.h"
#define NI_ETX_TO_RPL_ETX(etx)						\
	((etx) * (RPL_DAG_MC_ETX_DIVISOR / NEIGHBOR_INFO_ETX_DIVISOR))
#endif /* WITH_RPL_P2P_MEASUREMENT */
#endif /* WITH_RPL_P2P */

/*---------------------------------------------------------------------------*/
#define RPL_DIO_GROUNDED                 0x80
#define RPL_DIO_MOP_SHIFT                3
#define RPL_DIO_MOP_MASK                 0x38
#define RPL_DIO_PREFERENCE_MASK          0x07

#define UIP_IP_BUF       ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UIP_ICMP_BUF     ((struct uip_icmp_hdr *)&uip_buf[uip_l2_l3_hdr_len])
#define UIP_ICMP_PAYLOAD ((unsigned char *)&uip_buf[uip_l2_l3_icmp_hdr_len])
/*---------------------------------------------------------------------------*/
static void dis_input(void);
static void dio_input(void);
static uint8_t rpl_send_uip_buf = 0;
#if RPL_MOP_DEFAULT == RPL_MOP_STORING_NO_MULTICAST || RPL_MOP_DEFAULT == RPL_MOP_NON_STORING
static void dao_input(void);
static void dao_ack_input(void);

static uint8_t dao_sequence;
/*---------------------------------------------------------------------------*/
static int
get_global_addr(uip_ipaddr_t *addr)
{
  int i;
  int state;

  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(uip_ds6_if.addr_list[i].isused &&
       (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
      if(!uip_is_addr_link_local(&uip_ds6_if.addr_list[i].ipaddr)) {
        memcpy(addr, &uip_ds6_if.addr_list[i].ipaddr, sizeof(uip_ipaddr_t));
        return 1;
      }
    }
  }
  return 0;
}
#endif /* RPL_MOP_DEFAULT == RPL_MOP_STORING_NO_MULTICAST || RPL_MOP_DEFAULT == RPL_MOP_NON_STORING */
/*---------------------------------------------------------------------------*/
//#if (DEBUG) & DEBUG_PRINT
void
print_buffer(uint8_t *buf, uint8_t len)
{
  int i;
  for (i = 0; i < len; i++) {
    printf("0x%02x ", *buf);
    buf++;
    if ((i + 1) % 4 == 0) printf("\n");
  }
  printf("\n");
}
//#endif /* (DEBUG) & DEBUG_PRINT */
/*---------------------------------------------------------------------------*/
static uint32_t
get32(uint8_t *buffer, int pos)
{
  return (uint32_t)buffer[pos] << 24 | (uint32_t)buffer[pos + 1] << 16 |
         (uint32_t)buffer[pos + 2] << 8 | buffer[pos + 3];
}
/*---------------------------------------------------------------------------*/
static void
set32(uint8_t *buffer, int pos, uint32_t value)
{
  buffer[pos++] = value >> 24;
  buffer[pos++] = (value >> 16) & 0xff;
  buffer[pos++] = (value >> 8) & 0xff;
  buffer[pos++] = value & 0xff;
}
/*---------------------------------------------------------------------------*/
static uint16_t
get16(uint8_t *buffer, int pos)
{
  return (uint16_t)buffer[pos] << 8 | buffer[pos + 1];
}
/*---------------------------------------------------------------------------*/
static void
set16(uint8_t *buffer, int pos, uint16_t value)
{
  buffer[pos++] = value >> 8;
  buffer[pos++] = value & 0xff;
}
/*---------------------------------------------------------------------------*/
static void
dis_input(void)
{
  rpl_dag_t *dag;

  /* DAG Information Solicitation */
  PRINTF("RPL: Received a DIS from ");
  PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
  PRINTF("\n");

  dag = rpl_get_dag(RPL_ANY_INSTANCE, NULL);
  if(dag != NULL) {
    if(uip_is_addr_mcast(&UIP_IP_BUF->destipaddr)) {
      PRINTF("RPL: Multicast DIS => reset DIO timer\n");
      rpl_reset_dio_timer(dag, 0);
    } else {
      PRINTF("RPL: Unicast DIS, reply to sender\n");
      dio_output(dag, &UIP_IP_BUF->srcipaddr);
    }
  }
}
/*---------------------------------------------------------------------------*/
void
dis_output(uip_ipaddr_t *addr)
{
  unsigned char *buffer;
  uip_ipaddr_t tmpaddr;

  /* DAG Information Solicitation  - 2 bytes reserved */
  /*      0                   1                   2        */
  /*      0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3  */
  /*     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
  /*     |     Flags     |   Reserved    |   Option(s)...  */
  /*     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */

  buffer = UIP_ICMP_PAYLOAD;
  buffer[0] = buffer[1] = 0;

  if(addr == NULL) {
    PRINTF("RPL: Sending a DIS\n");
    uip_create_linklocal_rplnodes_mcast(&tmpaddr);
    addr = &tmpaddr;
  } else {
    PRINTF("RPL: Sending a unicast DIS\n");
  }
#ifdef SENSLAB
  printf("RPL DIS 2 Bytes\n");
#endif /* SENSLAB */
  uip_icmp6_send(addr, ICMP6_RPL, RPL_CODE_DIS, 2);
}
/*---------------------------------------------------------------------------*/
static void
dio_input(void)
{
  unsigned char *buffer;
  uint8_t buffer_length;
  rpl_dio_t dio;
  uint8_t subopt_type;
  int i;
  int len;
  uip_ipaddr_t from;
  int j, mc_len;
  rpl_prefix_t rif;
  rpl_dodag_config_t dcf;
  rpl_prefix_t pif;
#if WITH_RPL_P2P
  rpl_dio_rdo_t rdo;
#endif /* SENSLAB */
 
  memset(&dio, 0, sizeof(dio));
  uip_ipaddr_copy(&from, &UIP_IP_BUF->srcipaddr);

  /* DAG Information Object */
  PRINTF("RPL: Received a DIO from ");
  PRINT6ADDR(&from);
  PRINTF("\n");

  buffer_length = uip_len - uip_l2_l3_icmp_hdr_len;

#if RPL_CONF_ADJUST_LLH_LEN
  buffer_length+=UIP_LLH_LEN; //Add jackdaw, minimal-net ethernet header
#endif

  /* Process the DIO base option. */
  i = 0;
  buffer = UIP_ICMP_PAYLOAD;

  //print_buffer(buffer, buffer_length);

  dio.instance_id = buffer[i++];
  dio.version = buffer[i++];
  dio.rank = get16(buffer, i);
  i += 2;

  PRINTF("RPL: Incoming DIO rank %u\n", (unsigned)dio.rank);

  dio.grounded = buffer[i] & RPL_DIO_GROUNDED;
  dio.mop = (buffer[i]& RPL_DIO_MOP_MASK) >> RPL_DIO_MOP_SHIFT;
  dio.preference = buffer[i++] & RPL_DIO_PREFERENCE_MASK;

  dio.dtsn = buffer[i++];
  /* two reserved bytes */
  i += 2;

  memcpy(&dio.dag_id, buffer + i, sizeof(dio.dag_id));
  i += sizeof(dio.dag_id);

#ifdef SENSLAB
  /* XXX OK here it gets dirty! This should never occur in the first place!
   * It still has to be investigated where these DIOs come from. It might be
   * either defective nodes in the testbed, or rather due to stack overflow on
   * the processing node. If there are DIOs with invalid IDs it is very likely
   * that there are also DIOs with corrupted data in less obvoius fields. */
  if(dio.dag_id.u16[0] != 0xaaaa || dio.dag_id.u16[1] != 0 || dio.dag_id.u16[2] != 0 || dio.dag_id.u16[3] != 0 ||
     dio.dag_id.u16[4] != 0 || dio.dag_id.u16[5] != 0 || dio.dag_id.u16[6] != 0 || dio.dag_id.u8[14] != 0) {
    puts("ERR DIO bad id");
    return;
  }
#endif /* SENSLAB */

  dio.dodag_config = NULL;
  dio.route_info = NULL;
  dio.prefix_info = NULL;
  dio.mc.type = RPL_DAG_MC_NONE;
#if WITH_RPL_P2P
  /* Additional options for P2P extension */
  dio.rdo = NULL;
  dio.constr = NULL;
#endif /* WITH_RPL_P2P */

  /* Check if there are any DIO suboptions. */
  for(; i < buffer_length; i += len) {
    subopt_type = buffer[i];
    if(subopt_type == RPL_OPTION_PAD1) {
      len = 1;
    } else {
      /* Suboption with a two-byte header + payload */
      len = 2 + buffer[i + 1];
    }

    if(len + i > buffer_length) {
      PRINTF("RPL: Invalid DIO packet\n");
      RPL_STAT(rpl_stats.malformed_msgs++);
      return;
    }

    PRINTF("RPL: DIO option %u, length: %u\n", subopt_type, len - 2);

    switch(subopt_type) {
    case RPL_OPTION_DAG_METRIC_CONTAINER:
      if(len < 6) {
        PRINTF("RPL: Invalid DAG MC, len = %d\n", len);
        RPL_STAT(rpl_stats.malformed_msgs++);
        return;
      }
      /* One metric option may have multiple metric containers */
      for(j = i + 2; j < i + len; j += mc_len) {
        mc_len = 4 + buffer[j + 3];
        if((buffer[j + 1] << 1) & RPL_DAG_MC_FLAG_C) {
          /* Constraint object (only evaluated for P2P extension) */
#if WITH_RPL_P2P
          rpl_metric_container_t constr;
          dio.constr = &constr;
          constr.type = buffer[j];
          constr.flags = buffer[j + 1] << 1;
          constr.flags |= buffer[j + 2] >> 7;
          constr.aggr = (buffer[j + 2] >> 4) & 0x3;
          constr.prec = buffer[j + 2] & 0xf;
          constr.length = buffer[j + 3];

          if(constr.type == RPL_DAG_MC_ETX) {
            constr.obj.etx = buffer[j + 4] << 8;
            constr.obj.etx |= buffer[j + 5];
          } else {
           PRINTF("RPL: Unhandled DAG MC type: %u\n", (unsigned)constr.type);
           return;
          }
#endif /* WITH_RPL_P2P */
        } else {
          /* Metric object */
          dio.mc.type = buffer[j];
          dio.mc.flags = buffer[j + 1] << 1;
          dio.mc.flags |= buffer[j + 2] >> 7;
          dio.mc.aggr = (buffer[j + 2] >> 4) & 0x3;
          dio.mc.prec = buffer[j + 2] & 0xf;
          dio.mc.length = buffer[j + 3];

          if(dio.mc.type == RPL_DAG_MC_ETX) {
            dio.mc.obj.etx = get16(buffer, j + 4);
            PRINTF("RPL: DAG MC: type %u, flags %u, aggr %u, prec %u, length %u, ETX %u\n",
             (unsigned)dio.mc.type,
             (unsigned)dio.mc.flags,
             (unsigned)dio.mc.aggr,
             (unsigned)dio.mc.prec,
             (unsigned)dio.mc.length,
             (unsigned)dio.mc.obj.etx);
          } else if(dio.mc.type == RPL_DAG_MC_ENERGY) {
            dio.mc.obj.energy.flags = buffer[j + 4];
            dio.mc.obj.energy.energy_est = buffer[j + 5];
          } else {
           PRINTF("RPL: Unhandled DAG MC type: %u\n", (unsigned)dio.mc.type);
           return;
          }
        }
      }
      break;
    case RPL_OPTION_ROUTE_INFO:
      if(len < 9) {
        PRINTF("RPL: Invalid destination prefix option, len = %d\n", len);
        RPL_STAT(rpl_stats.malformed_msgs++);
        return;
      }

      /* flags is both preference and flags for now */
      dio.route_info = &rif;
      rif.length = buffer[i + 2];
      rif.flags = buffer[i + 3];
      rif.lifetime = get32(buffer, i + 4);

      if(((rif.length + 7) / 8) + 8 <= len && rif.length <= 128) {
        PRINTF("RPL: Copying destination prefix\n");
        memcpy(&rif.prefix, &buffer[i + 8], (rif.length + 7) / 8);
      } else {
        PRINTF("RPL: Invalid route infoprefix option, len = %d\n", len);
        RPL_STAT(rpl_stats.malformed_msgs++);
        return;
      }

      break;
    case RPL_OPTION_DAG_CONF:
      if(len != 16) {
        PRINTF("RPL: Invalid DAG configuration option, len = %d\n", len);
        RPL_STAT(rpl_stats.malformed_msgs++);
        return;
      }
      dio.dodag_config = &dcf;
      /* Path control field not yet implemented - at i + 2 */
      dcf.dio_intdoubl = buffer[i + 3];
      dcf.dio_intmin = buffer[i + 4];
      dcf.dio_redund = buffer[i + 5];
      dcf.max_rankinc = get16(buffer, i + 6);
      dcf.min_hoprankinc = get16(buffer, i + 8);
      dcf.ocp = get16(buffer, i + 10);
      /* buffer + 12 is reserved */
      dcf.default_lifetime = buffer[i + 13];
      dcf.lifetime_unit = get16(buffer, i + 14);
      PRINTF("RPL: DIO Conf:dbl=%d, min=%d red=%d maxinc=%d mininc=%d ocp=%d d_l=%u l_u=%u\n",
          dcf.dio_intdoubl, dcf.dio_intmin, dcf.dio_redund, dcf.max_rankinc,
          dcf.min_hoprankinc, dcf.ocp, dcf.default_lifetime,
          dcf.lifetime_unit);
      break;
    case RPL_OPTION_PREFIX_INFO:
      if(len != 32) {
        PRINTF("RPL: DAG Prefix info not ok, len != 32\n");
        RPL_STAT(rpl_stats.malformed_msgs++);
        return;
      }
      dio.prefix_info = &pif;
      pif.length = buffer[i + 2];
      pif.flags = buffer[i + 3];
      /* valid lifetime is ingnored for now - at i + 4 */
      /* preferred lifetime stored in lifetime */
      pif.lifetime = get32(buffer, i + 8);
      /* 32-bit reserved at i + 12 */
      PRINTF("RPL: Copying prefix information\n");
      memcpy(&pif.prefix, &buffer[i + 16], 16);
      break;
#if WITH_RPL_P2P
    case RPL_DIO_SUBOPT_ROUTE_DISCOVERY:
      if(len <= 4) {
        PRINTF("RPL: Unrecognized route discovery option\n");
        RPL_STAT(rpl_stats.malformed_msgs++);
        return;
      }
      if(dio.rdo != NULL) {
        PRINTF("RPL: Multiple route discovery options\n");
        RPL_STAT(rpl_stats.malformed_msgs++);
        return;
      }
      if(RPL_BITMASK_READ(P2P_RDO_C, buffer[i + 2]) < RPL_P2P_RDO_COMPR_MIN) {
        PRINTF("RPL: Unable to handle RDO, compression too low.\n");
        return;
      }
      dio.rdo = &rdo;
      rdo.start = &buffer[i];
      rdo.both_directions = RPL_FLAG_GET(P2P_RDO_D, buffer[i + 2]);
      rdo.hop_by_hop = RPL_FLAG_GET(P2P_RDO_H, buffer[i + 2]);
      rdo.num_routes = RPL_BITMASK_READ(P2P_RDO_N, buffer[i + 2]);
      rdo.compr = RPL_BITMASK_READ(P2P_RDO_C, buffer[i + 2]);
      rdo.lifetime = RPL_BITMASK_READ(P2P_RDO_L, buffer[i + 3]);
      rdo.max_rank = RPL_BITMASK_READ(P2P_RDO_R, buffer[i + 3]);
      rdo.target = &buffer[i + 4];
      rdo.addr_start = &buffer[i + 4 + 16 - rdo.compr];
      /* Ignore empty address slots */
      rdo.addr_len = len - 4 - 16 + rdo.compr;
      break;
#endif /* WITH_RPL_P2P */
    default:
      PRINTF("RPL: Unsupported suboption type in DIO: %u\n",
	(unsigned)subopt_type);
    }
  }

  rpl_process_dio(&from, &dio);
}
/*---------------------------------------------------------------------------*/
static uint8_t
write_rdo(unsigned char *buffer, rpl_dag_t *dag, list_t addr_list)
{
  uip_ds6_src_route_addr_t *a;
  uint8_t pos = 0;

  buffer[pos++] = RPL_DIO_SUBOPT_ROUTE_DISCOVERY;
  buffer[pos] = 2 + (16 - dag->p2p->compr) * (1 + list_length(addr_list));
  /* We add our own address if we are neither the origin nor the target */
  if(!uip_ds6_is_my_addr(&dag->dag_id) && !rpl_p2p_is_target(dag->p2p)) {
    buffer[pos] += 16 - dag->p2p->compr;
  }
  buffer[++pos] = 0;
  if(dag->p2p->both_directions) {
    RPL_FLAG_SET(P2P_RDO_D, buffer[pos]);
  }
  if(dag->p2p->hop_by_hop) {
    RPL_FLAG_SET(P2P_RDO_H, buffer[pos]);
  }
  RPL_BITMASK_WRITE(P2P_RDO_N, buffer[pos], dag->p2p->num_routes);
  RPL_BITMASK_WRITE(P2P_RDO_C, buffer[pos], dag->p2p->compr);
  pos++;
  buffer[pos] = 0;
  RPL_BITMASK_WRITE(P2P_RDO_L, buffer[pos], dag->p2p->rdo_lifetime);
  RPL_BITMASK_WRITE(P2P_RDO_R, buffer[pos], dag->p2p->max_rank);
  pos++;
  /* Add the already compressed target */
  memcpy(&buffer[pos], &dag->p2p->target, 16 - dag->p2p->compr);
  pos += 16 - dag->p2p->compr;
  /* Add the already compressed source route addresses */
  for (a = list_head(addr_list); a != NULL; a = list_item_next(a)) {
    memcpy(&buffer[pos], &a->addr, 16 - dag->p2p->compr);
    pos += 16 - dag->p2p->compr;
  }
  /* Add our own compressed global address if we are neither the origin nor the target. */
  if(!uip_ds6_is_my_addr(&dag->dag_id) && !rpl_p2p_is_target(dag->p2p)) {
    memcpy(&buffer[pos], ((uint8_t *)rpl_get_own_addr(1)) + dag->p2p->compr, 16 - dag->p2p->compr);
    pos += 16 - dag->p2p->compr;
  }
  //print_buffer(buffer, pos);
  return pos;
}
/*---------------------------------------------------------------------------*/
void
dio_output(rpl_dag_t *dag, uip_ipaddr_t *uc_addr)
{
  unsigned char *buffer;
  int pos;
  uip_ipaddr_t addr;
#if WITH_RPL_P2P
  list_t partial_route;
  void *empty_list = NULL;
  uint8_t i, cnt;
  unsigned short r = 0;
#endif /* WITH_RPL_P2P */

#if WITH_RPL_P2P
  if (dag->mop == RPL_MOP_P2P_ROUTE_DISCOVERY) {
    if(dag->p2p->expired) {
      PRINTF("P2P: Suppressing DIO for expired DAG\n");
      return;
    }
    /* Do not send DIO if the rank cannot be increased anyway. */
    if(dag->p2p->max_rank > 0 && DAG_RANK(dag->rank, dag) == dag->p2p->max_rank) {
      printf("P2P: Reached maximum rank, suppressing DIO\n");
      return;
    }
  }
#endif /* WITH_RPL_P2P */
  /* DAG Information Object */
  pos = 0;

  buffer = UIP_ICMP_PAYLOAD;
  buffer[pos++] = dag->instance_id;
  buffer[pos++] = dag->version;
  set16(buffer, pos, dag->rank);
  pos += 2;

  buffer[pos] = 0;
  if(dag->grounded) {
    buffer[pos] |= RPL_DIO_GROUNDED;
  }

  buffer[pos++] = dag->mop << RPL_DIO_MOP_SHIFT;
  /* XXX Why do the Contiki guys increase the DTSN with every DIO? */
#if WITH_RPL_P2P
  if(dag->p2p == NULL) {
#endif /* WITH_RPL_P2P */
    buffer[pos++] = ++dag->dtsn_out;
#if WITH_RPL_P2P
  } else {
    buffer[pos++] = dag->dtsn_out;
  }
#endif /* WITH_RPL_P2P */

  /* reserved 2 bytes */
  buffer[pos++] = 0; /* flags */
  buffer[pos++] = 0; /* reserved */

  memcpy(buffer + pos, &dag->dag_id, sizeof(dag->dag_id));
  pos += 16;

  if(dag->mc.type != RPL_DAG_MC_NONE) {
    dag->of->update_metric_container(dag);
    buffer[pos++] = RPL_OPTION_DAG_METRIC_CONTAINER;
    buffer[pos++] = 6;
    buffer[pos++] = dag->mc.type;
    buffer[pos++] = dag->mc.flags >> 1;
    buffer[pos] = (dag->mc.flags & 1) << 7;
    buffer[pos++] |= (dag->mc.aggr << 4) | dag->mc.prec;

    if(dag->mc.type == RPL_DAG_MC_ETX) {
      buffer[pos++] = 2;
      set16(buffer, pos, dag->mc.obj.etx);
      pos += 2;
    } else if(dag->mc.type == RPL_DAG_MC_ENERGY) {
      buffer[pos++] = 2;
      buffer[pos++] = dag->mc.obj.energy.flags;
      buffer[pos++] = dag->mc.obj.energy.energy_est;
    } else {
      PRINTF("RPL: Unable to send DIO because of unhandled DAG MC type %u\n",
  (unsigned)dag->mc.type);
      return;
    }
  }

#if WITH_RPL_P2P
  /* Add P2P constraint if present */
  if(dag->mop == RPL_MOP_P2P_ROUTE_DISCOVERY && dag->p2p->constr.type != RPL_DAG_MC_NONE) {
    /* Add the metric container option header if not already present */
    if(dag->mc.type == RPL_DAG_MC_NONE) {
      buffer[pos++] = RPL_OPTION_DAG_METRIC_CONTAINER;
      buffer[pos++] = 6;
    } else {
      /* Option header already present, only update the length */
      buffer[pos - 7] += 6;
    }
    buffer[pos++] = dag->p2p->constr.type;
    buffer[pos++] = dag->p2p->constr.flags >> 1;
    buffer[pos] = (dag->p2p->constr.flags  & 1) << 7;
    buffer[pos] = dag->p2p->constr.aggr << 4;
    buffer[pos++] |= dag->p2p->constr.prec;
    if(dag->p2p->constr.type == RPL_DAG_MC_ETX) {
      buffer[pos++] = 2;
      buffer[pos++] = dag->p2p->constr.obj.etx >> 8;
      buffer[pos++] = dag->p2p->constr.obj.etx & 0xff;
    } else {
      PRINTF("RPL: Unable to send DIO because of unhandled DAG MC type %u\n",
  (unsigned)dag->p2p->constr.type);
      return;
    }
  }
#endif /* WITH_RPL_P2P */

  /* FIXME
   * The information communicated in this option is generally static and
   * unchanging within the DODAG, therefore it is not necessary to include in
   * every DIO.
   */
#if 1
  /* Always add a DAG configuration option. */
  buffer[pos++] = RPL_OPTION_DAG_CONF;
  buffer[pos++] = 14;
  buffer[pos++] = 0; /* No Auth, PCS = 0 */
  buffer[pos++] = dag->config.dio_intdoubl;
  buffer[pos++] = dag->config.dio_intmin;
  buffer[pos++] = dag->config.dio_redund;
  set16(buffer, pos, dag->config.max_rankinc);
  pos += 2;
  set16(buffer, pos, dag->config.min_hoprankinc);
  pos += 2;
  /* OCP is in the DAG_CONF option */
  set16(buffer, pos, dag->of->ocp);
  pos += 2;
  buffer[pos++] = 0; /* reserved */
  buffer[pos++] = dag->config.default_lifetime;
  set16(buffer, pos, dag->config.lifetime_unit);
  pos += 2;
#endif

#if WITH_RPL_P2P
  if (dag->mop == RPL_MOP_P2P_ROUTE_DISCOVERY) {
    /* chose a partial route to include in the DIO */
    for(i = 0, cnt = 0; i < RPL_P2P_MAX_PARTIAL_ROUTES; i++) {
      if(list_length(dag->p2p->partial_routes[i]) != 0) {
        cnt++;
      }
    }
    if(cnt > 1) {
      r = random_rand() / (RANDOM_RAND_MAX / cnt);
      printf("P2P: Including partial route %u of %u in DIO\n", r + 1, cnt);
    }
    partial_route = dag->p2p->partial_routes[r];
    if(list_length(partial_route) == 1 &&
       memcmp(&((rpl_src_route_addr_t *)list_head(partial_route))->addr, ((uint8_t *)&dag->dag_id) + dag->p2p->compr, 16 - dag->p2p->compr) == 0) {
      /* treat as empty address list */
      partial_route = &empty_list;
    }
    /* Add the route discovery option. */
    pos += write_rdo(&buffer[pos], dag, partial_route);
  }
  else
#endif /* WITH_RPL_P2P */
  if (dag->prefix_info.length > 0) {
    buffer[pos++] = RPL_OPTION_PREFIX_INFO;
    buffer[pos++] = 30; /* always 30 bytes + 2 long */
    buffer[pos++] = dag->prefix_info.length;
    buffer[pos++] = dag->prefix_info.flags;
    set32(buffer, pos, dag->prefix_info.lifetime);
    pos += 4;
    set32(buffer, pos, dag->prefix_info.lifetime);
    pos += 4;
    memset(&buffer[pos], 0, 4);
    pos += 4;
    memcpy(&buffer[pos], &dag->prefix_info.prefix, 16);
    pos += 16;
    PRINTF("RPL: Sending prefix info in DIO for ");
    PRINT6ADDR(&dag->prefix_info.prefix);
    PRINTF("\n");
  } else {
    PRINTF("RPL: No prefix to announce (len %d)\n",
           dag->prefix_info.length);
  }

#ifdef SENSLAB
  if(dag->instance_id == RPL_DEFAULT_INSTANCE) {
    printf("RPL ");
  } else {
    printf("P2P ");
  }
  printf("DIO ");
  print6addr(&dag->dag_id);
  printf(" 0x%02x rank %u, %u Bytes\n", dag->instance_id, dag->rank, pos);
#endif /* SENSLAB */

  //print_buffer(buffer, pos);
  /* Unicast requests get unicast replies! */
  if(uc_addr == NULL) {
    PRINTF("RPL: Sending a multicast-DIO with rank %u\n",
        (unsigned)dag->rank);
    uip_create_linklocal_rplnodes_mcast(&addr);
    uip_icmp6_send(&addr, ICMP6_RPL, RPL_CODE_DIO, pos);
  } else {
    PRINTF("RPL: Sending unicast-DIO with rank %u to ",
        (unsigned)dag->rank);
    PRINT6ADDR(uc_addr);
    PRINTF("\n");
    uip_icmp6_send(uc_addr, ICMP6_RPL, RPL_CODE_DIO, pos);
  }
}
/*---------------------------------------------------------------------------*/
#if RPL_MOP_DEFAULT == RPL_MOP_STORING_NO_MULTICAST || RPL_MOP_DEFAULT == RPL_MOP_NON_STORING
static void
dao_input(void)
{
  uip_ipaddr_t dao_sender_addr;
  rpl_dag_t *dag;
  unsigned char *buffer;
  uint16_t sequence;
  uint8_t instance_id;
  rpl_lifetime_t lifetime;
  uint8_t prefixlen;
  uint8_t flags;
  uint8_t subopt_type;
  uint8_t pathcontrol;
  uint8_t pathsequence;
  uip_ipaddr_t prefix;
  uip_ds6_route_t *rep;
  uint8_t buffer_length;
  int pos;
  int len;
  int i;
  int learned_from;
  rpl_parent_t *p;

  prefixlen = 0;

  uip_ipaddr_copy(&dao_sender_addr, &UIP_IP_BUF->srcipaddr);

  /* Destination Advertisement Object */
  PRINTF("RPL: Received a DAO from ");
  PRINT6ADDR(&dao_sender_addr);
  PRINTF("\n");

  buffer = UIP_ICMP_PAYLOAD;
  buffer_length = uip_len - uip_l2_l3_icmp_hdr_len;
#if RPL_CONF_ADJUST_LLH_LEN
  buffer_length += UIP_LLH_LEN; /* Add jackdaw, minimal-net ethernet header */
#endif

  pos = 0;
  instance_id = buffer[pos++];

  dag = rpl_get_dag(instance_id, NULL);
  if(dag == NULL) {
    PRINTF("RPL: Ignoring a DAO for a different RPL instance (%u)\n",
           instance_id);
    return;
  }

  lifetime = dag->config.default_lifetime;

  flags = buffer[pos++];
  /* reserved */
  pos++;
  sequence = buffer[pos++];

  /* Is the DAGID present? */
  if(flags & RPL_DAO_D_FLAG) {
    /* Currently the DAG ID is ignored since we only use global
       RPL Instance IDs. */
    pos += 16;
  }

  /* Check if there are any RPL options present. */
  i = pos;
  for(; i < buffer_length; i += len) {
    subopt_type = buffer[i];
    if(subopt_type == RPL_OPTION_PAD1) {
      len = 1;
    } else {
      /* The option consists of a two-byte header and a payload. */
      len = 2 + buffer[i + 1];
    }

    switch(subopt_type) {
    case RPL_OPTION_TARGET:
      /* handle the target option */
      prefixlen = buffer[i + 3];
      memset(&prefix, 0, sizeof(prefix));
      memcpy(&prefix, buffer + i + 4, (prefixlen + 7) / CHAR_BIT);
      break;
    case RPL_OPTION_TRANSIT:
      /* The path sequence and control are ignored. */
      pathcontrol = buffer[i + 3];
      pathsequence = buffer[i + 4];
      lifetime = buffer[i + 5];
      /* The parent address is also ignored. */
      break;
    }
  }

  PRINTF("RPL: DAO lifetime: %u, prefix length: %u prefix: ",
         (unsigned)lifetime, (unsigned)prefixlen);
  PRINT6ADDR(&prefix);
  PRINTF("\n");

  rep = uip_ds6_route_lookup(&prefix);

  if(lifetime == ZERO_LIFETIME) {
    /* No-Path DAO received; invoke the route purging routine. */
    if(rep != NULL && rep->state.saved_lifetime == 0) {
      PRINTF("RPL: Setting expiration timer for prefix ");
      PRINT6ADDR(&prefix);
      PRINTF("\n");
      rep->state.saved_lifetime = rep->state.lifetime;
      rep->state.lifetime = DAO_EXPIRATION_TIMEOUT;
    }
    return;
  }

  learned_from = uip_is_addr_mcast(&dao_sender_addr) ?
                 RPL_ROUTE_FROM_MULTICAST_DAO : RPL_ROUTE_FROM_UNICAST_DAO;

  if(learned_from == RPL_ROUTE_FROM_UNICAST_DAO) {
    /* Check whether this is a DAO forwarding loop. */
    p = rpl_find_parent(dag, &dao_sender_addr);
    if(p != NULL && DAG_RANK(p->rank, dag) < DAG_RANK(dag->rank, dag)) {
      PRINTF("RPL: Loop detected when receiving a unicast DAO from a node with a lower rank! (%u < %u)\n",
          DAG_RANK(p->rank, dag), DAG_RANK(dag->rank, dag));
#ifdef SENSLAB
      puts("ERR loop");
#endif /* SENSLAB */
      p->rank = INFINITE_RANK;
      p->updated = 1;
      return;
    }
  }

  if(rep == NULL) {
    rep = rpl_add_route(dag, &prefix, prefixlen, &dao_sender_addr);
    if(rep == NULL) {
      RPL_STAT(rpl_stats.mem_overflows++);
      PRINTF("RPL: Could not add a route after receiving a DAO\n");
#ifdef SENSLAB
    puts("ERR route add");
#endif /* SENSLAB */
      return;
    }
  }

  rep->state.lifetime = RPL_LIFETIME(dag, lifetime);
  rep->state.learned_from = learned_from;

  if(learned_from == RPL_ROUTE_FROM_UNICAST_DAO) {
    if(dag->preferred_parent) {
      PRINTF("RPL: Forwarding DAO to parent ");
      PRINT6ADDR(&dag->preferred_parent->addr);
      PRINTF("\n");
      uip_icmp6_send(&dag->preferred_parent->addr,
                     ICMP6_RPL, RPL_CODE_DAO, buffer_length);
    } else if(flags & RPL_DAO_K_FLAG) {
      dao_ack_output(dag, &dao_sender_addr, sequence);
    }
  }
}
/*---------------------------------------------------------------------------*/
void
dao_output(rpl_parent_t *n, rpl_lifetime_t lifetime)
{
  rpl_dag_t *dag;
  unsigned char *buffer;
  uint8_t prefixlen;
  uip_ipaddr_t addr;
  uip_ipaddr_t prefix;
  int pos;

  /* Destination Advertisement Object */
  if(get_global_addr(&prefix) == 0) {
    PRINTF("RPL: No global address set for this node - suppressing DAO\n");
    return;
  }

  if(n == NULL) {
    dag = rpl_get_dag(RPL_ANY_INSTANCE, NULL);
    if(dag == NULL) {
      PRINTF("RPL: Did not join a DAG before sending DAO\n");
      return;
    }
  } else {
    dag = n->dag;
  }

  buffer = UIP_ICMP_PAYLOAD;

  ++dao_sequence;
  pos = 0;

  buffer[pos++] = dag->instance_id;
#if RPL_CONF_DAO_ACK
  buffer[pos++] = RPL_DAO_K_FLAG; /* DAO ACK request, no DODAGID */
#else
  buffer[pos++] = 0; /* No DAO ACK request, no DODAGID */
#endif
  buffer[pos++] = 0; /* reserved */
  buffer[pos++] = dao_sequence & 0xff;

  /* create target subopt */
  prefixlen = sizeof(prefix) * CHAR_BIT;
  buffer[pos++] = RPL_OPTION_TARGET;
  buffer[pos++] = 2 + ((prefixlen + 7) / CHAR_BIT);
  buffer[pos++] = 0; /* reserved */
  buffer[pos++] = prefixlen;
  memcpy(buffer + pos, &prefix, (prefixlen + 7) / CHAR_BIT);
  pos += ((prefixlen + 7) / CHAR_BIT);

  /* Create a transit information sub-option. */
  buffer[pos++] = RPL_OPTION_TRANSIT;
  buffer[pos++] = 4;
  buffer[pos++] = 0; /* flags - ignored */
  buffer[pos++] = 0; /* path control - ignored */
  buffer[pos++] = 0; /* path seq - ignored */
  buffer[pos++] = lifetime;

  if(n == NULL) {
    uip_create_linklocal_rplnodes_mcast(&addr);
  } else {
    uip_ipaddr_copy(&addr, &n->addr);
  }

#ifdef SENSLAB
  printf("RPL DAO ");
  print6addr(&dag->dag_id);
  printf(" 0x%02x %u Bytes\n", dag->instance_id, pos);
#endif /* SENSLAB */

  PRINTF("RPL: Sending DAO with prefix ");
  PRINT6ADDR(&prefix);
  PRINTF(" to ");
  if(n != NULL) {
    PRINT6ADDR(&n->addr);
  } else {
    PRINTF("multicast address");
  }
  PRINTF("\n");

  uip_icmp6_send(&addr, ICMP6_RPL, RPL_CODE_DAO, pos);
}
/*---------------------------------------------------------------------------*/
static void
dao_ack_input(void)
{
  unsigned char *buffer;
  uint8_t buffer_length;
  uint8_t instance_id;
  uint8_t sequence;
  uint8_t status;

  buffer = UIP_ICMP_PAYLOAD;
  buffer_length = uip_len - uip_l2_l3_icmp_hdr_len;
#if RPL_CONF_ADJUST_LLH_LEN
  buffer_length+=UIP_LLH_LEN; //Add jackdaw, minimal-net ethernet header
#endif

  instance_id = buffer[0];
  sequence = buffer[2];
  status = buffer[3];

  PRINTF("RPL: Received a DAO ACK with sequence number %d and status %d from ",
    sequence, status);
  PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
  PRINTF("\n");
}
/*---------------------------------------------------------------------------*/
void
dao_ack_output(rpl_dag_t *dag, uip_ipaddr_t *dest, uint8_t sequence)
{
  unsigned char *buffer;

  PRINTF("RPL: Sending a DAO ACK with sequence number %d to ", sequence);
  PRINT6ADDR(dest);
  PRINTF("\n");

  buffer = UIP_ICMP_PAYLOAD;

  buffer[0] = dag->instance_id;
  buffer[1] = 0;
  buffer[2] = sequence;
  buffer[3] = 0;

  uip_icmp6_send(dest, ICMP6_RPL, RPL_CODE_DAO_ACK, 4);
}
#endif /* RPL_MOP_DEFAULT == RPL_MOP_STORING_NO_MULTICAST || RPL_MOP_DEFAULT == RPL_MOP_NON_STORING */
#if WITH_RPL_P2P
#if WITH_RPL_P2P_MEASUREMENT
/*---------------------------------------------------------------------------*/
static uint8_t mo_sequence = 0;
static struct process *mo_processes[RPL_P2P_MO_MAX_REQUESTS];
/*---------------------------------------------------------------------------*/
static void
mo_input(void)
{
  unsigned char *buffer;
  uint8_t buffer_length;
  uint8_t pos;
  uint8_t i;
  static rpl_mo_result_t r;
  uip_ipaddr_t origin;
  uip_ipaddr_t *nexthop;
  uip_ds6_route_t *hbhrt;
  uip_ds6_src_route_t *srcrt;
  uip_ds6_src_route_addr_t *a;
  rpl_dag_t *dag;
  uip_ds6_nbr_t *nbr;

  buffer = UIP_ICMP_PAYLOAD;
  buffer_length = uip_len - uip_l2_l3_icmp_hdr_len;

  PRINTF("RPL: Received MO\n");
  /*print_buffer(buffer, buffer_length);*/

  /* Decompress origin and target */
  memcpy(&origin, rpl_get_own_addr(1), RPL_P2P_RDO_COMPR);
  memcpy(((uint8_t *)&origin) + RPL_P2P_RDO_COMPR, &buffer[4], 16 - RPL_P2P_RDO_COMPR);
  memcpy(&r.target, rpl_get_own_addr(1), RPL_P2P_RDO_COMPR);
  memcpy(((uint8_t *)&r.target) + RPL_P2P_RDO_COMPR, &buffer[4 + 16 - RPL_P2P_RDO_COMPR], 16 - RPL_P2P_RDO_COMPR);

  /* Extract path metric */
  pos = 4 + (RPL_BITMASK_READ(P2P_MO_N, buffer[3]) + 2) * (16 - RPL_P2P_RDO_COMPR) + 6;
  r.path_metric = buffer[pos] << 8;
  r.path_metric |= buffer[pos + 1];

  /* Compare origin address */
  if(uip_ds6_is_my_addr(&origin)) {
    /* We are the origin */
    if(RPL_FLAG_GET(P2P_MO_T, buffer[1])) {
      PRINTF("RPL: Received MO request but we are the origin!\n");
      return;
    }
    PRINTF("ETX for route to ");
    PRINT6ADDR(&r.target);
    PRINTF(" is %u.%u\n", r.path_metric / RPL_DAG_MC_ETX_DIVISOR,	(r.path_metric %
          RPL_DAG_MC_ETX_DIVISOR * 100) / RPL_DAG_MC_ETX_DIVISOR);
    /* Notify the application layer that we received the MO reply. */
    if(RPL_BITMASK_READ(P2P_MO_S, buffer[2]) < RPL_P2P_MO_MAX_REQUESTS && mo_processes[RPL_BITMASK_READ(P2P_MO_S, buffer[2])] != NULL) {
      process_post_synch(mo_processes[RPL_BITMASK_READ(P2P_MO_S, buffer[2])], rpl_p2p_event_mo, &r);
    }
    return;
  }

  /* Get the value of the link metric to the previous hop */
  nbr = uip_ds6_nbr_lookup(&UIP_IP_BUF->srcipaddr);
  if(nbr != NULL) {
    /* Update path metric value */
    r.path_metric += NI_ETX_TO_RPL_ETX(neighbor_info_get_metric((rimeaddr_t *)&nbr->lladdr));
    buffer[pos] = r.path_metric >> 8;
    buffer[pos + 1] = r.path_metric & 0xff;
  } else {
    PRINTF("RPL: unable to get link metric to %u discarding MO!\n", UIP_IP_BUF->srcipaddr.u8[15]);
    return;
  }

  /* Compare target address */
  if(uip_ds6_is_my_addr(&r.target)) {
    /* We are the target */
    if(!RPL_FLAG_GET(P2P_MO_T, buffer[1])) {
      PRINTF("RPL: Received MO reply but we are the target!\n");
      return;
    }
    /* Generate MO reply */
    RPL_FLAG_CLR(P2P_MO_T, buffer[1]);
    if((buffer[0] & RPL_LOCAL_DEFAULT_INSTANCE && RPL_FLAG_GET(P2P_MO_A, buffer[1])) ||
        RPL_FLAG_GET(P2P_MO_R, buffer[1])) {
      /* Address vector contains a source route that we can inverse and use to
       * send the MO reply. */
      /* Create an entry in the source routing table */
      rpl_p2p_add_src_route(&origin, &buffer[4 + 2 * (16 - RPL_P2P_RDO_COMPR)],
          (RPL_BITMASK_READ(P2P_MO_I, buffer[3]) - 1) * (16 - RPL_P2P_RDO_COMPR),
          RPL_P2P_RDO_COMPR, 1);
    }
    /* Remove address vector to decrease packet size */
    memmove(&buffer[4 + 2 * (16 - RPL_P2P_RDO_COMPR)], &buffer[pos - 6], 8);
    buffer[3] = 0;
    /* calculate new length */
    pos = 4 + 2 * (16 - RPL_P2P_RDO_COMPR) + 8;

    PRINTF("RPL: Sending MO reply\n");
    /*print_buffer(buffer, pos);*/
    /* Send content of uip_buf when this function returns. */
    rpl_send_uip_buf = 1;
    /* The uip_buf contains the MO, we only have to change relevant header fields. */
    UIP_IP_BUF->len[0] = (UIP_ICMPH_LEN + pos) >> 8;
    UIP_IP_BUF->len[1] = (UIP_ICMPH_LEN + pos) & 0xff;
    memcpy(&UIP_IP_BUF->destipaddr, &origin, 16);
    uip_ds6_select_src(&UIP_IP_BUF->srcipaddr, &UIP_IP_BUF->destipaddr);
    UIP_ICMP_BUF->icode = RPL_CODE_MO;
    UIP_ICMP_BUF->icmpchksum = 0;
    UIP_ICMP_BUF->icmpchksum = ~uip_icmp6chksum();
    uip_len = UIP_IPH_LEN + UIP_ICMPH_LEN + pos;
    return;
  }

  /* We are an intermediate node */
  if(!RPL_FLAG_GET(P2P_MO_T, buffer[1])) {
    PRINTF("RPL: Received MO reply but we are an intermediate node!\n");
    return;
  }
  /* Determine next hop */
  if(!RPL_FLAG_GET(P2P_MO_H, buffer[1])) {
    /* MO travels along source route */
    buffer[3]++; /* XXX unhandled overflow!!! */
    if(RPL_BITMASK_READ(P2P_MO_I, buffer[3]) <= RPL_BITMASK_READ(P2P_MO_N, buffer[3])) {
      /* Decompress nexthop address (reuse target variable) */
      memcpy(((uint8_t *)&r.target) + RPL_P2P_RDO_COMPR, &buffer[4 + (RPL_BITMASK_READ(P2P_MO_I, buffer[3]) + 1)* (16 - RPL_P2P_RDO_COMPR)], 16 - RPL_P2P_RDO_COMPR);
    }
    nexthop = &r.target;
  } else {
    /* MO travels along Hop-by-hop route */
    dag = rpl_get_dag(buffer[0], NULL);
    /* Are we the root of a global non-storing DAG? */
    if(!buffer[0] & RPL_LOCAL_DEFAULT_INSTANCE && dag != NULL &&
        dag->mop == RPL_MOP_NON_STORING && dag->rank == ROOT_RANK(dag)) {
      /* Look up source route to the target */
      srcrt = uip_ds6_src_route_lookup(&r.target);
      if(srcrt == NULL) {
        PRINTF("RPL: Root of global non-storing DAG but no src route to target!\n");
        uip_icmp6_error_output(ICMP6_DST_UNREACH, ICMP6_DST_UNREACH_NOROUTE, 0);
        return;
      }
      /* Clear flags */
      RPL_FLAG_CLR(P2P_MO_H, buffer[1]);
      RPL_FLAG_CLR(P2P_MO_A, buffer[1]);
      RPL_FLAG_CLR(P2P_MO_R, buffer[1]);
      /* Index is 1 */
      RPL_BITMASK_WRITE(P2P_MO_I, buffer[3], 1);
      RPL_BITMASK_WRITE(P2P_MO_N, buffer[3], (list_length(srcrt->addr_list) - 1));
      /* Move metric container to make space for address vector */
      memmove(&buffer[4 + (RPL_BITMASK_READ(P2P_MO_N, buffer[3]) + 2) * (16 - RPL_P2P_RDO_COMPR)], &buffer[pos - 6], 8);
      pos = 4 + 2 * (16 - RPL_P2P_RDO_COMPR);
      a = list_head(srcrt->addr_list);
      nexthop = &a->addr;
      /* Insert address vector with comressed source route but ommit last
       * address (link-local address of the target) */
      for (; a != NULL && list_item_next(a) != NULL; a = list_item_next(a)) {
        memcpy(&buffer[pos], ((uint8_t *)&a->addr) + RPL_P2P_RDO_COMPR, 16 - RPL_P2P_RDO_COMPR);
        pos += 16 - RPL_P2P_RDO_COMPR;
      }
      /* Correct the packet length */
      pos += 8;
      UIP_IP_BUF->len[0] = (UIP_ICMPH_LEN + pos) >> 8;
      UIP_IP_BUF->len[1] = (UIP_ICMPH_LEN + pos) & 0xff;
      uip_len = UIP_IPH_LEN + UIP_ICMPH_LEN + pos;
    } else {
      /* Look up hop-by-hop route to target
       * XXX use dodagid and instance to lookup correct route */
      if((hbhrt = uip_ds6_route_lookup(&r.target)) != NULL) {
        nexthop = &hbhrt->nexthop;
      } else if((nexthop = uip_ds6_defrt_choose()) == NULL) {
        printf("RPL: Unable to determine nexthop for MO!\n");
        uip_icmp6_error_output(ICMP6_DST_UNREACH, ICMP6_DST_UNREACH_NOROUTE, 0);
        return;
      }
      /* Do we need to add our address to the vector? */
      if(buffer[0] & RPL_LOCAL_DEFAULT_INSTANCE && RPL_FLAG_GET(P2P_MO_A, buffer[1])) {
        /* Check address vector for duplicate */
        for(i = 2; i <= RPL_BITMASK_READ(P2P_MO_I, buffer[3]); i++) {
          /* Decompress address (reuse target variable) */
          memcpy(((uint8_t *)&r.target) + RPL_P2P_RDO_COMPR, &buffer[4 + i * (16 - RPL_P2P_RDO_COMPR)], 16 - RPL_P2P_RDO_COMPR);
          if(uip_ds6_is_my_addr(&r.target)) {
            PRINTF("RPL: Loop in address vector, dropping MO!\n");
            return;
          }
        }
        /* add our compressed address */
        memcpy(&buffer[4 + i * (16 - RPL_P2P_RDO_COMPR)], ((uint8_t *)rpl_get_own_addr(1)) + RPL_P2P_RDO_COMPR, 16 - RPL_P2P_RDO_COMPR);
        buffer[3]++;
      }
    }
  }
  /* XXX This is a hack! we need a link local address for the next hop,
   * since neighbor discovery is not implemented or does not work. */
  uip_create_linklocal_prefix(nexthop);
  /* Unicast the message to the next hop */
  PRINTF("RPL: Forwarding MO to ");
  PRINT6ADDR(nexthop);
  PRINTF("\n");
  /*print_buffer(buffer, buffer_length);*/
  /* Send content of uip_buf when this function returns. */
  rpl_send_uip_buf = 1;
  /* Packet header is unchanged except for IPv6 addresses and ICMP checksum. */
  memcpy(&UIP_IP_BUF->destipaddr, nexthop, 16);
  uip_ds6_select_src(&UIP_IP_BUF->srcipaddr, &UIP_IP_BUF->destipaddr);
  UIP_ICMP_BUF->icmpchksum = 0;
  UIP_ICMP_BUF->icmpchksum = ~uip_icmp6chksum();
}
/*---------------------------------------------------------------------------*/
void
mo_output(uip_ipaddr_t *target)
{
  unsigned char *buffer;
  uint8_t pos;
  uip_ds6_src_route_t *srcrt;
  uip_ds6_src_route_addr_t *a;
  uip_ds6_route_t *hbhrt;
  uip_ipaddr_t *nexthop;
  rpl_dag_t *dag;

  /* Measurement Object */
  if(target == NULL) {
    PRINTF("RPL: Invalid target address, unable to send MO!\n");
    return;
  }

  mo_sequence = (mo_sequence + 1) % RPL_P2P_MO_MAX_REQUESTS;
  mo_processes[mo_sequence] = PROCESS_CURRENT();

  buffer = UIP_ICMP_PAYLOAD;
  /* Set fields that are independet of the type of route that we are measuring */
  pos = 1;
  buffer[pos] = 0;
  RPL_BITMASK_WRITE(P2P_MO_C, buffer[pos], RPL_P2P_RDO_COMPR);
  RPL_FLAG_SET(P2P_MO_T, buffer[pos]);
  buffer[++pos] = 0;
  RPL_BITMASK_WRITE(P2P_MO_S, buffer[pos], mo_sequence);
  buffer[++pos] = 0;
  pos++;
  /* Add compressed origin and target */
  memcpy(&buffer[pos], ((uint8_t *)rpl_get_own_addr(1)) + RPL_P2P_RDO_COMPR, 16 - RPL_P2P_RDO_COMPR);
  pos += 16 - RPL_P2P_RDO_COMPR;
  memcpy(&buffer[pos], ((uint8_t *)target) + RPL_P2P_RDO_COMPR, 16 - RPL_P2P_RDO_COMPR);
  pos += 16 - RPL_P2P_RDO_COMPR;

  srcrt = uip_ds6_src_route_lookup(target);
  if(srcrt != NULL) {
    /* We have a full source route to the target. */
    buffer[0] = RPL_LOCAL_DEFAULT_INSTANCE;
    RPL_FLAG_SET(P2P_MO_R, buffer[1]);
    /* Index is 1 */
    RPL_BITMASK_WRITE(P2P_MO_I, buffer[3], 1);
    RPL_BITMASK_WRITE(P2P_MO_N, buffer[3], (list_length(srcrt->addr_list) - 1));
    a = list_head(srcrt->addr_list);
    nexthop = &a->addr;
    /* Add compressed addresses except last one (link-local address of the
     * target) */
    for (; a != NULL && list_item_next(a) != NULL; a = list_item_next(a)) {
      memcpy(&buffer[pos], ((uint8_t *)&a->addr) + RPL_P2P_RDO_COMPR, 16 - RPL_P2P_RDO_COMPR);
      pos += 16 - RPL_P2P_RDO_COMPR;
    }
  } else {
    /* No source route, see if we have a hbh route */
    hbhrt = uip_ds6_route_lookup(target);
    if(hbhrt != NULL) {
      buffer[0] = hbhrt->state.instance;
      RPL_FLAG_SET(P2P_MO_H, buffer[1]);
      nexthop = &hbhrt->nexthop;
      /* If the instance ID is local, than the DADAGID must be our global
       * address, otherwise we cannot measure this route. */
      if(hbhrt->state.instance & RPL_LOCAL_DEFAULT_INSTANCE) {
        if(!uip_ds6_is_my_addr(&hbhrt->state.dodagid)) {
          PRINTF("RPL: Local hop-by-hop route does not belong to me, unable to send MO!\n");
          return;
        }
        /* We measure a hop-by-hop route with local instance ID. */
        RPL_FLAG_SET(P2P_MO_A, buffer[1]);
        /* Index is 1 */
        RPL_BITMASK_WRITE(P2P_MO_I, buffer[3], 1);
        RPL_BITMASK_WRITE(P2P_MO_N, buffer[3], RPL_P2P_MO_DEFAULT_VECTOR_SIZE);
        /* Add an empty address vector */
        memset(&buffer[4 + 2 * (16 - RPL_P2P_RDO_COMPR)], 0, RPL_P2P_MO_DEFAULT_VECTOR_SIZE * (16 - RPL_P2P_RDO_COMPR));
        pos += RPL_P2P_MO_DEFAULT_VECTOR_SIZE * (16 - RPL_P2P_RDO_COMPR);
      } else {
        /* We measure a hop-by-hop route with global instance ID. */
        /* Do not record the route and do not add an address vector */
      }
    /* Try to use default route along global DODAG */
    } else if((nexthop = uip_ds6_defrt_choose()) != NULL &&
              (dag = rpl_get_dag(RPL_DEFAULT_INSTANCE, NULL)) != NULL) {
      buffer[0] = dag->instance_id;
      RPL_FLAG_SET(P2P_MO_H, buffer[1]);
    } else {
      /* No route at all */
      PRINTF("RPL: No route to target, unable to send MO!\n");
      return;
    }
  }

  /* Add metric container */
  buffer[pos++] = RPL_OPTION_DAG_METRIC_CONTAINER;
  buffer[pos++] = 6;
  buffer[pos++] = RPL_DAG_MC_ETX;
  /* no flags */
  buffer[pos++] = 0;
  buffer[pos++] = 0 | (RPL_DAG_MC_AGGR_ADDITIVE << 4);
  buffer[pos++] = 2;
  /* ETX value of a link may differ depeneding on the direction in which it is
   * measured (especially since we currently approximate it from LQI). Therefore
   * we measure it in backwards direction as in the P2P route discovery. */
  buffer[pos++] = 0;
  buffer[pos++] = 0;

  /* Unicast the message to the next hop */
  PRINTF("RPL: Sending MO\n");
  /*print_buffer(buffer, pos);*/
  uip_icmp6_send(nexthop, ICMP6_RPL, RPL_CODE_MO, pos);
}
#endif /* WITH_RPL_P2P_MEASUREMENT */
/*---------------------------------------------------------------------------*/
MEMB(dro_cache_memb, rpl_dro_cache_entry_t, RPL_MAX_DRO_CACHE_ENTRIES);
LIST(dro_cache);
/*---------------------------------------------------------------------------*/
static void
dro_rexmit(void *ptr)
{
  uip_ipaddr_t addr;
  rpl_dro_cache_entry_t *c;
  c = (rpl_dro_cache_entry_t *)ptr;

  memcpy(UIP_ICMP_PAYLOAD, &c->buffer, c->length);
  c->rexmits++;
  if(c->rexmits < RPL_MAX_DRO_REXMITS) {
    ctimer_reset(&c->timer);
  } else {
    list_remove(dro_cache, c);
    memb_free(&dro_cache_memb, c);
  }

  PRINTF("RPL: Retransmiting DRO at %p, length: %d, rexmits: %d\n", &c->buffer, c->length, c->rexmits);
  uip_create_linklocal_rplnodes_mcast(&addr);
  uip_icmp6_send(&addr, ICMP6_RPL, RPL_CODE_DRO, c->length);
}
/*---------------------------------------------------------------------------*/
static void
dro_ack_input(void)
{
  unsigned char *buffer;
  rpl_dro_cache_entry_t *c;

  uint8_t removed = 0;

  buffer = UIP_ICMP_PAYLOAD;
  printf("P2P: Received DRO-ACK from %d\n", UIP_IP_BUF->srcipaddr.u8[15]);
  /* print_buffer(buffer, uip_len - uip_l2_l3_icmp_hdr_len); */

  /* Check whether we have any pending retransmissions for this DRO */
  for(c = list_head(dro_cache); c != NULL; c = list_item_next(c)) {
    /* DRO matches if it has the same DODAGID, instance and DRO sequence number */
    if(buffer[0] == c->buffer[0] &&\
       uip_ipaddr_cmp(&buffer[4], &c->buffer[4]) &&\
       RPL_BITMASK_READ(P2P_DRO_Q, buffer[2]) == RPL_BITMASK_READ(P2P_DRO_Q, buffer[2])) {
      /* DRO has arrived, cancel retransmission timer and free memory. */
      PRINTF("RPL: Canceling rexmit for DRO at %p length %d\n", &c->buffer, c->length);
      ctimer_stop(&c->timer);
      list_remove(dro_cache, c);
      memb_free(&dro_cache_memb, c);
      removed = 1;
      break;
    }
  }
  if(!removed) {
    PRINTF("Did not find entry for ");
    PRINT6ADDR((uip_ipaddr_t *)&buffer[4]);
    PRINTF(" 0x%02x (%d packets cached)\n", buffer[0], list_length(dro_cache));
  }
}
/*---------------------------------------------------------------------------*/
static void
dro_input(void)
{
  rpl_dag_t *dag;
  unsigned char *buffer;
  uint8_t buffer_length;
  uint8_t instance_id;
  uint8_t version;
  uint8_t compr;
  uint8_t rem;
  uip_ipaddr_t *dodag_id;
  uip_ipaddr_t target;
  uip_ipaddr_t nexthop;
  uip_ds6_route_t *hbhrt;
  int pos;

  buffer = UIP_ICMP_PAYLOAD;
  buffer_length = uip_len - uip_l2_l3_icmp_hdr_len;

  /* Discovery Reply Object */
  PRINTF("RPL: Received DRO from %d\n", UIP_IP_BUF->srcipaddr.u8[15]);
  //print_buffer(buffer, buffer_length);

  pos = 0;
  instance_id = buffer[pos++];
  version = buffer[pos++];
  /* Skip flags and reserved */
  pos += 2;
  dodag_id = (uip_ipaddr_t *)&buffer[pos];
  pos += 16;
  while(buffer[pos] != RPL_DIO_SUBOPT_ROUTE_DISCOVERY && pos < buffer_length) {
    if(pos >= buffer_length) {
      PRINTF("RPL: Received DRO without RDO.\n");
      return;
    }
    pos += buffer[pos + 1] + 2;
  }
  compr = RPL_BITMASK_READ(P2P_RDO_C, buffer[pos + 2]);
  rem = RPL_BITMASK_READ(P2P_RDO_R, buffer[pos + 3]);

  /* Evaluate STOP bit */
  if(RPL_FLAG_GET(P2P_DRO_S, buffer[2])) {
    dag = rpl_get_dag(instance_id, dodag_id);
    if(dag != NULL && !dag->p2p->expired) {
      printf("RPL: Received DRO with STOP bit while DAG still active, detaching.\n");
      ctimer_stop(&dag->dio_timer);
      dag->p2p->lifetime = 0;
    }
  }

  /* Are we the origin of the corresponding route request? */
  if(uip_ds6_is_my_addr(dodag_id)) {
    PRINTF("RPL: DRO reached origin\n");
#if (DEBUG) & DEBUG_ANNOTATE
    rpl_annotate_rdo(&buffer[pos], dodag_id);
#endif /* (DEBUG) & DEBUG_ANNOTATE */
#ifdef SENSLAB
    printf("P2P DAG ");
    print6addr(dodag_id);
    printf(" 0x%02x REPLY ", buffer[0]);
    rpl_print_rdo(&buffer[pos], dodag_id);
    putchar('\n');
#endif /* SENSLAB */
    /* Decompress the target */
    memcpy(&target, dodag_id, compr);
    memcpy(((uint8_t *)&target) + compr, &buffer[pos + 4], 16 - compr);
    if(RPL_FLAG_GET(P2P_RDO_H, buffer[pos + 2])) {
      if(pos + 4 + 16 - compr == buffer_length) {
        /* If the address vector is empty, target = nexthop */
        memcpy(&nexthop, &target, 16);
      } else {
        /* Decompress first address in address vector */
        memcpy(&nexthop, dodag_id, compr);
        memcpy(((uint8_t *)&nexthop) + compr, &buffer[pos + 4 + 16 - compr], 16 - compr);
      }
      /* XXX This is a hack! we need a link local address for the next hop,
       * since neighbor discovery is not implemented or does not work. */
      uip_create_linklocal_prefix(&nexthop);
      /* Store state for hop by hop route */
      rpl_p2p_add_hbh_route(instance_id, dodag_id, &target, &nexthop);
    } else {
      /* Store the source route contained in the RDO */
      rpl_p2p_add_src_route(&target, &buffer[pos + 4 + 16 - compr], buffer[pos + 1] - 2 - 16 + compr, compr, 0);
    }
    /* Send DRO-ACK if requested */
    if(RPL_FLAG_GET(P2P_DRO_A, buffer[2])) {
      /* Packet follows the same format as DRO, everything after Seq is reserved. */
      buffer[2] &= RPL_P2P_DRO_Q_MASK;
      buffer[3] = 0;
      /* Send content of uip_buf when this function returns. */
      rpl_send_uip_buf = 1;
      /* The uip_buf contains a DRO, we only have to change relevant header fields. */
      UIP_IP_BUF->len[0] = (UIP_ICMPH_LEN + 20) >> 8;
      UIP_IP_BUF->len[1] = (UIP_ICMPH_LEN + 20) & 0xff;
      /* Decompress the target */
      memcpy(&UIP_IP_BUF->destipaddr, dodag_id, compr);
      memcpy(((uint8_t *)&UIP_IP_BUF->destipaddr) + compr, &buffer[pos + 4], 16 - compr);
      uip_ds6_select_src(&UIP_IP_BUF->srcipaddr, &UIP_IP_BUF->destipaddr);
      UIP_ICMP_BUF->icode = RPL_CODE_DRO_ACK;
      UIP_ICMP_BUF->icmpchksum = 0;
      UIP_ICMP_BUF->icmpchksum = ~uip_icmp6chksum();
      uip_len = UIP_IPH_LEN + UIP_ICMPH_LEN + 20;
#ifdef SENSLAB
      printf("P2P DRO-ACK ");
      print6addr(dodag_id);
      printf(" 0x%02x 20 Bytes\n", buffer[0]);
#endif /* SENSLAB */
      /* print_buffer(buffer, 20); */
    }
    /* Notify the application layer that we received the route reply. */
    if((dag = rpl_get_dag(instance_id, dodag_id)) != NULL && dag->p2p->process != NULL) {
      PRINTF("RPL: Notifying process %s at %p\n", PROCESS_NAME_STRING(dag->p2p->process), dag->p2p->process);
      process_post_synch(dag->p2p->process, rpl_p2p_event_dro, NULL);
    } else {
      /* Normally the DRO should arrive before the DODAG expires. And even if it
       * is expired, the data structure is still kept around for some time. If
       * this turns out to be not sufficient, we need to implement an additional
       * static data structure that stores pointers to the processes that
       * initiated a route request. */
      PRINTF("RPL: Unable to notify application about DRO reception!\n");
    }
    return;
  }

  /* We are an intermediate node */
  /* Decompress address at position REM */
  memcpy(&nexthop, dodag_id, compr);
  memcpy(((uint8_t *)&nexthop) + compr, &buffer[pos + 4 + rem * (16 - compr)], 16 - compr);
  /* Stop processing and do not forward if we do not take part in the route. It's
   * important to check for Rem > 0 because address[0] points to the target
   * address and the target would thus forward the DRO again in that case. */
  if(rem <= 0 || !uip_ds6_is_my_addr(&nexthop)) {
    PRINTF("RPL: Not forwarding DRO\n");
    return;
  }

  /* We take part in the route, store hop by hop route if requested */
  if(RPL_FLAG_GET(P2P_RDO_H, buffer[pos + 2])) {
    /* Decompress the target */
    memcpy(&target, dodag_id, compr);
    memcpy(((uint8_t *)&target) + compr, &buffer[pos + 4], 16 - compr);
    if(pos + 4 + (rem + 1) * (16 - compr) >= buffer_length) {
      /* Rem points to last address, target is nexthop */
      memcpy(&nexthop, &target, 16);
    } else {
      /* Decompress address at Rem + 1 in address vector */
      memcpy(&nexthop, dodag_id, compr);
      memcpy(((uint8_t *)&nexthop) + compr, &buffer[pos + 4 + (rem + 1) * (16 - compr)], 16 - compr);
    }
    /* XXX This is a hack! we need a link local address for the next hop,
     * since neighbor discovery is not implemented or does not work. */
    uip_create_linklocal_prefix(&nexthop);
	/* Do we already have a route with same RPLInstanceID and DODAGID to this target? */
	if((hbhrt = uip_ds6_route_lookup(&target)) != NULL) {
	  /* XXX actually compare RPLInstanceID and DODAGID */
	  if(uip_ipaddr_cmp(&hbhrt->nexthop, &nexthop)) {
		PRINTF("RPL: Received DRO with conflicting next hop\n");
		return;
	  }
	} else {
      /* Store state for hop by hop route */
      rpl_p2p_add_hbh_route(instance_id, dodag_id, &target, &nexthop);
	}
  }
  /* Rem field is at the very end of the field, thus we can simply substract */
  buffer[pos + 3] -= 1;

  PRINTF("RPL: Forwarding DRO\n");
  /*print_buffer(buffer, pos + 2 + buffer[pos + 1]);*/

#ifdef SENSLAB
  printf("P2P DRO ");
  print6addr(dodag_id);
  printf(" 0x%02x %u Bytes\n", buffer[0], pos + 2 + buffer[pos + 1]);
#endif /* SENSLAB */

  /* Send content of uip_buf when this function returns. */
  rpl_send_uip_buf = 1;
  /* Packet header is unchanged except for IPv6 source address and ICMP checksum. */
  uip_ds6_select_src(&UIP_IP_BUF->srcipaddr, &UIP_IP_BUF->destipaddr);
  UIP_ICMP_BUF->icmpchksum = 0;
  UIP_ICMP_BUF->icmpchksum = ~uip_icmp6chksum();
}
/*---------------------------------------------------------------------------*/
void
dro_output(rpl_dag_t *dag, list_t addr_list)
{
  unsigned char *buffer;
  uint8_t pos, rem;
  uip_ipaddr_t addr;

  /* Destination Reply Object */
  if(dag == NULL || dag->p2p == NULL) {
    PRINTF("RPL: Invalid DAG object, unable to send DRO!\n");
    return;
  }

  buffer = UIP_ICMP_PAYLOAD;
  pos = 0;

  buffer[pos++] = dag->instance_id;
  buffer[pos++] = dag->version;
  buffer[pos] = 0;
  /* Sequence number */
  RPL_BITMASK_WRITE(P2P_DRO_Q, buffer[pos], dag->p2p->dro_seq++);
  /*Set STOP bit and let DAG expire if we don't need further routes. */
  if(RPL_P2P_SET_STOP_BIT) {
    PRINTF("P2P: Setting STOP bit in DRO\n");
    RPL_FLAG_SET(P2P_DRO_S, buffer[pos]);
    dag->p2p->lifetime = 0;
  }
  /* Request DRO-ACK */
  RPL_FLAG_SET(P2P_DRO_A, buffer[pos]);
  pos++;
  /* Reserved */
  buffer[pos++] = 0;
  memcpy(&buffer[pos], &dag->dag_id, 16);
  pos += 16;

  /* Add the route discovery option */
  write_rdo(&buffer[pos], dag, addr_list);
  /* Clear Rem field */
  buffer[pos + 3] &= ~RPL_P2P_RDO_R_MASK;
  /* Set Rem field to number of address in the vector */
  rem = (buffer[pos + 1] - 2) / (16 - RPL_BITMASK_READ(P2P_RDO_C, buffer[pos + 2])) - 1;
  RPL_BITMASK_WRITE(P2P_RDO_R, buffer[pos + 3], rem);
  pos += buffer[pos + 1] + 2;
  /* No metric container */

  /* Cache the packet in order to being able to retransmit it in case no ack was
   * received. */
  if(pos <= RPL_DRO_CACHE_ENTRY_SIZE) {
    rpl_dro_cache_entry_t *c;
    c = memb_alloc(&dro_cache_memb);
    if (c != NULL) {
      c->length = pos;
      memcpy(&c->buffer, buffer, c->length);
      c->rexmits = 0;
      ctimer_set(&c->timer, RPL_DRO_ACK_WAIT_TIME, dro_rexmit, c);
      list_add(dro_cache, c);
      PRINTF("RPL: Caching DRO at %p length %d\n", &c->buffer, c->length);
    } else {
      PRINTF("RPL: No free memory to cache DRO\n");
    }
  } else {
    PRINTF("RPL: DRO too large to cache.\n");
  }

#ifdef SENSLAB
  printf("P2P DRO ");
  print6addr(&dag->dag_id);
  printf(" 0x%02x %u Bytes ", dag->instance_id, pos);
  rpl_print_rdo(&buffer[20], &dag->dag_id);
  putchar('\n');
#endif /* SENSLAB */

  /* print_buffer(buffer, pos); */
  PRINTF("RPL: Sending DRO\n");
  uip_create_linklocal_rplnodes_mcast(&addr);
  uip_icmp6_send(&addr, ICMP6_RPL, RPL_CODE_DRO, pos);
}
#endif /* WITH_RPL_P2P */
/*---------------------------------------------------------------------------*/
void
uip_rpl_input(void)
{
  uip_ds6_nbr_t *nbr;

  /* Make sure neighbor cache is also filled when receiving DROs. */
  if((nbr = uip_ds6_nbr_lookup(&UIP_IP_BUF->srcipaddr)) == NULL) {
    if((nbr = uip_ds6_nbr_add(&UIP_IP_BUF->srcipaddr, (uip_lladdr_t *)
                              packetbuf_addr(PACKETBUF_ADDR_SENDER),
                              0, NBR_REACHABLE)) != NULL) {
      /* set reachable timer */
      stimer_set(&nbr->reachable, UIP_ND6_REACHABLE_TIME / 1000);
      PRINTF("RPL: Neighbor added to neighbor cache ");
      PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
      PRINTF(", ");
      PRINTLLADDR((uip_lladdr_t *)packetbuf_addr(PACKETBUF_ADDR_SENDER));
      PRINTF("\n");
    }
  } else {
    PRINTF("RPL: Neighbor already in neighbor cache\n");
  }

  PRINTF("Received an RPL control message\n");
  switch(UIP_ICMP_BUF->icode) {
  case RPL_CODE_DIO:
    dio_input();
    break;
  case RPL_CODE_DIS:
    dis_input();
    break;
#if RPL_MOP_DEFAULT == RPL_MOP_STORING_NO_MULTICAST || RPL_MOP_DEFAULT == RPL_MOP_NON_STORING
  case RPL_CODE_DAO:
    dao_input();
    break;
  case RPL_CODE_DAO_ACK:
    dao_ack_input();
    break;
#endif /* RPL_MOP_DEFAULT == RPL_MOP_STORING_NO_MULTICAST || RPL_MOP_DEFAULT == RPL_MOP_NON_STORING */
#if WITH_RPL_P2P
  case RPL_CODE_DRO:
    dro_input();
    break;
  case RPL_CODE_DRO_ACK:
    dro_ack_input();
    break;
#if WITH_RPL_P2P_MEASUREMENT
  case RPL_CODE_MO:
    mo_input();
    break;
#endif /* WITH_RPL_P2P_MEASUREMENT */
#endif /* WITH_RPL_P2P */
  default:
    PRINTF("RPL: received an unknown ICMP6 code (%u)\n", UIP_ICMP_BUF->icode);
    break;
  }

  /* When this function returns and uip_len != 0 then the content of the uip_buf
   * gets sent. */
  if(rpl_send_uip_buf) {
    rpl_send_uip_buf = 0;
  } else {
    uip_len = 0;
  }
}
