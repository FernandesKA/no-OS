#ifndef __XLWIPCONFIG_H_
#define __XLWIPCONFIG_H_

#define XLWIP_CONFIG_INCLUDE_GEM 1
#define XLWIP_CONFIG_N_TX_DESC 64
#define XLWIP_CONFIG_N_RX_DESC 64
#define XLWIP_CONFIG_N_TX_COALESCE 1
#define XLWIP_CONFIG_N_RX_COALESCE 1
#define XLWIP_CONFIG_EMAC_NUMBER 0

/* One pool item must hold a complete 1518-byte Ethernet frame. */
#define PBUF_POOL_BUFSIZE 1536

/* Board-specific IIOD Ethernet parameters. */
#define IIO_NETWORK_MAC_ADDRESS \
	{ 0x00, 0x0a, 0x35, 0x00, 0x01, 0x22 }
#define IIO_NETWORK_GEM_BASEADDR XPAR_XEMACPS_0_BASEADDR

/* Some bare-metal newlib variants omit the C99 format macros. */
#ifndef PRIu64
#define PRIu64 "llu"
#endif
#ifndef PRIi64
#define PRIi64 "lld"
#endif
#ifndef PRIu32
#define PRIu32 "lu"
#endif
#ifndef PRIi32
#define PRIi32 "li"
#endif

#endif
