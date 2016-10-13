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
PACKAGES-$(PTXCONF_TOUCHCONFIG_STARTSCRIPT) += touchconfig

#
# Paths and names
#
TOUCHCONFIG_STARTSCRIPT_VERSION	:= 1.0

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/touchconfig.targetinstall:
	@$(call targetinfo)

	@$(call install_init,  touchconfig)
	@$(call install_fixup, touchconfig, PRIORITY, optional)
	@$(call install_fixup, touchconfig, VERSION, TOUCHCONFIG_STARTSCRIPT_VERSION)
	@$(call install_fixup, touchconfig, SECTION, base)
	@$(call install_fixup, touchconfig, AUTHOR, "Tim Jaacks <tim.jaacks@garz-fricke.com>")
	@$(call install_fixup, touchconfig, DESCRIPTION, missing)

	@$(call install_alternative, touchconfig, 0, 0, 0755, /etc/init.d/touchconfig, n)
ifneq ($(call remove_quotes,$(PTXCONF_TOUCHCONFIG_BBINIT_LINK)),)
	@$(call install_link, touchconfig, \
		../init.d/touchconfig, \
		/etc/rc.d/$(PTXCONF_TOUCHCONFIG_BBINIT_LINK))
endif

	@$(call install_finish, touchconfig)
	@$(call touch)

# vim: syntax=make
