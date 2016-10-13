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
PACKAGES-$(PTXCONF_EEPROMTEST) += eepromtest

#
# Paths and names
#
EEPROMTEST_VERSION	:= trunk
EEPROMTEST			:= eepromtest
EEPROMTEST_SUFFIX	:= 
EEPROMTEST_URL		:= file://$(PTXDIST_WORKSPACE)/local_src/common/$(EEPROMTEST)
EEPROMTEST_SRCDIR	:= $(PTXDIST_WORKSPACE)/local_src/common/$(EEPROMTEST)
EEPROMTEST_DIR		:= $(BUILDDIR)/$(EEPROMTEST)
EEPROMTEST_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

$(STATEDIR)/eepromtest.extract:
	@$(call targetinfo)
	@$(call clean, $(EEPROMTEST_DIR))
	@rm -Rf $(EEPROMTEST_DIR)
	@cp -R $(EEPROMTEST_SRCDIR) $(EEPROMTEST_DIR)
	@$(call patchin, EEPROMTEST)
	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

$(STATEDIR)/eepromtest.prepare:
	@$(call targetinfo)
	@$(call clean, $(EEPROMTEST_DIR))
	@rm -Rf $(EEPROMTEST_DIR)
	@cp -R $(EEPROMTEST_SRCDIR) $(EEPROMTEST_DIR)
	@$(call patchin, EEPROMTEST)
	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

$(STATEDIR)/eepromtest.compile:
	@$(call targetinfo)
	@cd $(EEPROMTEST_DIR) && PATH=$(CROSS_PATH) $(CROSS_ENV) $(MAKE)
	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

$(STATEDIR)/eepromtest.install:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/eepromtest.targetinstall:
	@$(call targetinfo)

	@$(call install_init,  eepromtest)
	@$(call install_fixup, eepromtest,PACKAGE,eepromtest)
	@$(call install_fixup, eepromtest,PRIORITY,optional)
	@$(call install_fixup, eepromtest,VERSION,$(EEPROMTEST_VERSION))
	@$(call install_fixup, eepromtest,SECTION,base)
	@$(call install_fixup, eepromtest,AUTHOR,"Carsten Behling <carsten.behling@garz-fricke.com>")
	@$(call install_fixup, eepromtest,DEPENDS,)
	@$(call install_fixup, eepromtest,DESCRIPTION,missing)

	@$(call install_copy, eepromtest, 0, 0, 0755, $(EEPROMTEST_DIR)/eepromtest, /usr/local/bin/eepromtest)

	@$(call install_finish, eepromtest)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

#$(STATEDIR)/eepromtest.clean:
#	@$(call targetinfo)
#	@$(call clean_pkg, EEPROMTEST)

# vim: syntax=make
