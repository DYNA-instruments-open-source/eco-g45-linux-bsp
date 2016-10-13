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
PACKAGES-$(PTXCONF_RNG_TOOLS) += rng-tools

#
# Paths and names
#
RNG_TOOLS_VERSION	:= 3
RNG_TOOLS_MD5		:= fa305916ec101c85c0065aeceb81a38d
RNG_TOOLS		:= rng-tools-$(RNG_TOOLS_VERSION)
RNG_TOOLS_SUFFIX	:= tar.gz
RNG_TOOLS_URL		:= http://pkgs.fedoraproject.org/repo/pkgs/rng-tools/rng-tools-3.tar.gz/fa305916ec101c85c0065aeceb81a38d/$(RNG_TOOLS).$(RNG_TOOLS_SUFFIX)
RNG_TOOLS_SOURCE	:= $(SRCDIR)/$(RNG_TOOLS).$(RNG_TOOLS_SUFFIX)
RNG_TOOLS_DIR		:= $(BUILDDIR)/$(RNG_TOOLS)
RNG_TOOLS_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Get
# ----------------------------------------------------------------------------

#$(RNG_TOOLS_SOURCE):
#	@$(call targetinfo)
#	@$(call get, RNG_TOOLS)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#RNG_TOOLS_CONF_ENV	:= $(CROSS_ENV)

#
# autoconf
#
RNG_TOOLS_CONF_TOOL	:= autoconf
#RNG_TOOLS_CONF_OPT	:= $(CROSS_AUTOCONF_USR)

#$(STATEDIR)/rng-tools.prepare:
#	@$(call targetinfo)
#	@$(call clean, $(RNG_TOOLS_DIR)/config.cache)
#	cd $(RNG_TOOLS_DIR) && \
#		$(RNG_TOOLS_PATH) $(RNG_TOOLS_ENV) \
#		./configure $(RNG_TOOLS_CONF_OPT)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

#$(STATEDIR)/rng-tools.compile:
#	@$(call targetinfo)
#	@$(call world/compile, RNG_TOOLS)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

#$(STATEDIR)/rng-tools.install:
#	@$(call targetinfo)
#	@$(call world/install, RNG_TOOLS)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/rng-tools.targetinstall:
	@$(call targetinfo)

	@$(call install_init, rng-tools)
	@$(call install_fixup, rng-tools,PRIORITY,optional)
	@$(call install_fixup, rng-tools,SECTION,base)
	@$(call install_fixup, rng-tools,AUTHOR,"Carsten Behling <carsten.behling@garz-fricke.com>")
	@$(call install_fixup, rng-tools,DESCRIPTION,missing)

	@$(call install_copy, rng-tools, 0, 0, 0755, $(RNG_TOOLS_DIR)/rngtest, /usr/bin/rngtest)
	@$(call install_copy, rng-tools, 0, 0, 0755, $(RNG_TOOLS_DIR)/rngd, /usr/sbin/rngd)

	@$(call install_alternative, rng-tools, 0, 0, 0755, \
		/etc/init.d/rngd)
	@$(call install_link, rng-tools, ../init.d/rngd, \
		/etc/rc.d/S91rngd)

	@$(call install_finish, rng-tools)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

#$(STATEDIR)/rng-tools.clean:
#	@$(call targetinfo)
#	@$(call clean_pkg, RNG_TOOLS)

# vim: syntax=make
