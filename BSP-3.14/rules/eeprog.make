# -*-makefile-*-
#
# Copyright (C) 2011 by Carsten Behling <carsten.behling@garz-fricke.com>
#
# See CREDITS for details about who has contributed to this project.
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_EEPROG) += eeprog

#
# Paths and names
#
EEPROG_VERSION	:= 0.7.6
EEPROG_MD5	:= a8460a68d36716b1b9524c2c8038f9e2
EEPROG		:= eeprog-$(EEPROG_VERSION)
EEPROG_SUFFIX	:= tar.gz
EEPROG_URL	:= http://www.codesink.org/download/$(EEPROG).$(EEPROG_SUFFIX)
EEPROG_SOURCE	:= $(SRCDIR)/$(EEPROG).$(EEPROG_SUFFIX)
EEPROG_DIR	:= $(BUILDDIR)/$(EEPROG)
EEPROG_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Get
# ----------------------------------------------------------------------------

#$(EEPROG_SOURCE):
#	@$(call targetinfo)
#	@$(call get, EEPROG)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

$(STATEDIR)/eeprog.prepare:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

$(STATEDIR)/eeprog.compile:
	@$(call targetinfo)
	@cd $(EEPROG_DIR) && PATH=$(CROSS_PATH) $(CROSS_ENV) $(MAKE)
	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

$(STATEDIR)/eeprog.install:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/eeprog.targetinstall:
	@$(call targetinfo)

	@$(call install_init, eeprog)
	@$(call install_fixup, eeprog,PRIORITY,optional)
	@$(call install_fixup, eeprog,SECTION,base)
	@$(call install_fixup, eeprog,AUTHOR,"Carsten Behling <carsten.behling@garz-fricke.com>")
	@$(call install_fixup, eeprog,DESCRIPTION,missing)

	@$(call install_copy, eeprog, 0, 0, 0755, $(EEPROG_DIR)/eeprog, /usr/bin/eeprog)

	@$(call install_finish, eeprog)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

#$(STATEDIR)/eeprog.clean:
#	@$(call targetinfo)
#	@$(call clean_pkg, EEPROG)

# vim: syntax=make
