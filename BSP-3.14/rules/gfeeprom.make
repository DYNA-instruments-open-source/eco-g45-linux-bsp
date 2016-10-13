# -*-makefile-*-
#
# Copyright (C) 2011 by Nils Grimm <nils.grimm@garz-fricke.com>
#
# See CREDITS for details about who has contributed to this project.
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_GFEEPROM) += gfeeprom

#
# Paths and names
#
GFEEPROM_VERSION	:= trunk
GFEEPROM			:= gfeeprom
GFEEPROM_SUFFIX		:=
GFEEPROM_URL		:= file://$(PTXDIST_WORKSPACE)/local_src/common/$(GFEEPROM)
GFEEPROM_SRCDIR		:= $(PTXDIST_WORKSPACE)/local_src/common/$(GFEEPROM)
GFEEPROM_DIR		:= $(BUILDDIR)/$(GFEEPROM)
GFEEPROM_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

$(STATEDIR)/gfeeprom.extract:
	@$(call targetinfo)
	@$(call clean, $(GFEEPROM_DIR))
	@rm -Rf $(GFEEPROM_DIR)
	@cp -R $(GFEEPROM_SRCDIR) $(GFEEPROM_DIR)
	@$(call patchin, GFEEPROM)
	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

$(STATEDIR)/gfeeprom.prepare:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

$(STATEDIR)/gfeeprom.compile:
	@$(call targetinfo)
	@cd $(GFEEPROM_DIR) && PATH=$(CROSS_PATH) $(CROSS_ENV) CFLAGS+=-I$(KERNEL_HEADERS_DIR)/include $(MAKE)
	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

$(STATEDIR)/gfeeprom.install:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/gfeeprom.targetinstall:
	@$(call targetinfo)

	@$(call install_init,  gfeeprom)
	@$(call install_fixup, gfeeprom,PACKAGE,gfeeprom)
	@$(call install_fixup, gfeeprom,PRIORITY,optional)
	@$(call install_fixup, gfeeprom,VERSION,$(GFEEPROM_VERSION))
	@$(call install_fixup, gfeeprom,SECTION,base)
	@$(call install_fixup, gfeeprom,AUTHOR,"Nils Grimm <nils.grimm@garz-fricke.com>")
	@$(call install_fixup, gfeeprom,DEPENDS,)
	@$(call install_fixup, gfeeprom,DESCRIPTION,missing)

	@$(call install_copy, gfeeprom, 0, 0, 0755, $(GFEEPROM_DIR)/gfeeprom, /usr/local/bin/gfeeprom)

	@$(call install_finish, gfeeprom)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

#$(STATEDIR)/gfeeprom.clean:
#	@$(call targetinfo)
#	@$(call clean_pkg, GFEEPROM)

# vim: syntax=make
