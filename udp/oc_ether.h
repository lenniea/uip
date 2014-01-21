#ifndef _OC_ETHER_H_
#define _OC_ETHER_H_
//--------------------------------------------------------
// Copyright (c) 2008 by Dynalith Systems Co., Ltd.
// All right reserved.
//
// http://www.dynalith.com
//--------------------------------------------------------
// oc_ether.h
//--------------------------------------------------------
// VERSION = 2008.12.31.
//--------------------------------------------------------
// OpenCores Ethernet MAC
//--------------------------------------------------------
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _ip_frame_t ip_frame_t;

extern void     eth_init (const uint8_t *mac);
extern void     eth_tx   (void *buf, uint32_t len);
extern void     eth_tx_b (void *buf, uint32_t len);
extern uint32_t eth_rx   (void *buf, uint32_t size);
extern uint32_t eth_rx_b (void *buf, uint32_t size);
extern void     eth_print_packet(void* buf, uint32_t len);
extern void     eth_print_ip_packet(ip_frame_t* pt);
extern void     eth_print_arp_packet(void* buf);
extern void     eth_mac_halt   (void);
extern void     eth_mac_enable (void);
extern void    *eth_get_tx_buf (void);
extern uint16_t mdio_read(uint8_t mdio_add, uint8_t phy_add);
extern void     mdio_write(uint8_t mdio_add, uint8_t phy_add, uint16_t data);
extern void     eth_mac_check(void);
extern void     eth_mac_addr_write(uint8_t *mac_addr);
extern void     eth_mac_addr_read (uint8_t *mac_addr);

//--------------------------------------------------------
#ifndef ADDR_ETH_START
#	error ADDR_ETH_START undefined
#endif
//--------------------------------------------------------
#define ETH_REG_BASE    ADDR_ETH_START
#define ETH_BD_BASE    (ETH_REG_BASE + 0x400)
#define ETH_TOTAL_BD    128
#define ETH_MAXBUF_LEN  0x600

//--------------------------------------------------------
#define ETH_TXBD_NUM      8
#define ETH_TXBD_NUM_MASK (ETH_TXBD_NUM - 1)
#define ETH_RXBD_NUM      8
#define ETH_RXBD_NUM_MASK (ETH_RXBD_NUM - 1)

/* Ethernet buffer descriptor */
typedef struct _eth_bd {
        volatile uint32_t len_status;     /* Buffer length and status */
        volatile uint32_t addr;          /* Buffer address */
} eth_bd;

// TX BD starting address and its elements
#define ETH_BD_BASE_TX	ETH_BD_BASE
#define ETH_BD_TX_LS(n)	(ETH_BD_BASE_TX+(n)*8)
#define ETH_BD_TX_AD(n)	(ETH_BD_BASE_TX+(n)*8+4)
// RX BD starting address
#define ETH_BD_BASE_RX	(ETH_BD_BASE+ETH_TXBD_NUM*8)
#define ETH_BD_RX_LS(n)	(ETH_BD_BASE_RX+(n)*8)
#define ETH_BD_RX_AD(n)	(ETH_BD_BASE_RX+(n)*8+4)

//--------------------------------------------------------
// Tx BD
#define ETH_TX_BD_READY    0x8000 /* Tx BD Ready */
#define ETH_TX_BD_IRQ      0x4000 /* Tx BD IRQ Enable */
#define ETH_TX_BD_WRAP     0x2000 /* Tx BD Wrap (last BD) */
#define ETH_TX_BD_PAD      0x1000 /* Tx BD Pad Enable */
#define ETH_TX_BD_CRC      0x0800 /* Tx BD CRC Enable */

#define ETH_TX_BD_UNDERRUN 0x0100 /* Tx BD Underrun Status */
#define ETH_TX_BD_RETRY    0x00F0 /* Tx BD Retry Status */
#define ETH_TX_BD_RETLIM   0x0008 /* Tx BD Retransmission Limit Status */
#define ETH_TX_BD_LATECOL  0x0004 /* Tx BD Late Collision Status */
#define ETH_TX_BD_DEFER    0x0002 /* Tx BD Defer Status */
#define ETH_TX_BD_CARRIER  0x0001 /* Tx BD Carrier Sense Lost Status */
#define ETH_TX_BD_STATS    (ETH_TX_BD_UNDERRUN | \
                            ETH_TX_BD_RETRY    | \
                            ETH_TX_BD_RETLIM   | \
                            ETH_TX_BD_LATECOL  | \
                            ETH_TX_BD_DEFER    | \
                            ETH_TX_BD_CARRIER)

//--------------------------------------------------------
// Rx BD
#define ETH_RX_BD_EMPTY    0x8000 /* Rx BD Empty */
#define ETH_RX_BD_IRQ      0x4000 /* Rx BD IRQ Enable */
#define ETH_RX_BD_WRAP     0x2000 /* Rx BD Wrap (last BD) */

#define ETH_RX_BD_MISS     0x0080 /* Rx BD Miss Status */
#define ETH_RX_BD_OVERRUN  0x0040 /* Rx BD Overrun Status */
#define ETH_RX_BD_INVSIMB  0x0020 /* Rx BD Invalid Symbol Status */
#define ETH_RX_BD_DRIBBLE  0x0010 /* Rx BD Dribble Nibble Status */
#define ETH_RX_BD_TOOLONG  0x0008 /* Rx BD Too Long Status */
#define ETH_RX_BD_SHORT    0x0004 /* Rx BD Too Short Frame Status */
#define ETH_RX_BD_CRCERR   0x0002 /* Rx BD CRC Error Status */
#define ETH_RX_BD_LATECOL  0x0001 /* Rx BD Late Collision Status */
#define ETH_RX_BD_STATS    (ETH_RX_BD_MISS    | \
                            ETH_RX_BD_OVERRUN | \
                            ETH_RX_BD_INVSIMB | \
                            ETH_RX_BD_DRIBBLE | \
                            ETH_RX_BD_TOOLONG | \
                            ETH_RX_BD_SHORT   | \
                            ETH_RX_BD_CRCERR  | \
                            ETH_RX_BD_LATECOL)

//--------------------------------------------------------
// Register space
#define ETH_MODER      (ETH_REG_BASE+0x00) /* Mode Register */
#define ETH_INT_SOURCE (ETH_REG_BASE+0x04) /* Interrupt Source Register */
#define ETH_INT_MASK   (ETH_REG_BASE+0x08) /* Interrupt Mask Register */
#define ETH_IPGT       (ETH_REG_BASE+0x0C) /* Back to Bak Inter Packet Gap Register */
#define ETH_IPGR1      (ETH_REG_BASE+0x10) /* Non Back to Back Inter Packet Gap Register 1 */
#define ETH_IPGR2      (ETH_REG_BASE+0x14) /* Non Back to Back Inter Packet Gap Register 2 */
#define ETH_PACKETLEN  (ETH_REG_BASE+0x18) /* Packet Length Register (min. and max.) */
#define ETH_COLLCONF   (ETH_REG_BASE+0x1C) /* Collision and Retry Configuration Register */
#define ETH_TX_BD_NUM  (ETH_REG_BASE+0x20) /* Transmit Buffer Descriptor Number Register */
#define ETH_CTRLMODER  (ETH_REG_BASE+0x24) /* Control Module Mode Register */
#define ETH_MIIMODER   (ETH_REG_BASE+0x28) /* MII Mode Register */
#define ETH_MIICOMMAND (ETH_REG_BASE+0x2C) /* MII Command Register */
#define ETH_MIIADDRESS (ETH_REG_BASE+0x30) /* MII Address Register */
#define ETH_MIITX_DATA (ETH_REG_BASE+0x34) /* MII Transmit Data Register */
#define ETH_MIIRX_DATA (ETH_REG_BASE+0x38) /* MII Receive Data Register */
#define ETH_MIISTATUS  (ETH_REG_BASE+0x3C) /* MII Status Register */
#define ETH_MAC_ADDR0  (ETH_REG_BASE+0x40) /* MAC Individual Address Register 0 */
#define ETH_MAC_ADDR1  (ETH_REG_BASE+0x44) /* MAC Individual Address Register 1 */
#define ETH_HASH_ADDR0 (ETH_REG_BASE+0x48) /* Hash Register 0 */
#define ETH_HASH_ADDR1 (ETH_REG_BASE+0x4C) /* Hash Register 1 */
#define ETH_TXCTRL     (ETH_REG_BASE+0x50) /* Hash Register 1 */

//--------------------------------------------------------
// MODER Register
#define ETH_MODER_DEFAULT  0x0000A000
#define ETH_MODER_RXEN     0x00000001 /* Receive Enable  */
#define ETH_MODER_TXEN     0x00000002 /* Transmit Enable */
#define ETH_MODER_NOPRE    0x00000004 /* No Preamble  */
#define ETH_MODER_BRO      0x00000008 /* Reject Broadcast */
#define ETH_MODER_IAM      0x00000010 /* Use Individual Hash */
#define ETH_MODER_PRO      0x00000020 /* Promiscuous (receive all) */
#define ETH_MODER_IFG      0x00000040 /* Min. IFG not required */
#define ETH_MODER_LOOPBCK  0x00000080 /* Loop Back */
#define ETH_MODER_NOBCKOF  0x00000100 /* No Backoff */
#define ETH_MODER_EXDFREN  0x00000200 /* Excess Defer */
#define ETH_MODER_FULLD    0x00000400 /* Full Duplex */
//#define ETH_MODER_RST      0x00000800 /* Reset MAC not available any more */
#define ETH_MODER_DLYCRCEN 0x00001000 /* Delayed CRC Enable */
#define ETH_MODER_CRCEN    0x00002000 /* CRC Enable */
#define ETH_MODER_HUGEN    0x00004000 /* Huge Enable */
#define ETH_MODER_PAD      0x00008000 /* Pad Enable */
#define ETH_MODER_RECSMALL 0x00010000 /* Receive Small */

//--------------------------------------------------------
// Interrupt Source Register
#define ETH_INT_SOURCE_DEFAULT 0x00000000
#define ETH_INT_SOURCE_TXB     0x00000001 /* Transmit Buffer IRQ */
#define ETH_INT_SOURCE_TXE     0x00000002 /* Transmit Error IRQ */
#define ETH_INT_SOURCE_RXF     0x00000004 /* Receive Frame IRQ */
#define ETH_INT_SOURCE_RXE     0x00000008 /* Receive Error IRQ */
#define ETH_INT_SOURCE_BUSY    0x00000010 /* Busy IRQ */
#define ETH_INT_SOURCE_TXC     0x00000020 /* Transmit Control Frame IRQ */
#define ETH_INT_SOURCE_RXC     0x00000040 /* Received Control Frame IRQ */

//--------------------------------------------------------
// Interrupt Mask Register
#define ETH_INT_MASK_DEFAULT   0x00000000
#define ETH_INT_MASK_TXB       0x00000001 /* Transmit Buffer IRQ Mask */
#define ETH_INT_MASK_TXE       0x00000002 /* Transmit Error IRQ Mask */
#define ETH_INT_MASK_RXF       0x00000004 /* Receive Frame IRQ Mask */
#define ETH_INT_MASK_RXE       0x00000008 /* Receive Error IRQ Mask */
#define ETH_INT_MASK_BUSY      0x00000010 /* Busy IRQ Mask */
#define ETH_INT_MASK_TXC       0x00000020 /* Transmit Control Frame IRQ Mask */
#define ETH_INT_MASK_RXC       0x00000040 /* Received Control Frame IRQ Mask */

//--------------------------------------------------------
// Control Module Mode Register
#define ETH_CTRLMODER_PASSALL 0x00000001 /* Pass Control Frames */
#define ETH_CTRLMODER_RXFLOW  0x00000002 /* Receive Control Flow Enable */
#define ETH_CTRLMODER_TXFLOW  0x00000004 /* Transmit Control Flow Enable */

//--------------------------------------------------------
// MII Mode Register
#define ETH_MIIMODER_CLKDIV   0x000000FF /* Clock Divider */
#define ETH_MIIMODER_NOPRE    0x00000100 /* No Preamble */
#define ETH_MIIMODER_RST      0x00000200 /* MIIM Reset */

//--------------------------------------------------------
// MII Command Register
#define ETH_MIICOMMAND_SCANSTAT  0x00000001 /* Scan Status */
#define ETH_MIICOMMAND_RSTAT     0x00000002 /* Read Status */
#define ETH_MIICOMMAND_WCTRLDATA 0x00000004 /* Write Control Data */

//--------------------------------------------------------
// MII Address Register
#define ETH_MIIA_FIAD_MSK 0x0000001F /* PHY Address mask */
#define ETH_MIIA_FIAD_SFT 0          /* PHY Address shift */
#define ETH_MIIA_RGAD_MSK 0x00001F00 /* RGAD Address mask */
#define ETH_MIIA_RGAD_SFT 8          /* RGAD Address shift */

//--------------------------------------------------------
// MII Status Register
#define ETH_MIISTATUS_LINKFAIL 0x00000001 /* Link Fail */
#define ETH_MIISTATUS_BUSY     0x00000002 /* MII Busy */
#define ETH_MIISTATUS_NVALID   0x00000004 /* Data in MII Status Register is invalid */

#ifdef __cplusplus
}
#endif

//--------------------------------------------------------
// Revision History
//
// Dec. 31, 2008: Start by Ando Ki (adki@dynalith.com)
//--------------------------------------------------------
#endif
