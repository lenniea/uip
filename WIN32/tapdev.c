/*
 * Copyright (c) 2001, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
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
 * Author: Adam Dunkels <adam@sics.se>
 *
 * $Id: tapdev.c,v 1.8 2006/06/07 08:39:58 adam Exp $
 */
 
#include <pcap.h>
#define htons	htons			/* avoid redefining htons error */
#include "uip.h"

#ifdef _DEBUG
  #define _DEBUG_RX     1
  #define _DEBUG_TX     1
#endif

static pcap_t *pcap_fp;

/*---------------------------------------------------------------------------*/
void
tapdev_init(const char* eth_dev)
{
  char errbuf[PCAP_ERRBUF_SIZE];
	
  pcap_fp = pcap_open_live(
				eth_dev,	/* name of the device */
				65536,		/* portion of the packet to capture. It doesn't matter in this case */
				1,			/* promiscuous mode (nonzero means promiscuous) */
				1,			/* read timeout */
				errbuf		/* error buffer */
				);
}
/*---------------------------------------------------------------------------*/
unsigned int
tapdev_read(void)
{
  struct pcap_pkthdr *header;
  const u_char *pkt_data;

  int ret = pcap_next_ex( pcap_fp, &header, &pkt_data);
  if (ret < 0) {
#ifdef _DEBUG_RX
    fprintf(stderr, "tapdev_read: error %d", ret);
#endif
	ret = 0;
  }
  else if (ret > 0)
  {
    /* copy packet data into uip_buf */
    ret = header->len;
    memcpy(uip_buf, pkt_data, ret);
#ifdef _DEBUG_RX
    fprintf(stderr, "tapdev_read: read %d bytes\n", ret);
    {
      int i;
      for(i = 0; i < 20; i++) {
        fprintf(stderr, "%02x ", uip_buf[i]);
      }
      fprintf(stderr, "\n");
    }
#endif
  }
  return ret;
}
/*---------------------------------------------------------------------------*/
void
tapdev_send(void)
{
  int ret = pcap_sendpacket(
				pcap_fp,	/* Adapter */
				uip_buf, 	/* buffer with the packet */
				uip_len 	/* size */
				);
#ifdef _DEBUG_TX
    fprintf(stderr, "tapdev_send: send %d bytes\n",  uip_len);
    {
      int i;
      for(i = 0; i < 20; i++) {
        fprintf(stderr, "%02x ", uip_buf[i]);
      }
      fprintf(stderr, "\nret = %d\n", ret);
    }
#endif
}
/*---------------------------------------------------------------------------*/
