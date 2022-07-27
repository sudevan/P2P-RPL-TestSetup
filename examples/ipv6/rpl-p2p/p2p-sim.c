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
 *         Example how to use the RPL Point to Point (P2P) extension.
 *
 * This program has been written for Cooja and Sky motes as it makes heavy use
 * of annotations and node attributes to visualize what is going on. But
 * nevertheless it should run on real nodes as well.
 *
 * Make sure to set DEBUG_ANNOTATE in rpl-icmp6.c, rpl-p2p.c, and rpl-dag.c
 *
 * Generation of IPv6 addresses from node IDs is hard-coded and requires
 * the mac address to be 02:00:00:00:00:00:00:<node-id> this can be achieved by
 * seting '#define IEEE_802154_MAC_ADDRESS {2,0,0,0,0,0,0,0}' in
 * platform/xxx/contiki-conf.h
 *
 * \author Matthias Philipp <matthias-philipp@gmx.de>
 *
 */

#include "contiki.h"
 #include "dev/serial-line.h"
#include "lib/random.h"
#include "sys/ctimer.h"
#include "net/uip.h"
#include "net/uip-ds6.h"
#include "net/uip-udp-packet.h"
#include "sys/ctimer.h"
#include "dev/button-sensor.h"
#include "net/neighbor-info.h"
#include "net/netstack.h"
#include "net/rpl/rpl-p2p.h"
#include "collect-view.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define UDP_CLIENT_PORT 0xcafe
#define UDP_SERVER_PORT 0xf00d

#define DEBUG DEBUG_FULL
#include "net/uip-debug.h"

#define MAX_PAYLOAD_LEN		10

#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])

#define NUM_NODES 9
#define MAX_PINGS 3
/* RPL_P2P_STORE_BACKWARD_ROUTES has to be enabled in rpl-p2p.h in oder to make replies work. */
#define WITH_UDP_PONG 1
#define WITH_GLOBAL_DAG 1
//static struct uip_udp_conn *client_conn;
static struct uip_udp_conn *out_conn;
static struct uip_udp_conn *in_conn;
static uip_ipaddr_t remote_ipaddr;
static uip_ipaddr_t own_ipaddr;
static uint8_t pings_sent = 0;
//static struct ctimer color_timer;
static struct ctimer timeout;
static uint8_t numberOfNodes;
static enum app_state{IDLE, MEASURE_BEFORE, DISCOVER, MEASURE_AFTER, PING} state = IDLE;
static unsigned long time_offset;
#define CACHESIZE 3
#define CACHETIMEOUT CLOCK_SECOND*120
struct cachetype
{
	unsigned long time;
	uint8_t target;
};
static struct cachetype p2pcache[CACHESIZE];

static unsigned long
get_time (void)
{
  return clock_seconds() + time_offset;
}

static void savetocache(uint8_t target)
{
	int i;
	for ( i = CACHESIZE-1; i > 0;i--)
	{
		p2pcache[i] =p2pcache[i-1];
	}
	p2pcache[0].time = get_time();
	p2pcache[0].target = target;
}
static uint8_t checkcache(uint8_t target)
{
	int i;
/*
	for(i = 0;i <CACHESIZE;i++)
	{
		if(p2pcache[i].target == target)
		{
			printf("Cache time : %u current time :%u", p2pcache[i].time,get_time());

			if(p2pcache[i].time + CACHETIMEOUT < get_time())
			{
				return 1;
			}
		}
	}*/
	return 0;
}
/*---------------------------------------------------------------------------*/
PROCESS(udp_client_process, "UDP client process");
AUTOSTART_PROCESSES(&udp_client_process);
/*---------------------------------------------------------------------------*/
static void
send_udp_ping()
{
  char buf[MAX_PAYLOAD_LEN];
  pings_sent++;
  //ANNOTATE("#A ping=S->%u#%u\n", remote_ipaddr.u8[15], pings_sent);
  sprintf(buf, "ping %d", pings_sent);
  PRINTF("Sending %s to %d\n", buf, remote_ipaddr.u8[15]);
  uip_udp_packet_sendto(out_conn, buf, strlen(buf),
                        &remote_ipaddr, UIP_HTONS(UDP_SERVER_PORT));
}
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
static void
reset_color(void *ptr)
{
  ANNOTATE("#A color=white\n");
}
/*---------------------------------------------------------------------------*/
static void
tcpip_handler(void)
{
  char *str;
  uint8_t seq;

  if(uip_newdata()) {
    str = uip_appdata;
    str[uip_datalen()] = '\0';
    if(strncmp(str, "ping", 4) == 0) {
      seq = atoi(&str[4]);
      PRINTF("Received %s from %d\n", str, UIP_IP_BUF->srcipaddr.u8[15]);
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
print_local_addresses(void)
{
  int i;
  uint8_t state;

  PRINTF("Client IPv6 addresses: ");
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(uip_ds6_if.addr_list[i].isused &&
       (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
      PRINT6ADDR(&uip_ds6_if.addr_list[i].ipaddr);
      PRINTF("\n");
      /* hack to make address "final" */
      if (state == ADDR_TENTATIVE) {
        uip_ds6_if.addr_list[i].state = ADDR_PREFERRED;
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
set_global_address(void)
{
  uip_ip6addr(&own_ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
  uip_ds6_set_addr_iid(&own_ipaddr, &uip_lladdr);
  uip_ds6_addr_add(&own_ipaddr, 0, ADDR_AUTOCONF);
  /* Initialize server address */
  uip_ipaddr_copy(&remote_ipaddr, &own_ipaddr);
}
/*---------------------------------------------------------------------------*/
static void
set_server_address(uint8_t target_id)
{
  remote_ipaddr.u8[15] = target_id;
  PRINTF("Set server to node %d (", target_id);
  PRINT6ADDR(&remote_ipaddr);
  PRINTF(")\n");
  /* Make sure the connection is only created once. */
  if(out_conn == NULL) {
    out_conn = udp_new(NULL, UIP_HTONS(UDP_SERVER_PORT), NULL);
  }
  udp_bind(out_conn, UIP_HTONS(UDP_CLIENT_PORT));
}


void
collect_common_recv(const rimeaddr_t *originator, uint8_t seqno, uint8_t hops,
                    uint8_t *payload, uint16_t payload_len)
{
  unsigned long time;
  uint16_t data;
  int i;

  printf("%u", 8 + payload_len / 2);
  /* Timestamp. Ignore time synch for now. */
  time = get_time();
  printf(" %lu %lu 0", ((time >> 16) & 0xffff), time & 0xffff);
  /* Ignore latency for now */
  printf(" %u %u %u %u",
         originator->u8[0] + (originator->u8[1] << 8), seqno, hops, 0);
  for(i = 0; i < payload_len / 2; i++) {
    memcpy(&data, payload, sizeof(data));
    payload += sizeof(data);
    printf(" %u", data);
  }
  printf("\n");
}
static void
print_data( uint8_t *appdata,uint16_t payload_len)
{

  rimeaddr_t sender;
  uint8_t seqno;
  uint8_t hops;

    sender.u8[0] = UIP_IP_BUF->srcipaddr.u8[15];
    sender.u8[1] = UIP_IP_BUF->srcipaddr.u8[14];
    seqno = *appdata;
    hops = uip_ds6_if.cur_hop_limit - UIP_IP_BUF->ttl + 1;
    collect_common_recv(&sender, seqno, hops,
                        appdata + 2,payload_len);
}

void
collect_common_send(void)
{
  static uint8_t seqno;
  struct {
    uint8_t seqno;
    uint8_t for_alignment;
    struct collect_view_data_msg msg;
  } msg;
  /* struct collect_neighbor *n; */
  uint16_t parent_etx;
  uint16_t rtmetric;
  uint16_t num_neighbors;
  uint16_t beacon_interval;
  rpl_parent_t *preferred_parent;
  rimeaddr_t parent;
  rpl_dag_t *dag;

  memset(&msg, 0, sizeof(msg));
  seqno++;
  if(seqno == 0) {
    /* Wrap to 128 to identify restarts */
    seqno = 128;
  }
  msg.seqno = seqno;

  rimeaddr_copy(&parent, &rimeaddr_null);
  parent_etx = 0;

  dag = rpl_get_dag(RPL_DEFAULT_INSTANCE,NULL);
  if(dag != NULL) {
    preferred_parent = dag->preferred_parent;
    if(preferred_parent != NULL) {
      uip_ds6_nbr_t *nbr;
      nbr = uip_ds6_nbr_lookup(&preferred_parent->addr);
      if(nbr != NULL) {
        /* Use parts of the IPv6 address as the parent address, in reversed byte order. */
        parent.u8[RIMEADDR_SIZE - 1] = nbr->ipaddr.u8[sizeof(uip_ipaddr_t) - 2];
        parent.u8[RIMEADDR_SIZE - 2] = nbr->ipaddr.u8[sizeof(uip_ipaddr_t) - 1];
        parent_etx = neighbor_info_get_metric((rimeaddr_t *) &nbr->lladdr) / 2;
      }
    }
    rtmetric = dag->rank;
    beacon_interval = (uint16_t) ((2L << dag->dio_intcurrent) / 1000);
    num_neighbors = RPL_PARENT_COUNT(dag);
  } else {
    rtmetric = 0;
    beacon_interval = 0;
    num_neighbors = 0;
  }

  /* num_neighbors = collect_neighbor_list_num(&tc.neighbor_list);*/
  collect_view_construct_message(&msg.msg, &parent,
                                 parent_etx, rtmetric,
                                 num_neighbors, beacon_interval);

 // uip_udp_packet_sendto(client_conn, &msg, sizeof(msg),
                    //    &server_ipaddr, UIP_HTONS(UDP_SERVER_PORT));
  print_data(&msg,sizeof(msg));
}
/*---------------------------------------------------------------------------*/
static void
timed_out(void *ptr)
{
	printf("Last Operation timed out\n");
    state = IDLE;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_client_process, ev, data)
{
  PROCESS_BEGIN();

  PROCESS_PAUSE();
  SENSORS_ACTIVATE(button_sensor);
  uart1_set_input(serial_line_input_byte);
  serial_line_init();
  set_global_address();
  print_local_addresses();
  /* The data sink runs with a 100% duty cycle in order to ensure high
     packet reception rates. */
  NETSTACK_MAC.off(1);

  in_conn = udp_new(NULL, UIP_HTONS(UDP_CLIENT_PORT), NULL);
  udp_bind(in_conn, UIP_HTONS(UDP_SERVER_PORT));

  PRINTF("Listening for connections on ");
  PRINT6ADDR(&in_conn->ripaddr);
  PRINTF(" local/remote port %u/%u\n",
	UIP_HTONS(in_conn->lport), UIP_HTONS(in_conn->rport));

#if WITH_GLOBAL_DAG
  if(own_ipaddr.u8[15] == 1) {
    rpl_set_root(&own_ipaddr);
  }
#endif /* WITH_GLOBAL_DAG */

  state = IDLE;
  printf("Ready\n");
  while(1) {
    char input[10];
    char command;

    uint8_t target_id=0,temp1;
    PROCESS_YIELD();
    /* UDP packet received */
    printf("Event received\n");
    if(ev == serial_line_event_message) {
    	strcpy(input,(char *)data);
        printf("received line: %s\n", input);


        switch(input[0])
        {
        	case ('m'):
        			target_id= atoi(input+2);
        			printf("Measuring distance to %d\n",target_id);
        			set_server_address(target_id);
					PRINTF("[%d] Measuring route to ", state);
					PRINT6ADDR(&remote_ipaddr);
					PRINTF("\n");
					state = MEASURE_BEFORE;
			        ctimer_set(&timeout, CLOCK_SECOND * 60, timed_out, NULL);
			        mo_output(&remote_ipaddr);
			        break;
        	case ('n'):
				    numberOfNodes = atoi(input+2);
        			break;
        	case ('p'):
					target_id= atoi(input+2);
        			set_server_address(target_id);
					send_udp_ping();
        			state = PING;
        			break;
        	case ('r'):
				target_id= atoi(input+2);
        		set_server_address(target_id);
                PRINTF("Sending P2P route request to target %d ", target_id);
                PRINT6ADDR(&remote_ipaddr);
                PRINTF("\n");
                state = DISCOVER;
                ctimer_set(&timeout, CLOCK_SECOND * 60, timed_out, NULL);
                rpl_discover_route(&remote_ipaddr);
                break;
        	case ('c'):
					collect_common_send();
        			break;
        	case ('a'):
					target_id= atoi(input+2);
        			if ( checkcache(target_id) == 1)
        			{
        				printf("Cache entry to %d :Valid",target_id);

        			}
        			else
        			{
        	        	printf("Cache entry to %d :Invalid",target_id);

        	        }
        			break;
        }

    }
    else if(ev == tcpip_event) {
      tcpip_handler();
    /* Button pressed */
    } else if(ev == rpl_p2p_event_mo) {
        ctimer_stop(&timeout);
        printf("ETX for route to %d %d ", target_id,10);
        print6addr(&((rpl_mo_result_t *)data)->target);
        printf(" is - %u.%u\n", ((rpl_mo_result_t *)data)->path_metric /
            RPL_DAG_MC_ETX_DIVISOR, (((rpl_mo_result_t *)data)->path_metric %
            RPL_DAG_MC_ETX_DIVISOR * 100) / RPL_DAG_MC_ETX_DIVISOR);
        state=IDLE;

    }else if(ev == rpl_p2p_event_dro) {
    	ctimer_stop(&timeout);
    	printf("Received DRO - from app\n");
    	savetocache(savetocache);
    	state=IDLE;
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
