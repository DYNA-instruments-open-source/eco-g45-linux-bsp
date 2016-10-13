# -*-makefile-*-
#
# Copyright (C) 2012 by Carsten behling <carsten.behling@garz-fricke.com>
#
# See CREDITS for details about who has contributed to this project.
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_IMX_LIB) += imx-lib

#
# Paths and names
#
IMX_LIB_VERSION	:= 11.09.01
IMX_LIB			:= imx-lib-$(IMX_LIB_VERSION)
IMX_LIB_SUFFIX	:= 
IMX_LIB_URL		:= file://$(PTXDIST_WORKSPACE)/local_src/common/$(IMX_LIB)
IMX_LIB_SRCDIR	:= $(PTXDIST_WORKSPACE)/local_src/common/$(IMX_LIB)
IMX_LIB_DIR		:= $(BUILDDIR)/$(IMX_LIB)
IMX_LIB_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

$(STATEDIR)/imx-lib.extract:
	@$(call targetinfo)
	@$(call clean, $(IMX_LIB_DIR))
	@rm -Rf $(IMX_LIB_DIR)
	@cp -R $(IMX_LIB_SRCDIR) $(IMX_LIB_DIR)
	@$(call patchin, IMX_LIB)
	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

$(STATEDIR)/imx-lib.prepare:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

$(STATEDIR)/imx-lib.compile:
	@$(call targetinfo)
	@cd $(IMX_LIB_DIR) && PATH=$(CROSS_PATH) $(CROSS_ENV) \
		PLATFORM=$(PTXCONF_IMX_LIB_PLATFORM) \
		CROSS_COMPILE=$(KERNEL_CROSS_COMPILE) INCLUDE='-I$(KERNEL_HEADERS_DIR)/include \
		-I$(KERNEL_DIR)/drivers/mxc/security/rng/include \
		-I$(KERNEL_DIR)/drivers/mxc/security/sahara2/include' $(MAKE)
	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

$(STATEDIR)/imx-lib.install:
	@$(call targetinfo)
	@cd $(IMX_LIB_DIR) && DEST_DIR=$(SYSROOT) \
		PLATFORM=$(PTXCONF_IMX_LIB_PLATFORM) $(MAKE) install
	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/imx-lib.targetinstall:
	@$(call targetinfo)

	@$(call install_init, imx-lib)
	@$(call install_fixup, imx-lib,PRIORITY,optional)
	@$(call install_fixup, imx-lib,SECTION,base)
	@$(call install_fixup, imx-lib,AUTHOR,"Carsten Behling <carsten.behling@garz-fricke.com>")
	@$(call install_fixup, imx-lib,DESCRIPTION,missing)

	@$(call install_copy, imx-lib, 0, 0, 0644, $(SYSROOT)/usr/lib/libipu.so.0, /usr/lib/libipu.so.0)
	@$(call install_link, imx-lib, libipu.so.0, /usr/lib/libipu.so)
	@$(call install_copy, imx-lib, 0, 0, 0644, $(SYSROOT)/usr/lib/libIpuScreenLayer.so.0, /usr/lib/libIpuScreenLayer.so.0)
	@$(call install_link, imx-lib, libIpuScreenLayer.so.0, /usr/lib/libIpuScreenLayer.so)
	@$(call install_copy, imx-lib, 0, 0, 0644, $(SYSROOT)/usr/lib/libfsl_shw.so.0, /usr/lib/libfsl_shw.so.0)
	@$(call install_link, imx-lib, libfsl_shw.so.0, /usr/lib/libfsl_shw.so)
	@$(call install_copy, imx-lib, 0, 0, 0644, $(SYSROOT)/usr/lib/libsahara.so.0, /usr/lib/libsahara.so.0)
	@$(call install_link, imx-lib, libsahara.so.0, /usr/lib/libsahara.so)
	@$(call install_copy, imx-lib, 0, 0, 0644, $(SYSROOT)/usr/lib/libvpu.so.0, /usr/lib/libvpu.so.0)
	@$(call install_link, imx-lib, libvpu.so.0, /usr/lib/libvpu.so)

	@$(call install_finish, imx-lib)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

#$(STATEDIR)/imx-lib.clean:
#	@$(call targetinfo)
#	@$(call clean_pkg, IMX_LIB)

# vim: syntax=make
