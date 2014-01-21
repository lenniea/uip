/*
 * Copyright (c) 2001, Adam Dunkels.
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
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by Adam Dunkels.
 * 4. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the uIP TCP/IP stack.
 *
 * $Id: main.c,v 1.16 2006/06/11 21:55:03 adam Exp $
 *
 */
#define htons       htons
#include <winsock2.h>
#include "uip.h"
#include "uip_arp.h"
#include "tapdev.h"

#include "timer.h"

#ifdef _DEBUG
  #include <stdio.h>
  #define DEBUG_PRINTF(s)      fprintf(stderr, s)
  #define DEBUG_PRINTF2(s,a)   fprintf(stderr, s, a)
#else
  #define DEBUG_PRINTF(s)
  #define DEBUG_PRINTF2(s,a)
#endif

#define BUF ((struct uip_eth_hdr *)&uip_buf[0])

#ifndef NULL
#define NULL (void *)0
#endif /* NULL */

/*---------------------------------------------------------------------------*/
int
main(int argc, const char* argv[])
{
  int i;
  uip_ipaddr_t ipaddr;
  struct timer periodic_timer, arp_timer;
  const char* eth_device =
//	"\\Device\\NPF_{76F14800-4F5C-4643-99C3-F268778028B9}";
//	"\\Device\\NPF_{CF312719-23B2-42CC-8A8E-2EED678F1919}";
	"\\Device\\NPF_{D4B8A900-9343-4C34-8C80-D0C4A512690C}";
  char hostname[80];
  const struct uip_eth_addr mac = { { 0x9c,0xeb,0xe8,0x07,0x3a,0x13 } };
  uip_setethaddr(mac);

  timer_set(&periodic_timer, CLOCK_SECOND / 2);
  timer_set(&arp_timer, CLOCK_SECOND * 10);
  
  if (argc > 1)
  {
    eth_device = argv[1];
  }
  
  tapdev_init(eth_device);
  uip_init();

  /* Set default IP address */
  uip_ipaddr(ipaddr, 192,168,1,9);
  if (gethostname(hostname, sizeof(hostname)) == 0)
  {
    HOSTENT* he = gethostbyname(hostname);
    if (he != NULL)
    {
      unsigned char* ip_addr = he->h_addr_list[0];
      uip_ipaddr(ipaddr, ip_addr[0], ip_addr[1], ip_addr[2], ip_addr[3]);
    }
  }

  /* Set & display host IP address */
  uip_sethostaddr(ipaddr);
#ifdef _DEBUG
  fprintf(stderr, "host ip=%d.%d.%d.%d\n", 
          uip_ipaddr1(ipaddr), uip_ipaddr2(ipaddr),
          uip_ipaddr3(ipaddr), uip_ipaddr4(ipaddr));
#endif
  uip_ipaddr(ipaddr, 192,168,1,1);
  uip_setdraddr(ipaddr);
  uip_ipaddr(ipaddr, 255,255,255,0);
  uip_setnetmask(ipaddr);

#ifdef _WEBSERVER
  httpd_init();
#endif
  
#ifdef __TELNET_H__
  telnetd_init();
#endif
  
  /*  hello_world_init();*/

#ifdef __SMTP_H__
  {
      u8_t mac[6] = {1,2,3,4,5,6};
      dhcpc_init(&mac, 6);
  }
  
  uip_ipaddr(ipaddr, 127,0,0,1);
  smtp_configure("localhost", ipaddr);
  SMTP_SEND("adam@sics.se", NULL, "uip-testing@example.com",
	    "Testing SMTP from uIP",
	    "Test message sent by uIP\r\n");
#endif

#ifdef _WEBCLIENT
    webclient_init();
    resolv_init();
    uip_ipaddr(ipaddr, 195,54,122,204);
    resolv_conf(ipaddr);
    resolv_query("www.sics.se");
#endif

#ifdef _UDPSERVER
	udpserver_init();
#endif

  
  while(1) {
    uip_len = tapdev_read();
    if(uip_len > 0) {
      if(BUF->type == HTONS(UIP_ETHTYPE_IP)) {
#ifndef _UDPSERVER
	uip_arp_ipin();
#endif
	uip_input();
	/* If the above function invocation resulted in data that
	   should be sent out on the network, the global variable
	   uip_len is set to a value > 0. */
	if(uip_len > 0) {
#ifndef _UDPSERVER
	  uip_arp_out();
#endif
	  tapdev_send();
	}
      } else if(BUF->type == HTONS(UIP_ETHTYPE_ARP)) {
	uip_arp_arpin();
	/* If the above function invocation resulted in data that
	   should be sent out on the network, the global variable
	   uip_len is set to a value > 0. */
	if(uip_len > 0) {
	  tapdev_send();
	}
      }

    } else if(timer_expired(&periodic_timer)) {
      timer_reset(&periodic_timer);
#if UIP_CONNS
      for(i = 0; i < UIP_CONNS; i++) {
	uip_periodic(i);
	/* If the above function invocation resulted in data that
	   should be sent out on the network, the global variable
	   uip_len is set to a value > 0. */
	if(uip_len > 0) {
	  uip_arp_out();
	  tapdev_send();
	}
      }
#endif

#if UIP_UDP
      for(i = 0; i < UIP_UDP_CONNS; i++) {
	uip_udp_periodic(i);
	/* If the above function invocation resulted in data that
	   should be sent out on the network, the global variable
	   uip_len is set to a value > 0. */
	if(uip_len > 0) {
	  uip_arp_out();
	  tapdev_send();
	}
      }
#endif /* UIP_UDP */
      
      /* Call the ARP timer function every 10 seconds. */
      if(timer_expired(&arp_timer)) {
	timer_reset(&arp_timer);
	uip_arp_timer();
      }
    }
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
void
uip_log(char *m)
{
  DEBUG_PRINTF2("uIP log message: %s\n", m);
}
void
resolv_found(char *name, u16_t *ipaddr)
{
  if(ipaddr == NULL) {
    DEBUG_PRINTF2("Host '%s' not found.\n", name);
  } else {
#ifdef _DEBUG
    fprintf(stderr, "Found name '%s' = %d.%d.%d.%d\n", name,
	   HTONS(ipaddr[0]) >> 8,
	   HTONS(ipaddr[0]) & 0xff,
	   HTONS(ipaddr[1]) >> 8,
	   HTONS(ipaddr[1]) & 0xff);
#endif
    /*    webclient_get("www.sics.se", 80, "/~adam/uip");*/
  }
}
#ifdef __DHCPC_H__
void
dhcpc_configured(const struct dhcpc_state *s)
{
  uip_sethostaddr(s->ipaddr);
  uip_setnetmask(s->netmask);
  uip_setdraddr(s->default_router);
  resolv_conf(s->dnsaddr);
}
#endif /* __DHCPC_H__ */
void
smtp_done(unsigned char code)
{
  DEBUG_PRINTF2("SMTP done with code %d\n", code);
}
void
webclient_closed(void)
{
  DEBUG_PRINTF("Webclient: connection closed\n");
}
void
webclient_aborted(void)
{
  DEBUG_PRINTF("Webclient: connection aborted\n");
}
void
webclient_timedout(void)
{
  DEBUG_PRINTF("Webclient: connection timed out\n");
}
void
webclient_connected(void)
{
  DEBUG_PRINTF("Webclient: connected, waiting for data...\n");
}
void
webclient_datahandler(char *data, u16_t len)
{
  DEBUG_PRINTF2("Webclient: got %d bytes of data.\n", len);
}
/*---------------------------------------------------------------------------*/
