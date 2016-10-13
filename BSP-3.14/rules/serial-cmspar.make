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
PACKAGES-$(PTXCONF_SERIAL_CMSPAR_TEST) += serial-cmspar

#
# Paths and names
#
SERIAL_CMSPAR_TEST_VERSION	:= trunk
SERIAL_CMSPAR_TEST			:= serial-cmspar
SERIAL_CMSPAR_TEST_SUFFIX	:= 
SERIAL_CMSPAR_TEST_URL		:= file://$(PTXDIST_WORKSPACE)/local_src/common/$(SERIAL_CMSPAR_TEST)
SERIAL_CMSPAR_TEST_SRCDIR	:= $(PTXDIST_WORKSPACE)/local_src/common/$(SERIAL_CMSPAR_TEST)
SERIAL_CMSPAR_TEST_DIR		:= $(BUILDDIR)/$(SERIAL_CMSPAR_TEST)
SERIAL_CMSPAR_TEST_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

$(STATEDIR)/serial-cmspar.extract:
	@$(call targetinfo)
	@$(call clean, $(SERIAL_CMSPAR_TEST_DIR))
	@rm -Rf $(SERIAL_CMSPAR_TEST_DIR)
	@cp -R $(SERIAL_CMSPAR_TEST_SRCDIR) $(SERIAL_CMSPAR_TEST_DIR)
	@$(call patchin, SERIAL_CMSPAR_TEST)
	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

$(STATEDIR)/serial-cmspar.prepare:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

$(STATEDIR)/serial-cmspar.compile:
	@$(call targetinfo)
	@cd $(SERIAL_CMSPAR_TEST_DIR) && PATH=$(CROSS_PATH) $(CROSS_ENV) $(MAKE)
	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

$(STATEDIR)/serial-cmspar.install:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/serial-cmspar.targetinstall:
	@$(call targetinfo)

	@$(call install_init,  serial-cmspar)
	@$(call install_fixup, serial-cmspar,PACKAGE,serial-cmspar)
	@$(call install_fixup, serial-cmspar,PRIORITY,optional)
	@$(call install_fixup, serial-cmspar,VERSION,$(SERIAL_CMSPAR_TEST_VERSION))
	@$(call install_fixup, serial-cmspar,SECTION,base)
	@$(call install_fixup, serial-cmspar,AUTHOR,"Tim Jaacks <tim.jaacks@garz-fricke.com>")
	@$(call install_fixup, serial-cmspar,DEPENDS,)
	@$(call install_fixup, serial-cmspar,DESCRIPTION,missing)

	@$(call install_copy, serial-cmspar, 0, 0, 0755, $(SERIAL_CMSPAR_TEST_DIR)/serial-cmspar, /usr/local/bin/serial-cmspar)

	@$(call install_finish, serial-cmspar)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

#$(STATEDIR)/serial-cmspar.clean:
#	@$(call targetinfo)
#	@$(call clean_pkg, SERIAL_CMSPAR_TEST)

# vim: syntax=make
