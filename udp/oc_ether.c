//--------------------------------------------------------
// Copyright (c) 2012 by Dynalith Systems Co., Ltd.
// All right reserved.
//
// http://www.dynalith.com
//--------------------------------------------------------
// oc_ether.c
//--------------------------------------------------------
// VERSION = 2012.06.16.
//--------------------------------------------------------
// OpenCores Ethernet MAC driver
//--------------------------------------------------------
// includes
#include <stdint.h>
#include <string.h>
#include "oc_ether.h"

//--------------------------------------------------------
#ifdef TRX_BFM
#define MEM_READ32(A,B)		BfmRead32((A),(unsigned int*)&(B))
#define MEM_WRITE32(A,B)	BfmWrite32((A),(B))
#else
#define MEM_READ32(A,B)		(B) = *((volatile uint32_t*) (A))
#define MEM_WRITE32(A,B)	*((volatile uint32_t*) (A)) = (B)
#define PRINT_SIMPLE
#endif

//#define _DEBUG		1

#ifdef _DEBUG
    #include <stdio.h>
    typedef int (*FUNC_PTR)(void);

    #define DEBUG_PRINTF(s)			printf(s)
    #define DEBUG_PRINTF1(s,a)			printf(s,a)
    #define DEBUG_PRINTF2(s,a,b)		printf(s,a,b)
    #define DEBUG_PRINTF3(s,a,b,c)		printf(s,a,b,c)
    #define DEBUG_PRINTF4(s,a,b,c,d)		printf(s,a,b,c,d)
    #define DEBUG_PRINTF5(s,a,b,c,d,e)		printf(s,a,b,c,d,e)
    #define DEBUG_PRINTF6(s,a,b,c,d,e,f)        printf(s,a,b,c,d,e,f)
#else
    #define DEBUG_PRINTF(s)
    #define DEBUG_PRINTF1(s,a)
    #define DEBUG_PRINTF2(s,a,b)
    #define DEBUG_PRINTF3(s,a,b,c)
    #define DEBUG_PRINTF4(s,a,b,c,d)
    #define DEBUG_PRINTF5(s,a,b,c,d,e)
    #define DEBUG_PRINTF6(s,a,b,c,d,e,f)
#endif

//--------------------------------------------------------
// Byte ordering macros

#if defined(BIG_ENDIAN_MACHINE) || defined(ENDIAN_BIG_MACHINE)
#	ifndef HTONS
#       	define HTONS(A) (A)
#	endif
#	ifndef HTONL
#       	define HTONL(A) (A)
#	endif
#	ifndef NTOHS
#       	define NTOHS(A) (A)
#	endif
#	ifndef NTOHL
#       	define NTOHL(A) (A)
#	endif
#elif defined(LITTLE_ENDIAN_MACHINE) || defined(ENDIAN_LITTLE_MACHINE)
#	ifndef HTONS
#       define HTONS(A) ((((uint16_t)(A) & 0xFF00) >> 8) |\
                         (((uint16_t)(A) & 0x00FF) << 8))
#	endif
#	ifndef HTONL
#       define HTONL(A) ((((uint32_t)(A) & 0xFF000000) >> 24) |\
                         (((uint32_t)(A) & 0x00FF0000) >>  8) |\
                         (((uint32_t)(A) & 0x0000FF00) <<  8) |\
                         (((uint32_t)(A) & 0x000000FF) << 24))
#	endif
#	ifndef NTOHS
#       	define NTOHS HTONS
#	endif
#	ifndef NTOHL
#       	define NTOHL HTONL
#	endif
#else
#       error BYTE_ORDER should be defined
#endif

#ifdef _DEBUG
//--------------------------------------------------------
// should use -fpack-struct for GCC
//--------------------------------------------------------
// Ethernet packet frame
#ifdef _MSC_VER
# pragma pack (1)
#endif
typedef struct _ether_frame_t {
  uint8_t  dst[6];
  uint8_t  src[6];
  uint16_t type; // leng when type <= 1500
  uint8_t  data; // can be IP packet
#ifdef _MSC_VER
} ether_frame_t;
# pragma pack ()
#else
}  __attribute__((packed)) ether_frame_t;
#endif
//--------------------------------------------------------
// IP packet frame within Ethernet frame
#ifdef _MSC_VER
# pragma pack (1)
#endif
struct _ip_frame_t {
  uint8_t  ver_hlen;
  uint8_t  type_of_service;
  uint16_t len;
  uint16_t id;
  uint16_t flag_offset;
  uint8_t  tlive;
  uint8_t  protocol;
  uint16_t check_sum;
  uint8_t  src_ip[4];
  uint8_t  dst_ip[4];
  uint8_t  options;
#ifdef _MSC_VER
};
# pragma pack ()
#else
} __attribute__((packed));
#endif
//--------------------------------------------------------
// ARP packet frame within Ethernet frame
#ifdef _MSC_VER
# pragma pack (1)
#endif
typedef struct _arp_frame_t {
  uint16_t hw_type;
  uint16_t pro_type;
  uint8_t  hw_len;
  uint8_t  pro_len;
  uint16_t msg_type; // 1 for req; 2 for resp
  uint8_t  src_mac[6]; // mac
  uint8_t  src_ip[4];  // IP
  uint8_t  dst_mac[6];
  uint8_t  dst_ip[4];  // IP
#ifdef _MSC_VER
} arp_frame_t;
# pragma pack ()
#else
} __attribute__((packed)) arp_frame_t;
#endif

#endif /* _DEBUG */

//--------------------------------------------------------
// Buffer descriptor in the CSR
//                     +------------+
//                     |            | __
//                     +------------+  |
//                     |            |  |
//                     +------------+  ETH_RXBD_NUM*8
//                     |            |  |
//                     +------------+  |
//                     |            | __
//                     +------------+  |
//                     |            |  |
//                     +------------+  ETH_TXBD_NUM*8
//                     |            |  |
//                     +------------+  |
// ETH_BD_TX_BASE  --> |            | --
//                     +------------+
//--------------------------------------------------------
//               Buffer
//              +------------+
//              |            | __
//              +------------+  |
//              |            |  |
//              +------------+  ETH_RXBD_NUM*ETH_MAXBUF_LEN
//              |            |  |
//              +------------+  |
//              |            | __
//              +------------+  |
//              |            |  |
//              +------------+  ETH_TXBD_NUM*ETH_MAXBUF_LEN
//              |            |  |
//              +------------+  |
// eth_data --> |            | --
//              +------------+
//--------------------------------------------------------
// global variables
#define ETH_DATA_BUF_SIZE ((ETH_TXBD_NUM + ETH_RXBD_NUM) * ETH_MAXBUF_LEN)

extern uint32_t eth_data[ETH_DATA_BUF_SIZE/4];
#undef  ETH_DATA_BASE
#define ETH_DATA_BASE ((uint32_t)eth_data)

//--------------------------------------------------------
// local variables
int tx_next; /* Next buffer to be given to the user */
int tx_last; /* Next buffer to be checked if packet sent */
int tx_full;
int rx_next; /* Next buffer to be checked for new packet and given to the user */

//--------------------------------------------------------
// static function prototypes
static void init_rx_bd_pool(void);
static void init_tx_bd_pool(void);
static void phy_check(uint8_t phy_add);
static void compare16(char* str, uint16_t val, uint16_t expect);
static void compare32(char* str, uint32_t val, uint32_t expect);

//--------------------------------------------------------
#ifdef TRX_BFM
// copy data in the hardware memroy region (in) to the host memory region (out).
// returns a pointer to the first byte of the out region.
void memcpy_get(void *out, void *in, unsigned int nbytes) {
      unsigned int to       = (unsigned int)out;
      unsigned int from     = (unsigned int)in ;
      unsigned int done     = 0;
      unsigned int nwords   = nbytes/4;
      unsigned int nremains = nbytes%4;
      BfmRead(from, (unsigned int *)to, 4, nwords);
      to   += (nwords*4);
      from += (nwords*4);
      if (nremains) {
          unsigned int data;
          unsigned char *cp=(unsigned char*)to;
          BfmRead(from, (unsigned int *)&data, nremains, 1);
          // It is assumed that hardware bus works as little-endian fashion,
          // so lower address byte goes lower byte lane.
          switch (nremains) {
          case 1: cp[0] =  data&0xFF; break;
          case 2: cp[0] =  data&0xFF;
                  cp[1] = (data&0xFF00)>>8; break;
          case 3: cp[0] =  data&0xFF;
                  cp[1] = (data&0xFF00)>>8;
                  cp[2] = (data&0xFF0000)>>16; break;
          default: DEBUG_PRINTF1("unexpected value %d\n", nremains);
          }
      }
}
// copy data in the host memroy region (in) to the hardware memory region (out).
// returns a pointer to the first byte of the out region.
void memcpy_put(void *out, void *in, unsigned int nbytes) {
      unsigned int to       = (unsigned int)out;
      unsigned int from     = (unsigned int)in ;
      unsigned int done     = 0;
      unsigned int nwords   = nbytes/4;
      unsigned int nremains = nbytes%4;
      BfmWrite(to, (unsigned int *)from, 4, nwords);
      to   += (nwords*4);
      from += (nwords*4);
      if (nremains) {
          unsigned int   data;
          unsigned char* cp = (unsigned char *)from;
          // It is assumed that hardware bus works as little-endian fashion,
          // so lower address byte goes lower byte lane.
          switch (nremains) {
          case 1: data = (unsigned int)(cp[0]); break;
          case 2: data = (unsigned int)((cp[1]<<8)|cp[0]); break;
          case 3: data = (unsigned int)((cp[2]<<16)|(cp[1]<<8)|cp[0]); break;
          default: DEBUG_PRINTF1("unexpected value %d\n", nremains);
          }
          BfmWrite(to, (unsigned int *)&data, nremains, 1);
      }
}
#endif


#ifdef FPGA_BASE

#include "clock-arch.h"

#define VIDEO_CYCLES_REG    0x10    /* [R/O] range: (0-4G) */
#define WATCH_DOG_REG       0x1C    /* [R/O] range: (0-4G) */

#define video_cycles        (*(volatile unsigned long*) (FPGA_BASE + VIDEO_CYCLES_REG))
#define watch_dog           (*(volatile unsigned long*) (FPGA_BASE + WATCH_DOG_REG))

/*---------------------------------------------------------------------------*/
clock_time_t
clock_time(void)
{
  static unsigned short last_watch_dog = 0x5555;
  last_watch_dog = ~last_watch_dog;
  watch_dog = last_watch_dog;
  return video_cycles;
}

#endif /* FPGA_TIMER */

//--------------------------------------------------------
#define PHY_INIT			1
#define PHY_ADR             0
#define PHY_RESET           0x8000
#define PHY_100MBIT         0x2000
#define PHY_AUTO_NEGOT		0x1200
#define PHY_FULL_DUPLEX     0x0100
void
eth_init (const uint8_t *mac_addr)
{
  uint32_t value;
#if defined(BIG_ENDIAN_MACHINE)
  DEBUG_PRINTF("Big endian\n");
#elif defined(LITTLE_ENDIAN_MACHINE)
  DEBUG_PRINTF("Little endian\n");
#else
  DEBUG_PRINTF("Endian is not defined\n");
#endif

  MEM_WRITE32(ETH_MODER, 0x0); // Disable receiver and transmitt

  // Setting TX BD number
  MEM_WRITE32(ETH_TX_BD_NUM, ETH_TXBD_NUM);
  
  // Initialize RX and TX buffer descriptors
  memset(eth_data, 0, ETH_DATA_BUF_SIZE);
  init_rx_bd_pool();
  init_tx_bd_pool();
  
  // Set min/max packet length
  MEM_WRITE32(ETH_PACKETLEN, 0x00400600);
  // Set IPGT register to recommended value
  MEM_WRITE32(ETH_IPGT, 0x00000012);
  // Set IPGR1 register to recommended value
  MEM_WRITE32(ETH_IPGR1, 0x0000000c);
  // Set IPGR2 register to recommended value
  MEM_WRITE32(ETH_IPGR2, 0x00000012);
  // Set COLLCONF register to recommended value
  MEM_WRITE32(ETH_COLLCONF, 0x000f003f);

#if 0
  MEM_WRITE32(ETH_CTRLMODER, OETH_CTRLMODER_TXFLOW | 
                             OETH_CTRLMODER_RXFLOW);
#else
  // Pause control frames are blocked
  // Received PAUSE control frames are ignored
  // Control frames are not passed to the host
  MEM_WRITE32(ETH_CTRLMODER, 0); // REG32(ETH_CTRLMODER) = 0;
#endif

  // Set local MAC address
  MEM_WRITE32(ETH_MAC_ADDR1, (mac_addr[0]<<8)|mac_addr[1]);
  MEM_WRITE32(ETH_MAC_ADDR0, (mac_addr[2]<<24)|
                             (mac_addr[3]<<16)|
                             (mac_addr[4]<<8)|
                              mac_addr[5]);

  // Clear all pending interrupts
  MEM_WRITE32(ETH_INT_SOURCE, 0xffffffff);
  // Enable interrupt sources
#if 0
  MEM_WRIT32(ETH_INT_MASK, ETH_INT_MASK_TXB  | 
                           ETH_INT_MASK_TXE  | 
                           ETH_INT_MASK_RXF  | 
                           ETH_INT_MASK_RXE  | 
                           ETH_INT_MASK_BUSY | 
                           ETH_INT_MASK_TXC  | 
                           ETH_INT_MASK_RXC);
#else
  MEM_WRITE32(ETH_INT_MASK, 0x00000000);
#endif

//  // Catches all multicast addresses
//  MEM_WRITE32(ETH_HASH_ADDR0, 0xFFFFFFFF); //REG32(ETH_HASH_ADDR0) = 0xFFFFFFFF; // Ando Ki
//  MEM_WRITE32(ETH_HASH_ADDR1, 0xFFFFFFFF); //REG32(ETH_HASH_ADDR1) = 0xFFFFFFFF; // Ando Ki

  value = 
        //| ETH_MODER_NOPRE    //0x00000004 /* No Preamble  */
        //| ETH_MODER_BRO      //0x00000008 /* Reject Broadcast */
        //| ETH_MODER_IAM      //0x00000010 /* Use Individual Hash */
            ETH_MODER_PRO      //0x00000020 /* Promiscuous (receive all) when 1 */
        //| ETH_MODER_IFG      //0x00000040 /* Min. IFG not required */
        //| ETH_MODER_LOOPBCK  //0x00000080 /* Loop Back */
        //| ETH_MODER_NOBCKOF  //0x00000100 /* No Backoff */
        //| ETH_MODER_EXDFREN  //0x00000200 /* Excess Defer */
          | ETH_MODER_FULLD    //0x00000400 /* Full Duplex when 1*/
        //| ETH_MODER_DLYCRCEN //0x00001000 /* Delayed CRC Enable when 1*/
          | ETH_MODER_CRCEN    //0x00002000 /* CRC Enable when 1*/
        //| ETH_MODER_HUGEN    //0x00004000 /* Huge Enable when 1 */
          | ETH_MODER_PAD      //0x00008000 /* Pad Enable - pad added when 1 */
        //| ETH_MODER_RECSMALL //0x00010000 /* Receive Small when 1*/
        ;
  MEM_WRITE32(ETH_MODER, value);

#ifdef PHY_INIT
  /* reset PHY */
  mdio_write(0, PHY_ADR, PHY_RESET);
  
  wait(2000);
  
  /* PHY auto negotiate */
//  mdio_write(0, PHY_ADR, PHY_AUTO_NEGOT | PHY_FULL_DUPLEX);
  /* PHY 100mbit,full duplex */
//  mdio_write(0, PHY_ADR, PHY_100MBIT | PHY_FULL_DUPLEX);
  /* PHY 10mbit,full duplex */
//  mdio_write(0, PHY_ADR, PHY_FULL_DUPLEX);

  /* Set LED to 00=Link/Act+SPEED, no loopback */
  mdio_write(0x1E, PHY_ADR, 0x0000);
  
#endif
  phy_check(PHY_ADR);

  /* finally enable RX,TX */
  value |=
            ETH_MODER_RXEN     //0x00000001 /* Receive Enable  */
          | ETH_MODER_TXEN;     //0x00000002 /* Transmit Enable */
  MEM_WRITE32(ETH_MODER, value); // enable receiver and transmitter
}

//--------------------------------------------------------
// Send a packet at address
// - buf: contains 'len' bytes in big-endian format
// - len: num of bytes including dest mac to the end of payload,
//        except CRC
void
eth_tx(void *buf, uint32_t len)
{
  unsigned int value;

  DEBUG_PRINTF3("eth_tx(0x%x, 0x%x(%d))----------------\n", (uint32_t)buf, len, len);
#ifdef _DEBUG2
  eth_print_packet(buf, len);
#endif

  // Clear all pending interrupts
  MEM_WRITE32(ETH_INT_SOURCE, 0xffffffff);
#if 1
  // copy data in the 'buf' to the TX buffer area in the hardware memory.
  MEM_READ32(ETH_BD_TX_AD(tx_last), value);
  memcpy((void *)value, buf, len);
#else
  MEM_WRITE32(ETH_BD_TX_AD(tx_last),(uint32_t)buf);
#endif
  MEM_READ32 (ETH_BD_TX_LS(tx_last), value);
  
#ifdef _DEBUG
  if (value & ETH_TX_BD_READY)
  {
	((FUNC_PTR)0)();
  }
#endif 
  value = value & ~(ETH_TX_BD_STATS | 0xFFFF0000);
  MEM_WRITE32(ETH_BD_TX_LS(tx_last), value);
  value = value | (len<<16) | (ETH_TX_BD_READY | ETH_TX_BD_IRQ | ETH_TX_BD_PAD | ETH_TX_BD_CRC);
  MEM_WRITE32(ETH_BD_TX_LS(tx_last), value);

  tx_last = (tx_last + 1) & ETH_TXBD_NUM_MASK;
  tx_full = 0;
}

//--------------------------------------------------------
// - buf: contains 'len' bytes in big-endian format
// - len: num of bytes including dest mac to the end of payload,
//        except CRC
void
eth_tx_b(void *buf, uint32_t len)
{
  uint32_t value;
  DEBUG_PRINTF3("eth_tx_b(0x%x, 0x%x(%d))----------------\n", (uint32_t)buf, len, len);
#ifdef DEBUG2
  eth_print_packet(buf, len);
#endif

#ifdef TRX_BFM
  // copy data in the 'buf' to the TX buffer area in the hardware memory.
  MEM_READ32(ETH_BD_TX_AD(tx_last), value);
  memcpy_put((void *)value, buf, len);
#else
  MEM_WRITE32(ETH_BD_TX_AD(tx_last),(uint32_t)buf);
  //memcpy(ETH_BD_TX_AD(tx_last),buf,len);
#endif
  MEM_READ32 (ETH_BD_TX_LS(tx_last), value);
  value = value & ~(ETH_TX_BD_STATS | 0xFFFF0000);
  MEM_WRITE32(ETH_BD_TX_LS(tx_last), value);
  value = value | (len<<16) | (ETH_TX_BD_READY | ETH_TX_BD_IRQ | ETH_TX_BD_PAD | ETH_TX_BD_CRC);
  MEM_WRITE32(ETH_BD_TX_LS(tx_last), value);

  // blocked until MAC reads it
  do { MEM_READ32(ETH_BD_TX_LS(tx_last), value);
  } while (value&ETH_TX_BD_READY);

  tx_last = (tx_last + 1) & ETH_TXBD_NUM_MASK;
  tx_full = 0;
}

//--------------------------------------------------------
// Waits for packet and pass it to the upper layers
// - buf: will contains 'len' bytes in big-endian format
// - len: num of bytes including dest mac to the end of payload,
//        except CRC
// Return: the num of byte had been received
//         0 when nothing has been received

uint32_t
eth_rx(void *buf, uint32_t size)
{
  uint32_t value;
  uint32_t len_status;
  uint32_t len = 0;

#ifdef DEBUG2
  DEBUG_PRINTF2("eth_rx(%8x,%u)----------------\n", buf, size);
#endif

  len = 0;
    MEM_READ32(ETH_BD_RX_LS(rx_next), len_status);
    if (len_status & ETH_RX_BD_EMPTY) return len;
    if ((len_status>>16)==0) return len;
	DEBUG_PRINTF3("len_status[%u]=%08x size=%u\n", rx_next, len_status, size);
    if (!(len_status & (ETH_RX_BD_OVERRUN |
                        ETH_RX_BD_INVSIMB |
                        ETH_RX_BD_DRIBBLE |
                        ETH_RX_BD_TOOLONG |
                        ETH_RX_BD_SHORT   |
                        ETH_RX_BD_CRCERR  |
                        ETH_RX_BD_LATECOL)))
    {
      uint16_t lx;
      MEM_READ32(ETH_BD_RX_LS(rx_next), value);
      lx = value>>16;
      MEM_READ32(ETH_BD_RX_AD(rx_next), value);
#ifdef TRX_BFM
      memcpy_get(buf, (void*)value, lx);
#else
      memcpy(buf, (void*)value, lx);
#endif
      len += lx;
    }
#ifdef RIGOR
    else {
    if(len_status & ETH_RX_BD_OVERRUN) DEBUG_PRINTF("eth_rx: ETH_RX_BD_OVERRUN\n");
    if(len_status & ETH_RX_BD_INVSIMB) DEBUG_PRINTF("eth_rx: ETH_RX_BD_INVSIMB\n");
    if(len_status & ETH_RX_BD_DRIBBLE) DEBUG_PRINTF("eth_rx: ETH_RX_BD_DRIBBLE\n");
    if(len_status & ETH_RX_BD_TOOLONG) DEBUG_PRINTF("eth_rx: ETH_RX_BD_TOOLONG\n");
    if(len_status & ETH_RX_BD_SHORT)   DEBUG_PRINTF("eth_rx: ETH_RX_BD_SHORT\n");
    if(len_status & ETH_RX_BD_CRCERR)  DEBUG_PRINTF("eth_rx: ETH_RX_BD_CRCERR\n");
    if(len_status & ETH_RX_BD_LATECOL) DEBUG_PRINTF("eth_rx: ETH_RX_BD_LATECOL\n");
    }
#endif

    MEM_READ32 (ETH_BD_RX_LS(rx_next), value);
    MEM_WRITE32(ETH_BD_RX_LS(rx_next), value & ~ETH_RX_BD_STATS);
    MEM_READ32 (ETH_BD_RX_LS(rx_next), value);
    MEM_WRITE32(ETH_BD_RX_LS(rx_next), value | ETH_RX_BD_EMPTY);

    rx_next = (rx_next + 1) & ETH_RXBD_NUM_MASK;
	
#ifdef DEBUG2
      eth_print_packet(buf, len);
#endif
    return len;
}

//--------------------------------------------------------
// - buf: will contains 'len' bytes in big-endian format
// - len: num of bytes including dest mac to the end of payload,
//        except CRC
uint32_t
eth_rx_b(void *buf, uint32_t size)
{
   uint32_t value;
#ifdef DEBUG2
  DEBUG_PRINTF("eth_rx_b()----------------\n");
#endif

  do { MEM_READ32(ETH_BD_RX_LS(rx_next), value);
  } while ( value & ETH_RX_BD_EMPTY);
  return eth_rx(buf, size);
}

//--------------------------------------------------------
// MDIO read status - read register of PHY
// INPUT: mdio_add - MDIO reg address 5-bit
//        phy_add - PHY address 5-bit
uint16_t
mdio_read(uint8_t mdio_add, uint8_t phy_add)
{
  uint32_t value;
#ifdef DEBUG2
   DEBUG_PRINTF2("mdio_read(A:0x%x, P:0x%x)\n", mdio_add, phy_add);
#endif
  MEM_WRITE32(ETH_MIIADDRESS, ((mdio_add&0x1F)<<ETH_MIIA_RGAD_SFT)
                              |(phy_add&ETH_MIIA_FIAD_MSK)); //REG32(ETH_MIIADDRESS) = ((mdio_add&0x1F)<<ETH_MIIA_RGAD_SFT)| (phy_add&ETH_MIIA_FIAD_MSK);
  MEM_WRITE32(ETH_MIICOMMAND, ETH_MIICOMMAND_RSTAT); //REG32(ETH_MIICOMMAND) = ETH_MIICOMMAND_RSTAT;
  do { MEM_READ32(ETH_MIISTATUS, value);
  } while (value & ETH_MIISTATUS_BUSY); //while (REG32(ETH_MIISTATUS) & ETH_MIISTATUS_BUSY);
  MEM_READ32(ETH_MIIRX_DATA, value);
  return value&0xFFFF;
}

//--------------------------------------------------------
// MDIO write control data - write register of PHY
// INPUT: mdio_add - MDIO reg address 5-bit
//        phy_add - PHY address 5-bit
//        data - data to write
void
mdio_write(uint8_t mdio_add, uint8_t phy_add, uint16_t data)
{
  uint32_t value;
#ifdef DEBUG2
   DEBUG_PRINTF3("mdio_write(A:0x%x, P:0x%x, D:0x%x)\n", mdio_add, phy_add, data);
#endif
  MEM_WRITE32(ETH_MIIADDRESS, ((mdio_add&0x1F)<<ETH_MIIA_RGAD_SFT)
                              |(phy_add&ETH_MIIA_FIAD_MSK)); //REG32(ETH_MIIADDRESS) = (uint32_t)((mdio_add&0x1F)<<ETH_MIIA_RGAD_SFT)| (phy_add&ETH_MIIA_FIAD_MSK);
  MEM_WRITE32(ETH_MIITX_DATA, (uint32_t)data); //REG32(ETH_MIITX_DATA) = (uint32_t)data;
  MEM_WRITE32(ETH_MIICOMMAND, ETH_MIICOMMAND_WCTRLDATA); //REG32(ETH_MIICOMMAND) = ETH_MIICOMMAND_WCTRLDATA;
  do { MEM_READ32(ETH_MIISTATUS, value);
  } while (value & ETH_MIISTATUS_BUSY); //while (REG32(ETH_MIISTATUS) & ETH_MIISTATUS_BUSY);
}

//--------------------------------------------------------
// Check default value of Ethernet MAC
void
eth_mac_check(void)
{
   uint32_t value;
   MEM_READ32(ETH_MODER     ,value); compare32("ETH_MODER     ", value, ETH_MODER_DEFAULT);
   MEM_READ32(ETH_INT_SOURCE,value); compare32("ETH_INT_SOURCE", value, ETH_INT_SOURCE_DEFAULT);
   MEM_READ32(ETH_INT_MASK  ,value); compare32("ETH_INT_MASK  ", value, ETH_INT_MASK_DEFAULT);
   MEM_READ32(ETH_IPGT      ,value); compare32("ETH_IPGT      ", value, 0x00000012);
   MEM_READ32(ETH_IPGR1     ,value); compare32("ETH_IPGR1     ", value, 0x0000000C);
   MEM_READ32(ETH_IPGR2     ,value); compare32("ETH_IPGR2     ", value, 0x00000012);
   MEM_READ32(ETH_PACKETLEN ,value); compare32("ETH_PACKETLEN ", value, 0x00400600);
   MEM_READ32(ETH_COLLCONF  ,value); compare32("ETH_COLLCONF  ", value, 0x000F003F);
   MEM_READ32(ETH_TX_BD_NUM ,value); compare32("ETH_TX_BD_NUM ", value, 0x00000040);
   MEM_READ32(ETH_CTRLMODER ,value); compare32("ETH_CTRLMODER ", value, 0x00000000);
   MEM_READ32(ETH_MIIMODER  ,value); compare32("ETH_MIIMODER  ", value, 0x00000064);
   MEM_READ32(ETH_MIICOMMAND,value); compare32("ETH_MIICOMMAND", value, 0x00000000);
   MEM_READ32(ETH_MIIADDRESS,value); compare32("ETH_MIIADDRESS", value, 0x00000000);
   MEM_READ32(ETH_MIITX_DATA,value); compare32("ETH_MIITX_DATA", value, 0x00000000);
   MEM_READ32(ETH_MIIRX_DATA,value); compare32("ETH_MIIRX_DATA", value, 0x00000000);
   MEM_READ32(ETH_MIISTATUS ,value); compare32("ETH_MIISTATUS ", value, 0x00000000);
   MEM_READ32(ETH_MAC_ADDR0 ,value); compare32("ETH_MAC_ADDR0 ", value, 0x00000000);
   MEM_READ32(ETH_MAC_ADDR1 ,value); compare32("ETH_MAC_ADDR1 ", value, 0x00000000);
   MEM_READ32(ETH_HASH_ADDR0,value); compare32("ETH_HASH_ADDR0", value, 0x00000000);
   MEM_READ32(ETH_HASH_ADDR1,value); compare32("ETH_HASH_ADDR1", value, 0x00000000);
   MEM_READ32(ETH_TXCTRL    ,value); compare32("ETH_TXCTRL    ", value, 0x00000000);
}

#ifdef _DEBUG

//--------------------------------------------------------
void
eth_print_packet(void* buf, uint32_t len)
{
  unsigned int i;
  ether_frame_t *pt = (ether_frame_t*) buf;
  ip_frame_t    *ip_pt = NULL;
  arp_frame_t   *arp_pt = NULL;
 
  DEBUG_PRINTF2("eth_print_packet(buf:0x%x, len:%d)\n", (uint32_t) buf, len);
  for(i = 0; i < len; i++) {
      if(!(i % 16))
          DEBUG_PRINTF("\n");
      DEBUG_PRINTF1(" %02x", *(((uint8_t *)buf) + i));
  }
  DEBUG_PRINTF("\n");
  DEBUG_PRINTF6("DST MAC  : %02X:%02X:%02X:%02X:%02X:%02X\n",
                   pt->dst[0], pt->dst[1], pt->dst[2], pt->dst[3], pt->dst[4], pt->dst[5]);
  DEBUG_PRINTF6("SRC MAC  : %02X:%02X:%02X:%02X:%02X:%02X\n",
                   pt->src[0], pt->src[1], pt->src[2], pt->src[3], pt->src[4], pt->src[5]);
  if (NTOHS(pt->type) <= 1500) {
      DEBUG_PRINTF2("LENG     : %x (%d)\n", NTOHS(pt->type), NTOHS(pt->type));
  } else if (NTOHS(pt->type) >= 1536) {
      DEBUG_PRINTF2("TYPE     : %x (%d) ", NTOHS(pt->type), NTOHS(pt->type));
      switch (NTOHS(pt->type)) {
      case 0x0060: DEBUG_PRINTF(": Eternet Loopback packe\n"); break;
      case 0x0200: DEBUG_PRINTF(": Eternet Echo packe\n"); break;
      case 0x0800: DEBUG_PRINTF(": IPv4 packet\n"); ip_pt = (ip_frame_t*)(&pt->data); break;
      case 0x0806: DEBUG_PRINTF(": ARP packet\n"); arp_pt = (arp_frame_t*)(&pt->data); break;
      case 0x86DD: DEBUG_PRINTF(": IPv6 packet\n"); break;
      case 0x9000: DEBUG_PRINTF(": Loopback\n"); break;
      default:     DEBUG_PRINTF(": etc...\n"); break;
      }
  } else {
      DEBUG_PRINTF2("TYPE/LENG: %x (%d)\n", NTOHS(pt->type), NTOHS(pt->type));
  }
  if ((ip_pt!=NULL)&&(ip_pt->ver_hlen&0x0F)&&NTOHS(ip_pt->len)) {
     eth_print_ip_packet(ip_pt);
  }
  if (arp_pt!=NULL) {
     eth_print_arp_packet(arp_pt);
  }
}

//--------------------------------------------------------
// print IP frame pointed at by 'addr'.
void
eth_print_ip_packet(ip_frame_t* pt)
{
  DEBUG_PRINTF1("ver_hlen       : %x\n", pt->ver_hlen);
  DEBUG_PRINTF1("type_of_service: %x\n", pt->type_of_service);
  DEBUG_PRINTF1("len            : %x\n", NTOHS(pt->len));
  DEBUG_PRINTF1("id             : %x\n", NTOHS(pt->id));
  DEBUG_PRINTF1("flag_offset    : %x\n", NTOHS(pt->flag_offset));
  DEBUG_PRINTF1("tlive          : %x\n", pt->tlive);
  DEBUG_PRINTF1("protocol       : %x ",  pt->protocol);      
  switch (pt->protocol) {
    case  1: DEBUG_PRINTF(": ICMP\n"); break;
    case  6: DEBUG_PRINTF(": TCP\n"); break;
    case 17: DEBUG_PRINTF(": UDP\n"); break;
    default: DEBUG_PRINTF("\n"); break;
  }
  DEBUG_PRINTF1("check_sum      : %x\n", NTOHS(pt->check_sum));      
  DEBUG_PRINTF4("SRC IP         : %d.%d.%d.%d\n",
          pt->src_ip[0], pt->src_ip[1], pt->src_ip[2],
          pt->src_ip[3]);
  DEBUG_PRINTF4("DST IP         : %d.%d.%d.%d\n",
          pt->dst_ip[0], pt->dst_ip[1], pt->dst_ip[2],
          pt->dst_ip[3]);
}

//--------------------------------------------------------
// print ARP frame pointed at by 'addr'.
void
eth_print_arp_packet(void* buf)
{
  arp_frame_t*    pt=(arp_frame_t*)buf;
  DEBUG_PRINTF("Hardware type: ");
  switch (NTOHS(pt->hw_type)) {
    case 0x0001: DEBUG_PRINTF2("%s (%x)\n", "Ethernet", NTOHS(pt->hw_type)); break;
    default:     DEBUG_PRINTF1("%x\n", NTOHS(pt->hw_type));
  }
  DEBUG_PRINTF("Protocol type: ");
  switch (NTOHS(pt->pro_type)) {
      case 0x0060: DEBUG_PRINTF("Eternet Loopback packet"); break;
      case 0x0200: DEBUG_PRINTF("Eternet Echo packe"); break;
      case 0x0800: DEBUG_PRINTF("IPv4 packet"); break;
      case 0x0806: DEBUG_PRINTF("ARP packet"); break;
      case 0x86DD: DEBUG_PRINTF("IPv6 packet"); break;
      case 0x9000: DEBUG_PRINTF("Loopback"); break;
      default:     DEBUG_PRINTF("etc..."); break;
  }
  DEBUG_PRINTF1(" %x\n", NTOHS(pt->pro_type));
  DEBUG_PRINTF1("Hardware size: %x octets\n", pt->hw_len);
  DEBUG_PRINTF1("Protocol size: %x octets\n", pt->pro_len);
  DEBUG_PRINTF("Opcode       : ");
  switch (NTOHS(pt->msg_type)) {
    case 0x0001: DEBUG_PRINTF2("%s (%x)\n", "request", NTOHS(pt->msg_type)); break;
    case 0x0002: DEBUG_PRINTF2("%s (%x)\n", "reply", NTOHS(pt->msg_type)); break;
    default:     DEBUG_PRINTF1("%x\n", NTOHS(pt->msg_type));
  }
  DEBUG_PRINTF6("SRC MAC      : %02X:%02X:%02X:%02X:%02X:%02X\n",
          pt->src_mac[0], pt->src_mac[1], pt->src_mac[2],
          pt->src_mac[3], pt->src_mac[4], pt->src_mac[5]);
  DEBUG_PRINTF4("SRC IP       : %d.%d.%d.%d\n",
          pt->src_ip[0], pt->src_ip[1], pt->src_ip[2],
          pt->src_ip[3]);
  DEBUG_PRINTF6("DST MAC      : %02X:%02X:%02X:%02X:%02X:%02X\n",
          pt->dst_mac[0], pt->dst_mac[1], pt->dst_mac[2],
          pt->dst_mac[3], pt->dst_mac[4], pt->dst_mac[5]);
  DEBUG_PRINTF4("DST IP       : %d.%d.%d.%d\n",
          pt->dst_ip[0], pt->dst_ip[1], pt->dst_ip[2],
          pt->dst_ip[3]);
}

#endif // _DEBUG

//--------------------------------------------------------
// Disable receiver and transmiter
void
eth_mac_halt(void)
{
  uint32_t value;
  MEM_READ32(ETH_MODER, value);
  MEM_WRITE32(ETH_MODER, value & ~(ETH_MODER_RXEN | ETH_MODER_TXEN)); //REG32(ETH_MODER) &= ~(ETH_MODER_RXEN | ETH_MODER_TXEN);
}

//--------------------------------------------------------
// Enable receiver and transmiter
void
eth_mac_enable(void)
{
  uint32_t value;
  MEM_READ32(ETH_MODER, value);
  MEM_WRITE32(ETH_MODER, value | ETH_MODER_RXEN | ETH_MODER_TXEN); //REG32(ETH_MODER) |= ETH_MODER_RXEN | ETH_MODER_TXEN;
}

//--------------------------------------------------------
// Returns pointer to next free buffer;
//         NULL if none available
void *
eth_get_tx_buf(void)
{
  uint32_t add, value;

  if(tx_full) {
     return (void *)0;
  }
  MEM_READ32(ETH_BD_TX_LS(tx_next), value);
  if(value & ETH_TX_BD_READY) { //if(REG32(ETH_BD_TX_LS(tx_next)) & ETH_TX_BD_READY) {
    return (void *)0;
  }
  MEM_READ32(ETH_BD_TX_AD(tx_next), add); //add = REG32(ETH_BD_TX_AD(tx_next));
  tx_next = (tx_next + 1) & ETH_TXBD_NUM_MASK;
  if(tx_next == tx_last) {
    tx_full = 1;
  }
  return (void *)add;
}

//--------------------------------------------------------
// Set local MAC address
// Input: 48-bit Physical MAC address, i.e. OUI (Orgznizationally Unique Identification)
//        mac_addr[0] - byte 0  : transferred first on Ethernet
//        mac_addr[1] - byte 1
//        mac_addr[2] - byte 2
//        mac_addr[3] - byte 3
//        mac_addr[4] - byte 4
//        mac_addr[5] - byte 5
void
eth_mac_addr_write(uint8_t *mac_addr)
{
  MEM_WRITE32(ETH_MAC_ADDR1, (mac_addr[0]<<8)|mac_addr[1]);
  //REG32(ETH_MAC_ADDR1) = mac_addr[0] <<  8 |
  //                       mac_addr[1];
  MEM_WRITE32(ETH_MAC_ADDR0, (mac_addr[2]<<24)|(mac_addr[2]<<24)|(mac_addr[3]<<16)|(mac_addr[4]<<8)|mac_addr[5]);
  //REG32(ETH_MAC_ADDR0) = mac_addr[2] << 24 |
  //                       mac_addr[3] << 16 |
  //                       mac_addr[4] <<  8 |
  //                       mac_addr[5];
}

//--------------------------------------------------------
// Get local MAC address
// OUTPUT: 48-bit Physical MAC address, i.e. OUI (Orgznizationally Unique Identification)
//        mac_addr[0] - byte 0  : transferred first on Ethernet
//        mac_addr[1] - byte 1
//        mac_addr[2] - byte 2
//        mac_addr[3] - byte 3
//        mac_addr[4] - byte 4
//        mac_addr[5] - byte 5
void
eth_mac_addr_read(uint8_t *mac_addr)
{
   uint32_t value;

   MEM_READ32(ETH_MAC_ADDR1, value); //value = REG32(ETH_MAC_ADDR1); // 2-byte
   mac_addr[0] = (value&0xFF00)>>8;
   mac_addr[1] = (value&0xFF);
   MEM_READ32(ETH_MAC_ADDR0, value); //value = REG32(ETH_MAC_ADDR0); // 4-byte
   mac_addr[5] = (value&0xFF);
   mac_addr[4] = (value&0xFF00)>>8;
   mac_addr[3] = (value&0xFF0000)>>16;
   mac_addr[2] = (value&0xFF000000)>>24;
}

//--------------------------------------------------------
static void
init_tx_bd_pool(void)
{
  uint8_t i;
  uint32_t len_status = 0;
  
  for(i = 0; i < (ETH_TXBD_NUM-1); i++) {
    // Set Tx BD status
    MEM_WRITE32(ETH_BD_TX_LS(i), len_status);
    //REG32(ETH_BD_TX_LS(i)) = len_status;
    // Initialize Tx buffer pointer
    MEM_WRITE32(ETH_BD_TX_AD(i), ETH_DATA_BASE + (i * ETH_MAXBUF_LEN));
    //REG32(ETH_BD_TX_AD(i)) = ETH_DATA_BASE + (i * ETH_MAXBUF_LEN);
  }
  MEM_WRITE32(ETH_BD_TX_LS(i),len_status | ETH_TX_BD_WRAP); // Last Tx BD - Wrap
  //REG32(ETH_BD_TX_LS(i)) = len_status | ETH_TX_BD_WRAP; // Last Tx BD - Wrap
  MEM_WRITE32(ETH_BD_TX_AD(i), ETH_DATA_BASE + (i * ETH_MAXBUF_LEN));
  //REG32(ETH_BD_TX_AD(i)) = ETH_DATA_BASE + (i * ETH_MAXBUF_LEN);
#ifdef RIGOR
{
  uint32_t dataR;
  eth_bd  *bd;
  bd = (eth_bd *)ETH_BD_BASE_TX;
  for(i = 0; i < (ETH_RXBD_NUM-1); i++){
    MEM_READ32((unsigned int)&bd[i].len_status, dataR);
    if (dataR != len_status) {
        DEBUG_PRINTF4("tx bd[%d] len_status should be 0x%x, but 0x%x\n",
                i, len_status, dataR);
    }
    MEM_READ32((unsigned int)&bd[i].addr, dataR);
    if (dataR != (ETH_DATA_BASE + (i * ETH_MAXBUF_LEN))) {
        DEBUG_PRINTF3("tx bd[%d] addr should be 0x%x, but 0x%x\n",
                i, (ETH_DATA_BASE + (i * ETH_MAXBUF_LEN)), dataR);
    }
  }
  MEM_READ32((unsigned int)&bd[i].len_status, dataR);
  if (dataR != (len_status|ETH_TX_BD_WRAP)) {
        DEBUG_PRINTF3("tx bd[%d] len_status should be 0x%x, but 0x%x\n",
                i, (len_status|ETH_TX_BD_WRAP), dataR);
  }
  MEM_READ32((unsigned int)&bd[i].addr, dataR);
  if (dataR != (ETH_DATA_BASE + (i * ETH_MAXBUF_LEN))) {
      DEBUG_PRINTF3("tx bd[%d] addr should be 0x%x, but 0x%x\n",
              i, (ETH_DATA_BASE + (i * ETH_MAXBUF_LEN)), dataR);
  }
}
#endif

  // Initialize tx pointers
  tx_next = 0;
  tx_last = 0;
  tx_full = 0;
}

//--------------------------------------------------------
static void
init_rx_bd_pool(void)
{
  uint8_t i;
  uint32_t len_status = (0 << 16) | ETH_RX_BD_EMPTY;
//  uint32_t len_status = (0 << 16) | ETH_RX_BD_EMPTY | ETH_RX_BD_IRQ;

  for(i = 0; i < (ETH_RXBD_NUM-1); i++){
    // Set Rx BD status
    MEM_WRITE32(ETH_BD_RX_LS(i), len_status);
    //REG32(ETH_BD_RX_LS(i)) = len_status;
    // Initialize Rx buffer pointer
    MEM_WRITE32(ETH_BD_RX_AD(i), ETH_DATA_BASE + ((ETH_TXBD_NUM + i) * ETH_MAXBUF_LEN));
    //REG32(ETH_BD_RX_AD(i)) = ETH_DATA_BASE + ((ETH_TXBD_NUM + i) * ETH_MAXBUF_LEN);
  }
  MEM_WRITE32(ETH_BD_RX_LS(i), len_status | ETH_RX_BD_WRAP); // Last Rx BD - Wrap
  //REG32(ETH_BD_RX_LS(i)) = len_status | ETH_RX_BD_WRAP;
  MEM_WRITE32(ETH_BD_RX_AD(i), ETH_DATA_BASE + ((ETH_TXBD_NUM + i) * ETH_MAXBUF_LEN));
  //REG32(ETH_BD_RX_AD(i)) = ETH_DATA_BASE + ((ETH_TXBD_NUM + i) * ETH_MAXBUF_LEN);

#ifdef RIGOR
{
  uint32_t dataR;
  eth_bd  *bd;
  bd = (eth_bd *)ETH_BD_BASE + ETH_TXBD_NUM;
  for(i = 0; i < (ETH_RXBD_NUM-1); i++){
    MEM_READ32((unsigned int)&bd[i].len_status, dataR);
    if (dataR != len_status) {
        DEBUG_PRINTF3("rx bd[%d] len_status should be 0x%x, but 0x%x\n",
                i, len_status, dataR);
    }
    MEM_READ32((unsigned int)&bd[i].addr, dataR);
    if (dataR != (ETH_DATA_BASE + ((ETH_TXBD_NUM + i) * ETH_MAXBUF_LEN))) {
        DEBUG_PRINTF3("rx bd[%d] addr should be 0x%x, but 0x%x\n",
                i, (ETH_DATA_BASE + ((ETH_TXBD_NUM + i) * ETH_MAXBUF_LEN)), dataR);
    }
  }
  MEM_READ32((unsigned int)&bd[i].len_status, dataR);
  if (dataR != (len_status|ETH_TX_BD_WRAP)) {
        DEBUG_PRINTF3("rx bd[%d] len_status should be 0x%x, but 0x%x\n",
                i, (len_status|ETH_TX_BD_WRAP), dataR);
  }
  MEM_READ32((unsigned int)&bd[i].addr, dataR);
  if (dataR != (ETH_DATA_BASE + ((ETH_TXBD_NUM + i) * ETH_MAXBUF_LEN))) {
      DEBUG_PRINTF3("rx bd[%d] addr should be 0x%x, but 0x%x\n",
              i, (ETH_DATA_BASE + ((ETH_TXBD_NUM + i) * ETH_MAXBUF_LEN)), dataR);
  }
}
#endif

  // Initialize rx pointers
  rx_next = 0;
}

//--------------------------------------------------------
// Ethernet interrupt handler
void
eth_int (void)
{
#ifdef DEBUG2
   DEBUG_PRINTF("eth_int: Ethernet interrupt occurs\n");
#endif
}

//--------------------------------------------------------
void
eth_int_enable(void)
{
  MEM_WRITE32(ETH_INT_MASK, ETH_INT_MASK_TXB  |
                         ETH_INT_MASK_TXE  |
                         ETH_INT_MASK_RXF  |
                         ETH_INT_MASK_RXE  |
                         ETH_INT_MASK_BUSY |
                         ETH_INT_MASK_TXC  |
                         ETH_INT_MASK_RXC);
  //REG32(ETH_INT_MASK) =  ETH_INT_MASK_TXB  |
  //                       ETH_INT_MASK_TXE  |
  //                       ETH_INT_MASK_RXF  |
  //                       ETH_INT_MASK_RXE  |
  //                       ETH_INT_MASK_BUSY |
  //                       ETH_INT_MASK_TXC  |
  //                       ETH_INT_MASK_RXC;
}

//--------------------------------------------------------
static void
compare16(char* str, uint16_t val, uint16_t expect)
{
#ifdef PRINT_SIMPLE
    if (val==expect) DEBUG_PRINTF2("%s 0x%x\n", str, val);
    else DEBUG_PRINTF3("%s 0x%x, but 0x%x expected\n",
                  str, val, expect);
#else
    if (val==expect) DEBUG_PRINTF2 ("%-10s 0x%.04x\n", str, val);
    else DEBUG_PRINTF3("%-10s 0x%.04x, but 0x%.04x expected\n",
                  str, val, expect);
#endif
}

//--------------------------------------------------------
static void
compare32(char* str, uint32_t val, uint32_t expect)
{
#ifdef PRINT_SIMPLE
    if (val==expect) DEBUG_PRINTF2("%s 0x%x\n", str, val);
    else DEBUG_PRINTF3("%s 0x%x, but 0x%x expected\n",
                  str, val, expect);
#else
    if (val==expect) DEBUG_PRINTF2("%-10s 0x%.08x\n", str, val);
    else DEBUG_PRINTF3("%-10s 0x%.08x, but 0x%.08x expected\n",
                  str, val, expect);
#endif
}

//--------------------------------------------------------
#undef REG32

static void phy_check(uint8_t phy_add)
{
    DEBUG_PRINTF1("phy_check(%02x)\n", phy_add);
    uint8_t phy_reg;
    for (phy_reg = 0; phy_reg < 32; ++phy_reg)
    {
	uint16_t value = mdio_read(phy_reg, phy_add);
	DEBUG_PRINTF2("phy[%02x] = %04x\n", phy_reg, value);
    }
}

//--------------------------------------------------------
// Revision History
//
// 2012.06.16: Re-written using MEM_WRITE32()/MEM_READ32()
// Dec. 31, 2008: Start by Ando Ki (adki@dynalith.com)
//--------------------------------------------------------
