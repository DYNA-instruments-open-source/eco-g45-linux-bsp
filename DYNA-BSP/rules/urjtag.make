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
PACKAGES-$(PTXCONF_URJTAG) += urjtag

#
# Paths and names
#
URJTAG_VERSION	:= git-7ba12da7845af7601e014a2a107670edc5d6997d
URJTAG_MD5		:= 855dae4d210ea805bad557f9ecad155b
#URJTAG_MD5		:= c685c9bb33bbfa73d6ab7bacb92e6268
URJTAG		:= urjtag-$(URJTAG_VERSION)
URJTAG_SUFFIX	:= zip
URJTAG_URL		:= http://sourceforge.net/code-snapshots/git/u/ur/urjtag/git.git/$(URJTAG).$(URJTAG_SUFFIX)
#URJTAG_SUFFIX	:= tar.bz2
#URJTAG_URL		:= http://downloads.sourceforge.net/project/urjtag/urjtag/$(URJTAG_VERSION)/$(URJTAG).$(URJTAG_SUFFIX)
URJTAG_SOURCE	:= $(SRCDIR)/$(URJTAG).$(URJTAG_SUFFIX)
URJTAG_DIR		:= $(BUILDDIR)/$(URJTAG)
URJTAG_LICENSE	:= GPL-V2

# ----------------------------------------------------------------------------
# Get
# ----------------------------------------------------------------------------

#$(URJTAG_SOURCE):
#	@$(call targetinfo)
#	@$(call get, URJTAG)

# ----------------------------------------------------------------------------
# extract
# ----------------------------------------------------------------------------

$(STATEDIR)/urjtag.extract:
	@$(call targetinfo)
	@$(call clean, $(URJTAG_DIR))
	@$(call extract, URJTAG)
	cd $(URJTAG_DIR) && \
		rm -rf web && \
		mv urjtag __urjtag && \
		mv __urjtag/* ./ && \
		rm -rf __urjtag
	@$(call patchin, URJTAG)
	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

URJTAG_CONF_ENV	:= $(CROSS_ENV) CONFIG_SITE="no"

#
# autoconf
#
URJTAG_CONF_TOOL	:= autoconf
URJTAG_CONF_OPT	:= $(CROSS_AUTOCONF_USR) \
	--disable-nls \
	--disable-rpath \
	--disable-python \
	--without-libusb \
	--without-libftdi \
	--without-ftd2xx \
	--without-inpout32 \
	--disable-bus \
	--enable-cable=gpio \
	--disable-lowlevel

$(STATEDIR)/urjtag.prepare:
	@$(call targetinfo)
	@$(call clean, $(URJTAG_DIR)/config.cache)
	cd $(URJTAG_DIR) && \
		$(URJTAG_PATH) $(URJTAG_ENV) $(URJTAG_CONF_ENV) \
		./autogen.sh $(URJTAG_CONF_OPT)
	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

#$(STATEDIR)/urjtag.compile:
#	@$(call targetinfo)
#	@$(call world/compile, URJTAG)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

#$(STATEDIR)/urjtag.install:
#	@$(call targetinfo)
#	@$(call world/install, URJTAG)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/urjtag.targetinstall:
	@$(call targetinfo)

	@$(call install_init, urjtag)
	@$(call install_fixup, urjtag,PACKAGE,urjtag)
	@$(call install_fixup, urjtag,VERSION,$(URJTAG_VERSION))
	@$(call install_fixup, urjtag,PRIORITY,optional)
	@$(call install_fixup, urjtag,SECTION,base)
	@$(call install_fixup, urjtag,AUTHOR,"DYNA Instruments GmbH <info@dynainstruments.com>")
	@$(call install_fixup, urjtag,DESCRIPTION,missing)

	@$(call install_tree, urjtag, 0, 0, -, /usr/bin)
	@$(call install_tree, urjtag, 0, 0, -, /usr/share/urjtag)
	@$(call install_copy, urjtag, 0, 0, 0755, -, /usr/lib/liburjtag.so.0.0.0)
	@$(call install_link, urjtag, liburjtag.so.0.0.0, /usr/lib/liburjtag.so.0)
	@$(call install_link, urjtag, liburjtag.so.0.0.0, /usr/lib/liburjtag.so)

	@$(call install_finish, urjtag)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

#$(STATEDIR)/urjtag.clean:
#	@$(call targetinfo)
#	@$(call clean_pkg, URJTAG)

# vim: syntax=make
