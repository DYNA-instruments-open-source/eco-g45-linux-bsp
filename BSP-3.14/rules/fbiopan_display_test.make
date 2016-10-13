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
PACKAGES-$(PTXCONF_FBIOPAN_DISPLAY_TEST) += fbiopan_display_test

#
# Paths and names
#
FBIOPAN_DISPLAY_TEST_VERSION	:= trunk
FBIOPAN_DISPLAY_TEST			:= fbiopan_display_test
FBIOPAN_DISPLAY_TEST_SUFFIX	:= 
FBIOPAN_DISPLAY_TEST_URL		:= file://$(PTXDIST_WORKSPACE)/local_src/common/$(FBIOPAN_DISPLAY_TEST)
FBIOPAN_DISPLAY_TEST_SRCDIR	:= $(PTXDIST_WORKSPACE)/local_src/common/$(FBIOPAN_DISPLAY_TEST)
FBIOPAN_DISPLAY_TEST_DIR		:= $(BUILDDIR)/$(FBIOPAN_DISPLAY_TEST)
FBIOPAN_DISPLAY_TEST_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

$(STATEDIR)/fbiopan_display_test.extract:
	@$(call targetinfo)
	@$(call clean, $(FBIOPAN_DISPLAY_TEST_DIR))
	@rm -Rf $(FBIOPAN_DISPLAY_TEST_DIR)
	@cp -R $(FBIOPAN_DISPLAY_TEST_SRCDIR) $(FBIOPAN_DISPLAY_TEST_DIR)
	@$(call patchin, FBIOPAN_DISPLAY_TEST)
	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

$(STATEDIR)/fbiopan_display_test.prepare:
	@$(call targetinfo)
	@$(call clean, $(FBIOPAN_DISPLAY_TEST_DIR))
	@rm -Rf $(FBIOPAN_DISPLAY_TEST_DIR)
	@cp -R $(FBIOPAN_DISPLAY_TEST_SRCDIR) $(FBIOPAN_DISPLAY_TEST_DIR)
	@$(call patchin, FBIOPAN_DISPLAY_TEST)
	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

$(STATEDIR)/fbiopan_display_test.compile:
	@$(call targetinfo)
	@cd $(FBIOPAN_DISPLAY_TEST_DIR) && PATH=$(CROSS_PATH) $(CROSS_ENV) $(MAKE)
	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

$(STATEDIR)/fbiopan_display_test.install:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/fbiopan_display_test.targetinstall:
	@$(call targetinfo)

	@$(call install_init,  fbiopan_display_test)
	@$(call install_fixup, fbiopan_display_test,PACKAGE,fbiopan_display_test)
	@$(call install_fixup, fbiopan_display_test,PRIORITY,optional)
	@$(call install_fixup, fbiopan_display_test,VERSION,$(FBIOPAN_DISPLAY_TEST_VERSION))
	@$(call install_fixup, fbiopan_display_test,SECTION,base)
	@$(call install_fixup, fbiopan_display_test,AUTHOR,"Tim Jaacks <tim.jaacks@garz-fricke.com>")
	@$(call install_fixup, fbiopan_display_test,DEPENDS,)
	@$(call install_fixup, fbiopan_display_test,DESCRIPTION,missing)

	@$(call install_copy, fbiopan_display_test, 0, 0, 0755, $(FBIOPAN_DISPLAY_TEST_DIR)/fbiopan_display_test, /usr/local/bin/fbiopan_display_test)

	@$(call install_finish, fbiopan_display_test)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

#$(STATEDIR)/fbiopan_display_test.clean:
#	@$(call targetinfo)
#	@$(call clean_pkg, FBIOPAN_DISPLAY_TEST)

# vim: syntax=make
