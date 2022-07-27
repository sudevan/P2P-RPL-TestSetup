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
 *   Example how to use the IPv6 Routing Header for Source Routes with RPL
 *                   draft-ietf-6man-rpl-routing-header-03
 *
 * Generation of IPv6 addresses from node IDs is hard-coded and requires
 * the mac address to be 02:00:00:00:00:00:00:<node-id> this can be achieved by
 * setting '#define IEEE_802154_MAC_ADDRESS {2,0,0,0,0,0,0,0}' in
 * platform/xxx/contiki-conf.h or via CFLAGS in the Makefile
 *
 * \author Matthias Philipp <matthias-philipp@gmx.de>
 *
 */

#include "contiki.h"
#include "lib/random.h"
#include "sys/ctimer.h"
#include "net/uip.h"
#include "net/uip-ds6.h"
#include "net/uip-udp-packet.h"
#include "sys/ctimer.h"
#include "dev/button-sensor.h"

#include "net/netstack.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define UDP_CLIENT_PORT 0xcccc
#define UDP_SERVER_PORT 0xbbbb

#define DEBUG DEBUG_FULL
#include "net/uip-debug.h"

#define SEND_INTERVAL		(60 * CLOCK_SECOND)
#define SEND_TIME		(random_rand() % (SEND_INTERVAL))
#define MAX_PAYLOAD_LEN		30

#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])

#define NUM_NODES 5
#define MAX_PINGS 3

static struct uip_udp_conn *out_conn;
static struct uip_udp_conn *in_conn;
static uip_ipaddr_t remote_ipaddr;
static uip_ipaddr_t own_ipaddr;
static uint8_t pings_sent = 0;
static struct ctimer color_timer;
static uint8_t recv_seq;
static struct ctimer pong_timer;
/*---------------------------------------------------------------------------*/
PROCESS(udp_client_process, "UDP client process");
AUTOSTART_PROCESSES(&udp_client_process);
/*---------------------------------------------------------------------------*/
static void
send_udp_ping()
{
  char buf[MAX_PAYLOAD_LEN];

  ANNOTATE("#A S=ping#%u->%u\n", pings_sent, remote_ipaddr.u8[15]);
  sprintf(buf, "ping %d", pings_sent);
  PRINTF("Sending '%s' to %d\n", buf, remote_ipaddr.u8[15]);
  uip_udp_packet_sendto(out_conn, buf, strlen(buf),
                        &remote_ipaddr, UIP_HTONS(UDP_SERVER_PORT));
}
/*---------------------------------------------------------------------------*/
static void
send_udp_pong(void *ptr)
{
  char buf[MAX_PAYLOAD_LEN];

  ANNOTATE("#A S=pong#%u->%u\n", recv_seq, in_conn->ripaddr.u8[15]);
  sprintf(buf, "pong %d", recv_seq);
  PRINTF("Sending '%s' to %d\n", buf, in_conn->ripaddr.u8[15]);
  uip_udp_packet_send(in_conn, buf, strlen(buf));
  memset(&in_conn->ripaddr, 0, 16);
}
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

  if(uip_newdata()) {
    str = uip_appdata;
    str[uip_datalen()] = '\0';
    if(strncmp(str, "ping", 4) == 0 || strncmp(str, "pong", 4) == 0) {
      recv_seq = atoi(&str[4]);
      PRINTF("Received '%s' from %d\n", str, UIP_IP_BUF->srcipaddr.u8[15]);
      str[4] = 0;
      ANNOTATE("#A color=red\n");
      ANNOTATE("#A R=%s#%u<-%u\n", str, recv_seq, UIP_IP_BUF->srcipaddr.u8[15]);
      ctimer_set(&color_timer, CLOCK_SECOND*4, reset_color, NULL);
      if(strncmp(str, "ping", 4) == 0) {
        uip_ipaddr_copy(&in_conn->ripaddr, &UIP_IP_BUF->srcipaddr);
        in_conn->rport = UIP_HTONS(UDP_CLIENT_PORT);
        ctimer_set(&pong_timer, CLOCK_SECOND*4, send_udp_pong, NULL);
      }
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
set_server_address(void)
{
  uint8_t target_id;

  /* Omit own address and the current server */
  do {
    target_id = (random_rand() % NUM_NODES) + 1;
  } while(target_id == remote_ipaddr.u8[15] ||\
          target_id == own_ipaddr.u8[15]);
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
/*---------------------------------------------------------------------------*/
static void
set_routes(void)
{
  uip_ds6_src_route_t *srcrt;
  uip_ipaddr_t ip;

  uint8_t target;
  uint8_t i;
  int add;

  /* Generate static routes for simple chain topology */
  for(target = 1; target <= NUM_NODES; target++) {
    if(target == own_ipaddr.u8[15]) {
      continue;
    }
    if(target < own_ipaddr.u8[15]) {
      add = -1;
    } else {
      add = 1;
    }
    uip_ipaddr_copy(&ip, &own_ipaddr);
    ip.u8[15] = target;
    /* Create source route to target */
    if((srcrt = uip_ds6_src_route_add(&ip)) == NULL) {
      PRINTF("ERROR: No memory to store source route\n");
      return;
    }
    PRINTF("Source route to ");
    PRINT6ADDR(&ip);
    PRINTF(" is: ");
    /* Add intermediate hops */
    for(i = own_ipaddr.u8[15] + add; i != target; i += add) {
      ip.u8[15] = i;
      uip_create_linklocal_prefix(&ip);
      if(uip_ds6_src_route_hop_add(srcrt, &ip, 0) == NULL) {
        PRINTF("ERROR: No memory to store source route hop\n");
        return;
      }
      PRINT6ADDR(&ip);
      PRINTF(" - ");
    }
    /* Since neighbor discovery does not work, we need to add the link-local
     * address of the target as last hop. */
    ip.u8[15] = target;
    uip_create_linklocal_prefix(&ip);
    if(uip_ds6_src_route_hop_add(srcrt, &ip, 0) == NULL) {
      PRINTF("ERROR: No memory to store source route hop\n");
      return;
    }
    PRINT6ADDR(&ip);
    PRINTF("\n");
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_client_process, ev, data)
{
  PROCESS_BEGIN();

  PROCESS_PAUSE();
  SENSORS_ACTIVATE(button_sensor);

  set_global_address();
  print_local_addresses();
  set_server_address();
  set_routes();
  /* The data sink runs with a 100% duty cycle in order to ensure high
     packet reception rates. */
  NETSTACK_MAC.off(1);

  in_conn = udp_new(NULL, UIP_HTONS(UDP_CLIENT_PORT), NULL);
  udp_bind(in_conn, UIP_HTONS(UDP_SERVER_PORT));

  PRINTF("Listening for connections on ");
  PRINT6ADDR(&in_conn->ripaddr);
  PRINTF(" local/remote port %u/%u\n",
	UIP_HTONS(in_conn->lport), UIP_HTONS(in_conn->rport));

  while(1) {
    PROCESS_YIELD();
    if(ev == tcpip_event) {
      tcpip_handler();
    } else if(ev == sensors_event && data == &button_sensor) {
      if(pings_sent >= MAX_PINGS) {
        set_server_address();
        pings_sent = 0;
      }
      pings_sent++;
      send_udp_ping();
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
