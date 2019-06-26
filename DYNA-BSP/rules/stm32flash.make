# -*-makefile-*-
#
# Copyright (C) 2014 by DYNA Instruments GmbH <info@dynainstruments.com>
#
# See CREDITS for details about who has contributed to this project.
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_STM32FLASH) += stm32flash

#
# Paths and names
#
STM32FLASH_VERSION	:= 0.4
STM32FLASH_MD5		:= ec9b5c8bae67f9a489786546d088bd14
STM32FLASH		:= stm32flash-$(STM32FLASH_VERSION)
STM32FLASH_SUFFIX	:= tar.gz
STM32FLASH_URL		:= http://releases.stm32flash.googlecode.com/git//$(STM32FLASH).$(STM32FLASH_SUFFIX)
STM32FLASH_SOURCE	:= $(SRCDIR)/$(STM32FLASH).$(STM32FLASH_SUFFIX)
STM32FLASH_DIR		:= $(BUILDDIR)/$(STM32FLASH)
STM32FLASH_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Get
# ----------------------------------------------------------------------------

#$(STM32FLASH_SOURCE):
#	@$(call targetinfo)
#	@$(call get, STM32FLASH)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#STM32FLASH_CONF_ENV	:= $(CROSS_ENV)

#
# autoconf
#
STM32FLASH_CONF_TOOL	:= NO
#STM32FLASH_CONF_OPT	:= $(CROSS_AUTOCONF_USR)

$(STATEDIR)/stm32flash.prepare:
	@$(call targetinfo)
#	@$(call clean, $(STM32FLASH_DIR)/config.cache)
#	cd $(STM32FLASH_DIR) && \
#		$(STM32FLASH_PATH) $(STM32FLASH_ENV) \
#		./configure $(STM32FLASH_CONF_OPT)
	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

STM32FLASH_MAKE_ENV    := $(CROSS_ENV) PREFIX=/usr

#$(STATEDIR)/stm32flash.compile:
#	@$(call targetinfo)
#	@$(call world/compile, STM32FLASH)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

#$(STATEDIR)/stm32flash.install:
#	@$(call targetinfo)
#	@$(call world/install, STM32FLASH)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/stm32flash.targetinstall:
	@$(call targetinfo)

	@$(call install_init, stm32flash)
	@$(call install_fixup, stm32flash,PRIORITY,optional)
	@$(call install_fixup, stm32flash,SECTION,base)
	@$(call install_fixup, stm32flash,AUTHOR,"DYNA Instruments GmbH <info@dynainstruments.com>")
	@$(call install_fixup, stm32flash,DESCRIPTION,"STM32 flash boot loader")

	@$(call install_copy, stm32flash, 0, 0, 0755, -, /usr/bin/stm32flash)

	@$(call install_finish, stm32flash)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

#$(STATEDIR)/stm32flash.clean:
#	@$(call targetinfo)
#	@$(call clean_pkg, STM32FLASH)

# vim: syntax=make
