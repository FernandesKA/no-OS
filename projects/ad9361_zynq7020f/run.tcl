connect

# Системный reset
targets -set -filter {name =~ "APU*"}
rst -system
after 1000

# Загрузка FPGA bitstream
fpga -file build/tmp/system_top.bit
after 500

# Первый Cortex-A9
targets -set -filter {name =~ "*Cortex-A9*#0"}

# FSBL инициализирует PS, DDR, clocks и MIO
dow build/output_boot_bin/fsbl.elf
con
after 3000
stop

# Загружаем приложение AD9361
dow build/ad9361_zynq7020f.elf
con
