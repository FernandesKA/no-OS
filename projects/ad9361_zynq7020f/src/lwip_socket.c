/* Project-local override of only the lwIP netif initialization path. */
#include <stdbool.h>
#include "no_os_util.h"
#include "lwip/tcp.h"

static err_t rk7020f_tcp_close(struct tcp_pcb *pcb);

#define no_os_lwip_init rk7020f_unused_generic_lwip_init
#define tcp_close rk7020f_tcp_close
#include "../../../network/lwip_raw_socket/lwip_socket.c"
#undef tcp_close
#undef no_os_lwip_init

#include "lwip_xilinx.h"

/* Also covers a remote FIN handled directly by the generic recv callback. */
static err_t rk7020f_tcp_close(struct tcp_pcb *pcb)
{
	err_t err = tcp_close(pcb);

	if (err != ERR_OK) {
		tcp_abort(pcb);
		return ERR_OK;
	}

	return ERR_OK;
}

/*
 * IIOD expects its transport write callback to consume the complete reply.
 * The generic raw lwIP backend returns a short write as soon as tcp_sndbuf()
 * becomes smaller than the reply.  This loses the rest of an IIOD response
 * and shifts every subsequent protocol line.  Pump RX/ACK processing and keep
 * writing until the complete buffer has been queued.
 */
static int32_t rk7020f_lwip_socket_send(void *net, uint32_t sock_id,
				       const void *data, uint32_t size)
{
	struct lwip_network_desc *desc = net;
	struct lwip_socket_desc *sock;
	const uint8_t *buf = data;
	uint32_t sent = 0;
	uint32_t wait_started = sys_now();
	err_t err;

	sock = _get_sock(desc, sock_id);
	if (!sock)
		return -EINVAL;
	if (sock->state != SOCKET_CONNECTED)
		return -ENOTCONN;

	while (sent < size) {
		u16_t avail = tcp_sndbuf(sock->pcb);
		u16_t chunk = no_os_min((uint32_t)avail, size - sent);
		u8_t flags = TCP_WRITE_FLAG_COPY;

		if (!chunk) {
			tcp_output(sock->pcb);
			no_os_lwip_step(desc, NULL);
			if (sys_now() - wait_started > 5000)
				return -ETIMEDOUT;
			continue;
		}

		if (sent + chunk < size)
			flags |= TCP_WRITE_FLAG_MORE;
		err = tcp_write(sock->pcb, buf + sent, chunk, flags);
		if (err == ERR_MEM) {
			tcp_output(sock->pcb);
			no_os_lwip_step(desc, NULL);
			continue;
		}
		if (err != ERR_OK)
			return err;

		sent += chunk;
		wait_started = sys_now();
		err = tcp_output(sock->pcb);
		if (err != ERR_OK && err != ERR_MEM)
			return err;
		no_os_lwip_step(desc, NULL);
	}

	return sent;
}

/* Fully release a raw TCP PCB so the same socket slot can be reused. */
static int32_t rk7020f_lwip_socket_close(void *net, uint32_t sock_id)
{
	struct lwip_network_desc *desc = net;
	struct lwip_socket_desc *sock = _get_sock(desc, sock_id);
	err_t err;

	if (!sock)
		return -EINVAL;

	if (sock->p) {
		if (sock->pcb)
			tcp_recved(sock->pcb, sock->p->tot_len);
		pbuf_free(sock->p);
		sock->p = NULL;
	}

	if (sock->pcb) {
		struct tcp_pcb *pcb = sock->pcb;

		tcp_arg(pcb, NULL);
		tcp_recv(pcb, NULL);
		tcp_err(pcb, NULL);
		err = tcp_close(pcb);
		if (err != ERR_OK)
			tcp_abort(pcb);
	}

	sock->pcb = NULL;
	sock->p_idx = 0;
	_release_socket(desc, sock_id);
	return 0;
}

int32_t no_os_lwip_init(struct lwip_network_desc **desc,
			struct lwip_network_param *param)
{
	struct lwip_network_desc *descriptor;
	struct netif *netif_descriptor;
	ip4_addr_t ipaddr, netmask, gw;
	unsigned int raw_ip[4] = { 0 };
	unsigned int raw_netmask[4] = { 0 };
	unsigned int raw_gateway[4] = { 0 };
	int ret;

	if (!desc || !param || !param->platform_ops)
		return -EINVAL;

	netif_descriptor = no_os_calloc(1, sizeof(*netif_descriptor));
	if (!netif_descriptor)
		return -ENOMEM;

	descriptor = no_os_calloc(1, sizeof(*descriptor));
	if (!descriptor) {
		no_os_free(netif_descriptor);
		return -ENOMEM;
	}

	memcpy(descriptor->hwaddr, param->hwaddr, sizeof(descriptor->hwaddr));
	lwip_init();

#ifndef CONFIG_NO_OS_IP
#error The RK7020F project requires CONFIG_NO_OS_IP
#endif
#ifndef CONFIG_NO_OS_NETMASK
#error CONFIG_NO_OS_NETMASK not defined
#endif
#ifndef CONFIG_NO_OS_GATEWAY
#error CONFIG_NO_OS_GATEWAY not defined
#endif
	sscanf(CONFIG_NO_OS_IP, "%u.%u.%u.%u", &raw_ip[0], &raw_ip[1],
	       &raw_ip[2], &raw_ip[3]);
	sscanf(CONFIG_NO_OS_NETMASK, "%u.%u.%u.%u", &raw_netmask[0],
	       &raw_netmask[1], &raw_netmask[2], &raw_netmask[3]);
	sscanf(CONFIG_NO_OS_GATEWAY, "%u.%u.%u.%u", &raw_gateway[0],
	       &raw_gateway[1], &raw_gateway[2], &raw_gateway[3]);

	IP4_ADDR(&ipaddr, raw_ip[0], raw_ip[1], raw_ip[2], raw_ip[3]);
	IP4_ADDR(&netmask, raw_netmask[0], raw_netmask[1], raw_netmask[2],
		 raw_netmask[3]);
	IP4_ADDR(&gw, raw_gateway[0], raw_gateway[1], raw_gateway[2],
		 raw_gateway[3]);

	ret = param->platform_ops->init(&descriptor->mac_desc, param->mac_param);
	if (ret)
		goto free_descriptor;
	descriptor->platform_ops = param->platform_ops;

	netif_descriptor->hwaddr_len = sizeof(descriptor->hwaddr);
	memcpy(netif_descriptor->hwaddr, descriptor->hwaddr,
	       netif_descriptor->hwaddr_len);
	if (!netif_add(netif_descriptor, &ipaddr, &netmask, &gw,
		       descriptor->mac_desc, rk7020f_xilinx_netif_init,
		       ethernet_input)) {
		ret = -EIO;
		goto platform_remove;
	}
	descriptor->lwip_netif = netif_descriptor;

	netif_set_default(netif_descriptor);
	netif_set_up(netif_descriptor);
	netif_set_link_up(netif_descriptor);

	ret = _lwip_start_mdns(descriptor, netif_descriptor);
	if (ret)
		goto remove_netif;

	lwip_config_if(descriptor);
	descriptor->no_os_net.socket_send = rk7020f_lwip_socket_send;
	descriptor->no_os_net.socket_close = rk7020f_lwip_socket_close;
	descriptor->no_os_net.socket_disconnect = rk7020f_lwip_socket_close;
	printf("IP address: %s\n", ip4addr_ntoa(&netif_descriptor->ip_addr));
	printf("Network mask: %s\n", ip4addr_ntoa(&netif_descriptor->netmask));
	printf("Gateway's IP address: %s\n", ip4addr_ntoa(&netif_descriptor->gw));

	for (int i = 0; i < NO_OS_MAX_SOCKETS; i++) {
		descriptor->sockets[i].state = SOCKET_CLOSED;
		descriptor->sockets[i].desc = descriptor;
		descriptor->sockets[i].id = i;
	}

	*desc = descriptor;
	return 0;

remove_netif:
	netif_remove(netif_descriptor);
platform_remove:
	param->platform_ops->remove(descriptor->mac_desc);
free_descriptor:
	no_os_free(descriptor);
	no_os_free(netif_descriptor);
	return ret;
}
