# -*-makefile-*-
#
# Copyright (C) 2010 by Tim Jaacks <tim.jaacks@garz-fricke.com>
#
# See CREDITS for details about who has contributed to this project.
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_XCONFIG) += xconfig

#
# Paths and names
#
XCONFIG_VERSION	:= trunk
XCONFIG			:= xconfig
XCONFIG_SUFFIX	:= 
XCONFIG_URL		:= file://$(PTXDIST_WORKSPACE)/local_src/common/$(XCONFIG)
XCONFIG_SRCDIR	:= $(PTXDIST_WORKSPACE)/local_src/common/$(XCONFIG)
XCONFIG_DIR		:= $(BUILDDIR)/$(XCONFIG)
XCONFIG_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

$(STATEDIR)/xconfig.extract:
	@$(call targetinfo)
	@$(call clean, $(XCONFIG_DIR))
	@rm -Rf $(XCONFIG_DIR)
	@cp -R $(XCONFIG_SRCDIR) $(XCONFIG_DIR)
	@$(call patchin, XCONFIG)
	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

$(STATEDIR)/xconfig.prepare:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

$(STATEDIR)/xconfig.compile:
	@$(call targetinfo)
	@cd $(XCONFIG_DIR) && PATH=$(CROSS_PATH) $(CROSS_ENV) CFLAGS+=-I$(KERNEL_HEADERS_DIR)/include $(MAKE)
	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

$(STATEDIR)/xconfig.install:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/xconfig.targetinstall:
	@$(call targetinfo)

	@$(call install_init,  xconfig)
	@$(call install_fixup, xconfig,PACKAGE,xconfig)
	@$(call install_fixup, xconfig,PRIORITY,optional)
	@$(call install_fixup, xconfig,VERSION,$(XCONFIG_VERSION))
	@$(call install_fixup, xconfig,SECTION,base)
	@$(call install_fixup, xconfig,AUTHOR,"Tim Jaacks <tim.jaacks@garz-fricke.com>")
	@$(call install_fixup, xconfig,DEPENDS,)
	@$(call install_fixup, xconfig,DESCRIPTION,missing)

	@$(call install_copy, xconfig, 0, 0, 0755, $(XCONFIG_DIR)/xconfig, /usr/bin/xconfig)

	@$(call install_finish, xconfig)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

#$(STATEDIR)/xconfig.clean:
#	@$(call targetinfo)
#	@$(call clean_pkg, XCONFIG)

# vim: syntax=make
