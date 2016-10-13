# -*-makefile-*-
#
# Copyright (C) 2012 by Tim Jaacks <tim.jaacks@garz-fricke.com>
#
# See CREDITS for details about who has contributed to this project.
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_MDBTEST) += mdbtest

#
# Paths and names
#
MDBTEST_VERSION	:= trunk
MDBTEST			:= mdbtest
MDBTEST_SUFFIX	:= 
MDBTEST_URL		:= file://$(PTXDIST_WORKSPACE)/local_src/common/libmdb_test
MDBTEST_SRCDIR	:= $(PTXDIST_WORKSPACE)/local_src/common/libmdb_test
MDBTEST_DIR		:= $(BUILDDIR)/$(MDBTEST)
MDBTEST_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

$(STATEDIR)/mdbtest.extract:
	@$(call targetinfo)
	@$(call clean, $(MDBTEST_DIR))
	@rm -Rf $(MDBTEST_DIR)
	@cp -R $(MDBTEST_SRCDIR) $(MDBTEST_DIR)
	@$(call patchin, MDBTEST)
	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

$(STATEDIR)/mdbtest.prepare:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

$(STATEDIR)/mdbtest.compile:
	@$(call targetinfo)
	@cd $(MDBTEST_DIR) && PATH=$(CROSS_PATH) $(CROSS_ENV) $(MAKE)
	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

$(STATEDIR)/mdbtest.install:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/mdbtest.targetinstall:
	@$(call targetinfo)

	@$(call install_init,  mdbtest)
	@$(call install_fixup, mdbtest, PACKAGE, mdbtest)
	@$(call install_fixup, mdbtest, PRIORITY, optional)
	@$(call install_fixup, mdbtest, VERSION, $(MDBTEST_VERSION))
	@$(call install_fixup, mdbtest, SECTION, base)
	@$(call install_fixup, mdbtest, AUTHOR, "Tim Jaacks <tim.jaacks@garz-fricke.com>")
	@$(call install_fixup, mdbtest, DEPENDS,)
	@$(call install_fixup, mdbtest, DESCRIPTION, missing)

	@$(call install_copy, mdbtest, 0, 0, 0755, $(MDBTEST_DIR)/mdbtest, /usr/local/bin/mdbtest)

	@$(call install_finish, mdbtest)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

#$(STATEDIR)/mdbtest.clean:
#	@$(call targetinfo)
#	@$(call clean_pkg, MDBTEST)

# vim: syntax=make
