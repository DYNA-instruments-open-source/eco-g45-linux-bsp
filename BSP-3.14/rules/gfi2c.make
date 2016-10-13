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
PACKAGES-$(PTXCONF_GFI2C) += gfi2c

#
# Paths and names
#
GFI2C_VERSION	:= trunk
GFI2C			:= gfi2c
GFI2C_SUFFIX	:= 
GFI2C_URL		:= file://$(PTXDIST_WORKSPACE)/local_src/common/$(GFI2C)
GFI2C_SRCDIR	:= $(PTXDIST_WORKSPACE)/local_src/common/$(GFI2C)
GFI2C_DIR		:= $(BUILDDIR)/$(GFI2C)
GFI2C_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

$(STATEDIR)/gfi2c.extract:
	@$(call targetinfo)
	@$(call clean, $(GFI2C_DIR))
	@rm -Rf $(GFI2C_DIR)
	@cp -R $(GFI2C_SRCDIR) $(GFI2C_DIR)
	@$(call patchin, GFI2C)
	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

$(STATEDIR)/gfi2c.prepare:
	@$(call targetinfo)
	@$(call clean, $(GFI2C_DIR))
	@rm -Rf $(GFI2C_DIR)
	@cp -R $(GFI2C_SRCDIR) $(GFI2C_DIR)
	@$(call patchin, GFI2C)
	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

$(STATEDIR)/gfi2c.compile:
	@$(call targetinfo)
	@cd $(GFI2C_DIR) && PATH=$(CROSS_PATH) $(CROSS_ENV) \
		CFLAGS+=-I$(KERNEL_HEADERS_DIR)/include $(MAKE)
	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

$(STATEDIR)/gfi2c.install:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/gfi2c.targetinstall:
	@$(call targetinfo)

	@$(call install_init,  gfi2c)
	@$(call install_fixup, gfi2c,PACKAGE,gfi2c)
	@$(call install_fixup, gfi2c,PRIORITY,optional)
	@$(call install_fixup, gfi2c,VERSION,$(GFI2C_VERSION))
	@$(call install_fixup, gfi2c,SECTION,base)
	@$(call install_fixup, gfi2c,AUTHOR,"Carsten Behling <carsten.behling@garz-fricke.com>")
	@$(call install_fixup, gfi2c,DEPENDS,)
	@$(call install_fixup, gfi2c,DESCRIPTION,missing)

	@$(call install_copy, gfi2c, 0, 0, 0755, $(GFI2C_DIR)/gfi2c, /usr/local/bin/gfi2c)

	@$(call install_finish, gfi2c)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

#$(STATEDIR)/gfi2c.clean:
#	@$(call targetinfo)
#	@$(call clean_pkg, GFI2C)

# vim: syntax=make
