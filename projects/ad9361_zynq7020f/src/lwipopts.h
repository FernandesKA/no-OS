#ifndef RK7020F_LWIPOPTS_H
#define RK7020F_LWIPOPTS_H

/* Start from the no-OS defaults, then enlarge only this board's pools. */
#include "../../../libraries/lwip/configs/lwipopts.h"

#undef MEM_SIZE
#define MEM_SIZE                        (256 * 1024)

#undef MEMP_NUM_PBUF
#define MEMP_NUM_PBUF                   256

/*
 * The Xilinx GEM port keeps one pool pbuf attached to every RX descriptor.
 * With 16 descriptors and the no-OS default pool of 30, only 14 buffers were
 * left for TCP traffic and recv_handler repeatedly failed allocation.
 */
#undef PBUF_POOL_SIZE
#define PBUF_POOL_SIZE                  256

#undef TCP_WND
#define TCP_WND                         (32 * TCP_MSS)

#endif /* RK7020F_LWIPOPTS_H */
