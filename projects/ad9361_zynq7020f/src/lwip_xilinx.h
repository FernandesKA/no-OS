#ifndef _RK7020F_LWIP_XILINX_H_
#define _RK7020F_LWIP_XILINX_H_

#include "lwip_socket.h"

err_t rk7020f_xilinx_netif_init(struct netif *netif);
extern const struct no_os_lwip_ops xilinx_lwip_ops;

#endif
