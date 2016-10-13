# -*-makefile-*-
#
# Copyright (C) 2010 by Sakire Aytac <sakire.aytac@garz-fricke.com>
#
# See CREDITS for details about who has contributed to this project.
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_EVTEST) += evtest

#
# Paths and names
#
EVTEST_VERSION	:= git-head
EVTEST			:= evtest
EVTEST_SUFFIX	:= 
EVTEST_URL		:= file://$(PTXDIST_WORKSPACE)/local_src/common/$(EVTEST)
EVTEST_SRCDIR	:= $(PTXDIST_WORKSPACE)/local_src/common/$(EVTEST)
EVTEST_DIR		:= $(BUILDDIR)/$(EVTEST)
EVTEST_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

$(STATEDIR)/evtest.extract:
	@$(call targetinfo)
	@$(call clean, $(EVTEST_DIR))
	@test -d $(EVTEST_SRCDIR)/.git || rm -Rf $(EVTEST_SRCDIR) && mkdir -p $(EVTEST_SRCDIR)
	@cd $(EVTEST_SRCDIR) && git clone http://anongit.freedesktop.org/git/evtest . \
		|| git checkout -- .
	@rm -Rf $(EVTEST_DIR)
	@cp -R $(EVTEST_SRCDIR) $(EVTEST_DIR)
	@$(call patchin, EVTEST)
	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

EVTEST_CONF_ENV	:= $(CROSS_ENV) CONFIG_SITE="no"

#
# autoconf
#
EVTEST_CONF_TOOL	:= autoconf
EVTEST_CONF_OPT	:= $(CROSS_AUTOCONF_USR)

$(STATEDIR)/evtest.prepare:
	@$(call targetinfo)
	@$(call clean, $(EVTEST_DIR)/config.cache)
	@cd $(EVTEST_DIR) && \
		$(EVTEST_PATH) $(EVTEST_ENV) $(EVTEST_CONF_ENV) \
		./autogen.sh $(EVTEST_CONF_OPT)
	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

#$(STATEDIR)/evtest.compile:
#	@$(call targetinfo)
#	@$(call world/compile, EVTEST)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

#$(STATEDIR)/urjtag.install:
#	@$(call targetinfo)
#	@$(call world/install, EVTEST)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/evtest.targetinstall:
	@$(call targetinfo)

	@$(call install_init,  evtest)
	@$(call install_fixup, evtest,PACKAGE,evtest)
	@$(call install_fixup, evtest,PRIORITY,optional)
	@$(call install_fixup, evtest,VERSION,$(EVTEST_VERSION))
	@$(call install_fixup, evtest,SECTION,base)
	@$(call install_fixup, evtest,AUTHOR,"DYNA Instruments GmbH <info@dynainstruments.com>")
	@$(call install_fixup, evtest,DEPENDS,)
	@$(call install_fixup, evtest,DESCRIPTION,missing)

	@$(call install_tree, evtest, 0, 0, -, /usr/bin)

	@$(call install_finish, evtest)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

#$(STATEDIR)/evtest.clean:
#	@$(call targetinfo)
#	@$(call clean_pkg, EVTEST)

# vim: syntax=make
