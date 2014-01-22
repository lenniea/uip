#include <stdio.h>
#include "udpcam.h"
#include "uip.h"
#include "udpserver.h"

#ifdef _DEBUG
  #define DEBUG_PRINTF(s)       fprintf(stderr, s)
  #define DEBUG_PRINTF1(s,a)    fprintf(stderr, s,a)
  #define DEBUG_PRINTF2(s,a,b)  fprintf(stderr, s,a,b)
#else
  #define DEBUG_PRINTF(s)
  #define DEBUG_PRINTF1(s,a)
  #define DEBUG_PRINTF2(s,a,b)
#endif

#define UDP_SERVER_PORT		8080

static struct udpserver_state server;

void udpserver_init(void) {
  uip_ipaddr_t addr;
  
  server.state = WELCOME_SENT;

  /* Listen to any incoming address, any port */
  uip_ipaddr(addr, 255,255,255,255);
  server.conn = uip_udp_new(&addr, 0);
  if(server.conn != NULL) {
    uip_udp_bind(server.conn, HTONS(UDP_SERVER_PORT));
  }
}

extern void *uip_sappdata;

void udpserver_app(void) {

  if(uip_newdata()) {
   
    /* Process UDP command */
    short* data = (short*) uip_appdata;
    short func = ntohs(data[0]);
    short count = uip_len - sizeof(short);
    short result = CamCommand(func, count, data + 1);

    DEBUG_PRINTF1("=d\n", result);

    data[0] = htons(result);
    uip_udp_send((result < 0) ? sizeof(short) : result + sizeof(short));
  }
}
