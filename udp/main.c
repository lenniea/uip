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
#include "uip.h"
#include "uip_arp.h"
#include "oc_ether.h"

#ifdef _WEBSERVER
  #include "httpd.h"
#endif
#ifdef _UDPSERVER
  #include "udpserver.h"
#endif

#include "timer.h"

//#define _DEBUG  1

#ifdef _DEBUG
  #include <stdio.h>
  #define TRACE(s)      printf(s)
  #define TRACE1(s,a)   printf(s, a)
#else
  #define TRACE(s)
  #define TRACE1(s,a)
#endif

#define BUF ((struct uip_eth_hdr *)&uip_buf[0])

#ifndef NULL
#define NULL (void *)0
#endif /* NULL */

#if defined(_MICO32)

/* copied from fpgareg.h */
#define DIGITAL_OUT_REG         0x14U /* [R/W] see DIGITAL_OUT_FORMAT */
#define digital_out             (*(volatile uint32_t*) (FPGA_BASE + DIGITAL_OUT_REG))
#define XP_ENABLE_MASK        	0x0700U

#define WATCH_DOG_RESET       0xDEAD    /* [W/O] Watch Dog Reset */
#define WATCH_DOG_REG         0x1C      /* [R/W] See WATCH_DOG_CONST */
#define watch_dog             (*(volatile uint32_t*) (FPGA_BASE + WATCH_DOG_REG))

#define SET_XP_OUT(x)           (digital_out = (digital_out & ~XP_ENABLE_MASK) | ((x) << 8))
		
typedef unsigned int size_t;

void* malloc(size_t size)
{
    return NULL;
}

void free(void* ptr)
{
}

int atexit( void (*func )( void ) )
{
    return 0;
}

void *memcpy(void* dst, const void* src, size_t count)
{
	register void* p = (short*) dst;
    if ((((uint32_t) p | (uint32_t) src | count) & 3) == 0)
    {
        while (((int32_t) (count -= 4)) >= 0)
        {
            *(int32_t*) p = *(const int32_t*) src;
            src = (const int32_t*) src + 1;
            p = (int32_t*) p + 1;
        }
    }
	else
	{
		while (((int32_t) (count -= 2)) >= 0)
		{
			*(int16_t*) p = *(const int16_t*) src;
			src = (const int16_t*) src + 1;
			p = (int16_t*) p + 1;
		}
	}
	return dst;
}

void *memset(void* p, int c, size_t count)
{
	count = (count + 1) & ~1;
    while (count > 1) {
	  *(short*) p = c;
	  p = (short*) p + 1;
	  count -= sizeof(short);
	}
    return p;
}

void exit( int status )
{
    watch_dog = WATCH_DOG_RESET;
}

void forceReset(void)
{
    watch_dog = WATCH_DOG_RESET;
}

/* Copied and modified from stub.c */
void breakpoint_handler(void)
{
    TRACE("Breakpoint Handler!\n");
#ifdef _32BIT_FLASH_ADDR
    SPI_FourByteDisable();
#endif
    forceReset();
}

void instruction_bus_error_handler(void)
{
    TRACE("Instruction Bus Error!\n");
#ifdef _32BIT_FLASH_ADDR
    SPI_FourByteDisable();
#endif
    forceReset();
}

void watchpoint_handler(void)
{
    TRACE("Watchpoint tripped!\n");
#ifdef _32BIT_FLASH_ADDR
    SPI_FourByteDisable();
#endif
    forceReset();
}

void data_bus_error_handler(void)
{
    TRACE("Data Bus Error!\n");
#ifdef _32BIT_FLASH_ADDR
    SPI_FourByteDisable();
#endif
    forceReset();
}

void divide_by_zero_handler(void)
{
    TRACE("Divide By Zero Handler!\n");
#ifdef _32BIT_FLASH_ADDR
    SPI_FourByteDisable();
#endif
    forceReset();
}

void wait(unsigned short msec)
{
	clock_time_t now = clock_time();
	const clock_time_t delay = msec * (CLOCK_SECOND / 1000);
	clock_time_t elapsed;
	do
		elapsed = clock_time() - now;
	while (elapsed < delay);
}

#endif

/*---------------------------------------------------------------------------*/
const struct uip_eth_addr mac = {
//  { 0x9c,0xeb,0xe8,0x07,0x3a,0x13 }
  { 0x00,0x40,0x7F,0x40,0x00,0x5E }
};

int
main(int argc, const char* argv[])
{
  int i;
  
  /* Initialize XP_BUS mode for ethernet! */
  SET_XP_OUT(5);
  wait(1000);
  
  uip_ipaddr_t ipaddr;
  struct timer periodic_timer, arp_timer;
  
  uip_setethaddr(mac);

  timer_set(&periodic_timer, CLOCK_SECOND / 2);
  timer_set(&arp_timer, CLOCK_SECOND * 10);
  
  eth_init(mac.addr);
  
  uip_init();

  /* Set & display host IP address */
  uip_ipaddr(ipaddr, 192,168,1,4);
  uip_sethostaddr(ipaddr);
#ifdef _DEBUG
  printf("host ip=%d.%d.%d.%d\n", 
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
#ifdef _UDPSERVER
  udpserver_init();
#endif

  for (;;)
  {
    uip_len = eth_rx(uip_buf, UIP_BUFSIZE);
    if(uip_len > 0) {
      if(BUF->type == HTONS(UIP_ETHTYPE_IP)) {
	uip_input();
	/* If the above function invocation resulted in data that
	   should be sent out on the network, the global variable
	   uip_len is set to a value > 0. */
	if(uip_len > 0) {
	  eth_tx(uip_buf, uip_len);
	}
      }
	  else if(BUF->type == HTONS(UIP_ETHTYPE_ARP)) {
	uip_arp_arpin();
	/* If the above function invocation resulted in data that
	   should be sent out on the network, the global variable
	   uip_len is set to a value > 0. */
	if(uip_len > 0) {
	  eth_tx(uip_buf, uip_len);
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
	  eth_tx(uip_buf, uip_len);
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
	  eth_tx(uip_buf, uip_len);
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
  TRACE1("uIP log message: %s\n", m);
}
