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
PACKAGES-$(PTXCONF_FSTYP) += fstyp

#
# Paths and names
#
FSTYP_VERSION	:= trunk
FSTYP			:= fstyp
FSTYP_SUFFIX	:= 
FSTYP_URL		:= file://$(PTXDIST_WORKSPACE)/local_src/common/$(FSTYP)
FSTYP_SRCDIR	:= $(PTXDIST_WORKSPACE)/local_src/common/$(FSTYP)
FSTYP_DIR		:= $(BUILDDIR)/$(FSTYP)
FSTYP_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

$(STATEDIR)/fstyp.extract:
	@$(call targetinfo)
	@$(call clean, $(FSTYP_DIR))
	@rm -Rf $(FSTYP_DIR)
	@cp -R $(FSTYP_SRCDIR) $(FSTYP_DIR)
	@$(call patchin, FSTYP)
	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#FSTYP_CONF_ENV	:= $(CROSS_ENV)

#
# autoconf
#
FSTYP_CONF_TOOL	:= autoconf
#FSTYP_CONF_OPT	:= $(CROSS_AUTOCONF_USR)

#$(STATEDIR)/fstyp.prepare:
#	@$(call targetinfo)
#	@$(call clean, $(FSTYP_DIR)/config.cache)
#	cd $(FSTYP_DIR) && \
#		$(FSTYP_PATH) $(FSTYP_ENV) \
#		./configure $(FSTYP_CONF_OPT)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

#$(STATEDIR)/fstyp.compile:
#	@$(call targetinfo)
#	@$(call world/compile, FSTYP)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

#$(STATEDIR)/fstyp.install:
#	@$(call targetinfo)
#	@$(call world/install, FSTYP)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/fstyp.targetinstall:
	@$(call targetinfo)

	@$(call install_init, fstyp)
	@$(call install_fixup, fstyp,PRIORITY,optional)
	@$(call install_fixup, fstyp,SECTION,base)
	@$(call install_fixup, fstyp,AUTHOR,"Carsten Behling <carsten.behling@garz-fricke.com>")
	@$(call install_fixup, fstyp,DESCRIPTION,missing)

	@$(call install_copy, fstyp, 0, 0, 0755, -, /usr/bin/fstyp)
	 
	@$(call install_finish, fstyp)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

#$(STATEDIR)/fstyp.clean:
#	@$(call targetinfo)
#	@$(call clean_pkg, FSTYP)

# vim: syntax=make
