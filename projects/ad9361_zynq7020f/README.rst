AD9361 Zynq-7020F Ethernet IIOD
================================

This project is the board-specific AD9361 application for the RK7020F Zynq
carrier. It reuses the standard ``projects/ad9361`` sources and adds the Zynq
GEM/lwIP transport for libiio.

The default network configuration is:

* IP address: ``192.168.0.224``
* Netmask: ``255.255.255.0``
* Gateway: ``192.168.0.1``
* IIOD TCP port: ``30431``

Build
-----

.. code-block:: bash

   cd projects/ad9361_zynq7020f
   make -j4

The generated boot image is ``build/output_boot_bin/BOOT.BIN``. Connect with:

.. code-block:: bash

   iio_info -u ip:192.168.0.224

The defaults can be overridden on the command line with
``CONFIG_NO_OS_IP``, ``CONFIG_NO_OS_NETMASK`` and ``CONFIG_NO_OS_GATEWAY``.
