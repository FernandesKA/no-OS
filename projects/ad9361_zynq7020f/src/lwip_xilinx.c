#ifdef NO_OS_LWIP_NETWORKING

#include <stdint.h>

#include "lwip/sio.h"
#include "netif/xemacpsif.h"
#include "netif/xtopology.h"
#include "xil_exception.h"
#include "xscugic.h"
#include "lwip_socket.h"
#include "lwip_xilinx.h"

int xtopology_n_emacs = 1;
struct xtopology_t xtopology[] = {
	{
		.emac_baseaddr = (UINTPTR)XPAR_XEMACPS_0_BASEADDR,
		.emac_type = xemac_type_emacps,
		.intc_baseaddr = 0,
		.intc_emac_intr = 0,
		.scugic_baseaddr = (UINTPTR)XPAR_SCUGIC_0_DIST_BASEADDR,
		.scugic_emac_intr = 54,
	},
	{ .emac_baseaddr = 0, .emac_type = xemac_type_unknown },
};

static int32_t xilinx_lwip_init(void **desc, void *param)
{
	if (!desc || !param)
		return -1;
	*desc = param;
	return 0;
}

static int32_t xilinx_lwip_remove(void *desc)
{
	(void)desc;
	return 0;
}

err_t rk7020f_xilinx_netif_init(struct netif *netif)
{
	static XScuGic gic;
	XScuGic_Config *config;
	err_t ret;

	config = XScuGic_LookupConfig(XPAR_SCUGIC_0_DEVICE_ID);
	if (!config)
		return ERR_IF;
	if (XScuGic_CfgInitialize(&gic, config, config->CpuBaseAddress) != XST_SUCCESS)
		return ERR_IF;

	Xil_ExceptionInit();
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				     (Xil_ExceptionHandler)XScuGic_InterruptHandler,
				     &gic);

	ret = xemacpsif_init(netif);
	if (ret != ERR_OK)
		return ret;
	Xil_ExceptionEnable();
	return ERR_OK;
}

static int32_t xilinx_lwip_step(struct lwip_network_desc *desc, void *data)
{
	(void)data;
	xemacpsif_input(desc->lwip_netif);
	return 0;
}

const struct no_os_lwip_ops xilinx_lwip_ops = {
	.init = xilinx_lwip_init,
	.remove = xilinx_lwip_remove,
	.step = xilinx_lwip_step,
};

sio_fd_t sio_open(u8_t devnum) { (void)devnum; return NULL; }
void sio_send(u8_t c, sio_fd_t fd) { (void)c; (void)fd; }
u32_t sio_tryread(sio_fd_t fd, u8_t *data, u32_t len)
{
	(void)fd;
	(void)data;
	(void)len;
	return 0;
}

#endif
