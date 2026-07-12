#include <stdint.h>
#include <string.h>

#include "iio_app.h"
#include "lwip_xilinx.h"
#include "xparameters.h"

static int rk7020f_iio_app_init(struct iio_app_desc **app,
				struct iio_app_init_param param);

#define iio_app_init rk7020f_iio_app_init
#include AD9361_PROJECT_MAIN_FILE
#undef iio_app_init

static int rk7020f_iio_app_init(struct iio_app_desc **app,
				struct iio_app_init_param param)
{
	static const uint8_t mac_address[6] = IIO_NETWORK_MAC_ADDRESS;

	param.lwip_param.platform_ops = &xilinx_lwip_ops;
	param.lwip_param.mac_param = (void *)(uintptr_t)IIO_NETWORK_GEM_BASEADDR;
	memcpy(param.lwip_param.hwaddr, mac_address, sizeof(mac_address));

	return iio_app_init(app, param);
}
