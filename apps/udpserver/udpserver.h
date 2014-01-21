/**
 * \addtogroup udpserver
 * @{
 */

/**
 * \file
 * Header file for the UDP server.
 * \author Lennie Araki (lennie.araki@gmail.com)
 */
 
#ifndef __UDPSERVER_H__
#define __UDPSERVER_H__

struct udpserver_state {
  enum {WELCOME_SENT, WELCOME_ACKED} state;
  struct uip_udp_conn *conn;
};

void udpserver_init(void);
void udpserver_request(void);

void udpserver_app(void);

typedef struct udpserver_state uip_udp_appstate_t;
typedef struct udpserver_state uip_tcp_appstate_t;
#define UIP_UDP_APPCALL udpserver_app

#endif /* __UDPSERVER_H__ */
