# -*-makefile-*-
#
# Copyright (C) 2012 by Carsten Behling <carsten.behling@garz-fricke.com>
#
# See CREDITS for details about who has contributed to this project.
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_FIRMWARE_IMX) += firmware-imx

#
# Paths and names
#
FIRMWARE_IMX_VERSION	:= 11.09.01
FIRMWARE_IMX			:= firmware-imx-$(FIRMWARE_IMX_VERSION)
FIRMWARE_IMX_SUFFIX		:= 
FIRMWARE_IMX_URL		:= file://$(PTXDIST_WORKSPACE)/local_src/common/$(FIRMWARE_IMX)
FIRMWARE_IMX_SRCDIR		:= $(PTXDIST_WORKSPACE)/local_src/common/$(FIRMWARE_IMX)
FIRMWARE_IMX_DIR		:= $(BUILDDIR)/$(FIRMWARE_IMX)
FIRMWARE_IMX_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

$(STATEDIR)/firmware-imx.extract:
	@$(call targetinfo)
	@$(call clean, $(FIRMWARE_IMX_DIR))
	@rm -Rf $(FIRMWARE_IMX_DIR)
	@cp -R $(FIRMWARE_IMX_SRCDIR) $(FIRMWARE_IMX_DIR)
	@$(call patchin, FIRMWARE_IMX)
	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

$(STATEDIR)/firmware-imx.prepare:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

$(STATEDIR)/firmware-imx.compile:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

$(STATEDIR)/firmware-imx.install:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/firmware-imx.targetinstall:
	@$(call targetinfo)

	@$(call install_init, firmware-imx)
	@$(call install_fixup, firmware-imx,PRIORITY,optional)
	@$(call install_fixup, firmware-imx,SECTION,base)
	@$(call install_fixup, firmware-imx,AUTHOR,"Carsten Behling <carsten.behling@garz-fricke.com>")
	@$(call install_fixup, firmware-imx,DESCRIPTION,missing)

	# Licenses 
	@$(call install_copy, firmware-imx, 0, 0, 0755, $(FIRMWARE_IMX_DIR)/licenses/COPYING, \
		/usr/share/$(FIRMWARE_IMX)/licenses/COPYING)
	@$(call install_copy, firmware-imx, 0, 0, 0755, $(FIRMWARE_IMX_DIR)/licenses/vpu/EULA, \
		/usr/share/$(FIRMWARE_IMX)/licenses/vpu/EULA)
	@$(call install_copy, firmware-imx, 0, 0, 0755, $(FIRMWARE_IMX_DIR)/firmware/LICENCE.atheros_firmware, \
		/usr/share/$(FIRMWARE_IMX)/licenses/atheros/LICENCE.atheros_firmware)
	@$(call install_copy, firmware-imx, 0, 0, 0755, $(FIRMWARE_IMX_DIR)/licenses/ar3k/30101/LICENSE_Apache.txt, \
		/usr/share/$(FIRMWARE_IMX)/licenses/atheros/ar3k/30101/LICENSE_Apache.txt)
	@$(call install_copy, firmware-imx, 0, 0, 0755, $(FIRMWARE_IMX_DIR)/licenses/ath6k/AR6003/License.txt, \
		/usr/share/$(FIRMWARE_IMX)/licenses/atheros/ath6k/AR6003/License.txt)
	@$(call install_copy, firmware-imx, 0, 0, 0755, $(FIRMWARE_IMX_DIR)/licenses/ath6k/AR6003/README.chromium, \
		/usr/share/$(FIRMWARE_IMX)/licenses/atheros/ath6k/AR6003/README.chromium)
	@$(call install_copy, firmware-imx, 0, 0, 0755, $(FIRMWARE_IMX_DIR)/licenses/ath6k/AR6102/License.txt, \
		/usr/share/$(FIRMWARE_IMX)/licenses/atheros/ath6k/AR6102/License.txt)

	# Firmware
ifdef PTXCONF_FIRMWARE_IMX_IMX25
	@$(call install_copy, firmware-imx, 0, 0, 0755, $(FIRMWARE_IMX_DIR)/firmware/sdma/sdma-imx25-to1.bin, \
		/lib/firmware/sdma/sdma-imx25-to1.bin)
endif
ifdef PTXCONF_FIRMWARE_IMX_IMX27
	@$(call install_copy, firmware-imx, 0, 0, 0755, $(FIRMWARE_IMX_DIR)/firmware/vpu/vpu_fw_imx27_TO1.bin, \
		/lib/firmware/vpu/vpu_fw_imx27_TO1.bin)
	@$(call install_copy, firmware-imx, 0, 0, 0755, $(FIRMWARE_IMX_DIR)/firmware/vpu/vpu_fw_imx27_TO2.bin, \
		/lib/firmware/vpu/vpu_fw_imx27_TO2.bin)
endif
ifdef PTXCONF_FIRMWARE_IMX_IMX31
	@$(call install_copy, firmware-imx, 0, 0, 0755, $(FIRMWARE_IMX_DIR)/firmware/sdma/sdma-imx31-to1.bin, \
		/lib/firmware/sdma/sdma-imx31-to1.bin)
	@$(call install_copy, firmware-imx, 0, 0, 0755, $(FIRMWARE_IMX_DIR)/firmware/sdma/sdma-imx31-to2.bin, \
		/lib/firmware/sdma/sdma-imx31-to2.bin)
endif
ifdef PTXCONF_FIRMWARE_IMX_IMX35
	@$(call install_copy, firmware-imx, 0, 0, 0755, $(FIRMWARE_IMX_DIR)/firmware/sdma/sdma-imx35-to1.bin, \
		/lib/firmware/sdma/sdma-imx35-to1.bin)
	@$(call install_copy, firmware-imx, 0, 0, 0755, $(FIRMWARE_IMX_DIR)/firmware/sdma/sdma-imx35-to2.bin, \
		/lib/firmware/sdma/sdma-imx35-to2.bin)
endif
ifdef PTXCONF_FIRMWARE_IMX_IMX51
	@$(call install_copy, firmware-imx, 0, 0, 0755, $(FIRMWARE_IMX_DIR)/firmware/vpu/vpu_fw_imx51.bin, \
		/lib/firmware/vpu/vpu_fw_imx51.bin)
	@$(call install_copy, firmware-imx, 0, 0, 0755, $(FIRMWARE_IMX_DIR)/firmware/sdma/sdma-imx51-to3.bin, \
		/lib/firmware/sdma/sdma-imx51-to3.bin)
endif
ifdef PTXCONF_FIRMWARE_IMX_IMX53
	@$(call install_copy, firmware-imx, 0, 0, 0755, $(FIRMWARE_IMX_DIR)/firmware/vpu/vpu_fw_imx53.bin, \
		/lib/firmware/vpu/vpu_fw_imx53.bin)
	@$(call install_copy, firmware-imx, 0, 0, 0755, $(FIRMWARE_IMX_DIR)/firmware/sdma/sdma-imx53-to1.bin, \
		/lib/firmware/sdma/sdma-imx53-to1.bin)
endif
ifdef PTXCONF_FIRMWARE_IMX_IMX6Q
	@$(call install_copy, firmware-imx, 0, 0, 0755, $(FIRMWARE_IMX_DIR)/firmware/vpu/vpu_fw_imx6q.bin, \
		/lib/firmware/vpu/vpu_fw_imx6q.bin)
endif
	@$(call install_copy, firmware-imx, 0, 0, 0755, $(FIRMWARE_IMX_DIR)/firmware/ar3k/30101/PS_ASIC.pst, \
		/lib/firmware/PS_ASIC.pst)
	@$(call install_copy, firmware-imx, 0, 0, 0755, $(FIRMWARE_IMX_DIR)/firmware/ar3k/30101/RamPatch.txt, \
		/lib/firmware/RamPatch.txt)
	@$(call install_copy, firmware-imx, 0, 0, 0755, $(FIRMWARE_IMX_DIR)/firmware/ar3k/30101/ar3kbdaddr.pst, \
		/lib/firmware/ar3kbdaddr.pst)
	@$(call install_copy, firmware-imx, 0, 0, 0755, $(FIRMWARE_IMX_DIR)/firmware/ath6k/AR6003/hw1.0/athwlan.bin.z77, \
		/lib/firmware/ath6k/AR6003/hw1.0/athwlan.bin.z77)
	@$(call install_copy, firmware-imx, 0, 0, 0755, $(FIRMWARE_IMX_DIR)/firmware/ath6k/AR6003/hw1.0/bdata.SD31.bin, \
		/lib/firmware/ath6k/AR6003/hw1.0/bdata.SD31.bin)
	@$(call install_copy, firmware-imx, 0, 0, 0755, $(FIRMWARE_IMX_DIR)/firmware/ath6k/AR6003/hw1.0/bdata.SD32.bin, \
		/lib/firmware/ath6k/AR6003/hw1.0/bdata.SD32.bin)
	@$(call install_copy, firmware-imx, 0, 0, 0755, $(FIRMWARE_IMX_DIR)/firmware/ath6k/AR6003/hw1.0/bdata.WB31.bin, \
		/lib/firmware/ath6k/AR6003/hw1.0/bdata.WB31.bin)
	@$(call install_copy, firmware-imx, 0, 0, 0755, $(FIRMWARE_IMX_DIR)/firmware/ath6k/AR6003/hw1.0/data.patch.bin, \
		/lib/firmware/ath6k/AR6003/hw1.0/data.patch.bin)
	@$(call install_copy, firmware-imx, 0, 0, 0755, $(FIRMWARE_IMX_DIR)/firmware/ath6k/AR6003/hw1.0/otp.bin.z77, \
		/lib/firmware/ath6k/AR6003/hw1.0/otp.bin.z77)
	@$(call install_copy, firmware-imx, 0, 0, 0755, $(FIRMWARE_IMX_DIR)/firmware/ath6k/AR6003/hw2.0/athwlan.bin.z77, \
		/lib/firmware/ath6k/AR6003/hw2.0/athwlan.bin.z77)
	@$(call install_copy, firmware-imx, 0, 0, 0755, $(FIRMWARE_IMX_DIR)/firmware/ath6k/AR6003/hw2.0/bdata.SD31.bin, \
		/lib/firmware/ath6k/AR6003/hw2.0/bdata.SD31.bin)
	@$(call install_copy, firmware-imx, 0, 0, 0755, $(FIRMWARE_IMX_DIR)/firmware/ath6k/AR6003/hw2.0/bdata.SD32.bin, \
		/lib/firmware/ath6k/AR6003/hw2.0/bdata.SD32.bin)
	@$(call install_copy, firmware-imx, 0, 0, 0755, $(FIRMWARE_IMX_DIR)/firmware/ath6k/AR6003/hw2.0/bdata.WB31.bin, \
		/lib/firmware/ath6k/AR6003/hw2.0/bdata.WB31.bin)
	@$(call install_copy, firmware-imx, 0, 0, 0755, $(FIRMWARE_IMX_DIR)/firmware/ath6k/AR6003/hw2.0/data.patch.bin, \
		/lib/firmware/ath6k/AR6003/hw2.0/data.patch.bin)
	@$(call install_copy, firmware-imx, 0, 0, 0755, $(FIRMWARE_IMX_DIR)/firmware/ath6k/AR6003/hw2.0/otp.bin.z77, \
		/lib/firmware/ath6k/AR6003/hw2.0/otp.bin.z77)		
	@$(call install_copy, firmware-imx, 0, 0, 0755, $(FIRMWARE_IMX_DIR)/firmware/ath6k/AR6003/hw2.1.1/athwlan.bin, \
		/lib/firmware/ath6k/AR6003/hw2.1.1/athwlan.bin)
	@$(call install_copy, firmware-imx, 0, 0, 0755, $(FIRMWARE_IMX_DIR)/firmware/ath6k/AR6003/hw2.1.1/bdata.SD31.bin, \
		/lib/firmware/ath6k/AR6003/hw2.1.1/bdata.SD31.bin)
	@$(call install_copy, firmware-imx, 0, 0, 0755, $(FIRMWARE_IMX_DIR)/firmware/ath6k/AR6003/hw2.1.1/data.patch.bin, \
		/lib/firmware/ath6k/AR6003/hw2.1.1/data.patch.bin)
	@$(call install_copy, firmware-imx, 0, 0, 0755, $(FIRMWARE_IMX_DIR)/firmware/ath6k/AR6003/hw2.1.1/otp.bin, \
		/lib/firmware/ath6k/AR6003/hw2.1.1/otp.bin)

	@$(call install_copy, firmware-imx, 0, 0, 0755, $(FIRMWARE_IMX_DIR)/firmware/ath6k/AR6102/athwlan.bin.z77, \
		/lib/firmware/ath6k/AR6102/athwlan.bin.z77)
	@$(call install_copy, firmware-imx, 0, 0, 0755, $(FIRMWARE_IMX_DIR)/firmware/ath6k/AR6102/calData_ar6102_15dBm.bin, \
		/lib/firmware/ath6k/AR6102/calData_ar6102_15dBm.bin)
	@$(call install_copy, firmware-imx, 0, 0, 0755, $(FIRMWARE_IMX_DIR)/firmware/ath6k/AR6102/data.patch.hw2_0.bin, \
		/lib/firmware/ath6k/AR6102/data.patch.hw2_0.bin)
	@$(call install_copy, firmware-imx, 0, 0, 0755, $(FIRMWARE_IMX_DIR)/firmware/ath6k/AR6102/eeprom.bin, \
		/lib/firmware/ath6k/AR6102/eeprom.bin)
	@$(call install_copy, firmware-imx, 0, 0, 0755, $(FIRMWARE_IMX_DIR)/firmware/ath6k/AR6102/eeprom.data, \
		/lib/firmware/ath6k/AR6102/eeprom.data)
		
	@$(call install_finish, firmware-imx)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

#$(STATEDIR)/firmware-imx.clean:
#	@$(call targetinfo)
#	@$(call clean_pkg, FIRMWARE_IMX)

# vim: syntax=make
