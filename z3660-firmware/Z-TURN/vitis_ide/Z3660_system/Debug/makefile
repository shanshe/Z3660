#
# This file was created by Xilinx Vitis tool.
#


#
# tools
#

RM = rm -f
RMDIR = rm -rf



#
# variables
#

SDCARD_GEN = sdcard_gen
GENERATE_BIF = ::scw::generate_bif

XPFM_PATH = C:/Users/shanshe/workspace/design_1_wrapper/export/design_1_wrapper/design_1_wrapper.xpfm
SYS_CONFIG = design_1_wrapper
DOMAINS = standalone_domain,standalone_ps7_cortexa9_1
BIF_PATH = C:/Users/shanshe/workspace/Z3660_system/Debug/system.bif

BITSTREAM_PATH = C:/Users/shanshe/workspace/Z3660/_ide/bitstream/design_1_wrapper.bit
SD_FILES = --elf C:/Users/shanshe/workspace/Z3660/Debug/Z3660.elf,ps7_cortexa9_0 --elf C:/Users/shanshe/workspace/Z3660_emu/Debug/Z3660_emu.elf,ps7_cortexa9_1

#
# primary build targets
#

package:
	-@echo Generating bif file for the system project
	generate_system_bif.bat 50703 ${XPFM_PATH} ${DOMAINS} ${BIF_PATH}
	${SDCARD_GEN} --xpfm ${XPFM_PATH} --sys_config ${SYS_CONFIG} --bif ${BIF_PATH} --bitstream ${BITSTREAM_PATH} ${SD_FILES}

clean:
	$(RMDIR) ./_sds
	${RMDIR} package
	${RM} system.bif

.PHONY: all package clean

all: package


