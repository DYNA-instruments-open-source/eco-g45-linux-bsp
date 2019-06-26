# -*-makefile-*-
#
# Copyright (C) 2014 by Andreas Pretzsch <apr-DoOW80uwmGKzQB+pC5nmwQ (AT) public (DOT) gmane.org>
#
# See CREDITS for details about who has contributed to this project.
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

PACKAGES-$(PTXCONF_TRIGGERHAPPY) += triggerhappy

TRIGGERHAPPY_VERSION := 0.3.4
TRIGGERHAPPY_MD5 := 9719903899b3082e75a7ff6e9c00752d
TRIGGERHAPPY := triggerhappy-$(TRIGGERHAPPY_VERSION)
TRIGGERHAPPY_SUFFIX := tar.gz
TRIGGERHAPPY_URL := https://github.com/wertarbyte/triggerhappy/archive/release/0.3.4.tar.gz
TRIGGERHAPPY_SOURCE := $(SRCDIR)/$(TRIGGERHAPPY).$(TRIGGERHAPPY_SUFFIX)
TRIGGERHAPPY_DIR := $(BUILDDIR)/$(TRIGGERHAPPY)
TRIGGERHAPPY_LICENSE := GPLv3+

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

TRIGGERHAPPY_CONF_TOOL := NO

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

TRIGGERHAPPY_PATH := PATH=$(CROSS_PATH)
TRIGGERHAPPY_MAKE_ENV := \
	$(CROSS_ENV) \
	EXTRAFLAGS="$(CROSS_CPPFLAGS) $(CROSS_LDFLAGS)"

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/triggerhappy.targetinstall:
	@$(call targetinfo)
	
	@$(call install_init, triggerhappy)
	@$(call install_fixup, triggerhappy,PRIORITY,optional)
	@$(call install_fixup, triggerhappy,SECTION,base)
	@$(call install_fixup, triggerhappy,AUTHOR,"Andreas Pretzsch <apr@cn-eng.de>")
	@$(call install_fixup, triggerhappy,DESCRIPTION,"thd - triggerhappy global hotkey daemon")
	
	@$(call install_copy, triggerhappy, 0, 0, 0755, -, /usr/sbin/thd)
	@$(call install_copy, triggerhappy, 0, 0, 0755, -, /usr/sbin/th-cmd)
	
ifdef PTXCONF_INITMETHOD_BBINIT
ifdef PTXCONF_TRIGGERHAPPY_STARTSCRIPT
	@$(call install_alternative, triggerhappy, 0, 0, 0755, /etc/init.d/triggerhappy)
	@$(call install_replace, triggerhappy, /etc/init.d/triggerhappy, \
	@TRIGGERS@, "$(PTXCONF_TRIGGERHAPPY_TRIGGERS)")
	@$(call install_replace, triggerhappy, /etc/init.d/triggerhappy, \
	@USER@, "$(PTXCONF_TRIGGERHAPPY_USER)")
	@$(call install_replace, triggerhappy, /etc/init.d/triggerhappy, \
	@INPUT_DEV@, "$(PTXCONF_TRIGGERHAPPY_INPUT_DEV)")
ifneq ($(call remove_quotes,$(PTXCONF_TRIGGERHAPPY_BBINIT_LINK)),)
	@$(call install_link, triggerhappy, \
		../init.d/triggerhappy, \
		/etc/rc.d/$(PTXCONF_TRIGGERHAPPY_BBINIT_LINK))
endif
endif
endif
	
ifdef PTXCONF_TRIGGERHAPPY_TRIGGERS_FILE
	@$(call install_alternative, triggerhappy, 0, 0, 0644, \
		$(PTXCONF_TRIGGERHAPPY_TRIGGERS))
endif
ifdef PTXCONF_TRIGGERHAPPY_TRIGGERS_DIR
	@$(call install_alternative_tree, triggerhappy, 0, 0, \
		$(PTXCONF_TRIGGERHAPPY_TRIGGERS))
endif
	
	@$(call install_finish, triggerhappy)
	
	@$(call touch)

# vim: syntax=make
