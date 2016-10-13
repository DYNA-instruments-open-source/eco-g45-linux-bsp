# -*-makefile-*-
#
# Copyright (C) 2011 by Carsten Behling
#
# See CREDITS for details about who has contributed to this project.
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_QT4_GUF_DBUS) += qt4-guf-dbus

#
# Paths and names
#
QT4_GUF_DBUS_VERSION	:= trunk
QT4_GUF_DBUS			:= qt4-guf-dbus
QT4_GUF_DBUS_SUFFIX		:=
QT4_GUF_DBUS_SRCDIR		:= $(PTXDIST_WORKSPACE)/local_src/common/$(QT4_GUF_DBUS)
QT4_GUF_DBUS_DIR		:= $(BUILDDIR)/$(QT4_GUF_DBUS)

# ----------------------------------------------------------------------------
# Get
# ----------------------------------------------------------------------------

$(QT4_GUF_DBUS_SOURCE):
	@$(call targetinfo)
	@$(call get, QT4_GUF_DBUS)

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

$(STATEDIR)/qt4-guf-dbus.extract:
	@$(call targetinfo)
	@$(call clean, $(QT4_GUF_DBUS_DIR))
#	@mkdir -p $(QT4_GUF_DBUS_DIR)
	@rm -Rf $(QT4_GUF_DBUS_DIR)
	@cp -R $(QT4_GUF_DBUS_SRCDIR) $(QT4_GUF_DBUS_DIR)
	@$(call patchin, QT4_GUF_DBUS)
	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

QT4_GUF_DBUS_PATH	:= PATH=$(CROSS_PATH)

QT4_GUF_DBUS_ENV = \
	$(CROSS_ENV) \
	QMAKEPATH=$(QT4_DIR) \
	INSTALL_ROOT=$(SYSROOT) \
	QMAKESPEC=$(QT4_DIR)/mkspecs/qws/linux-ptx-g++

#
# autoconf
#
QT4_GUF_DBUS_AUTOCONF := $(CROSS_AUTOCONF_USR)

$(STATEDIR)/qt4-guf-dbus.prepare:
	@$(call targetinfo)
	cd $(QT4_GUF_DBUS_DIR) && \
		$(QT4_GUF_DBUS_PATH) $(QT4_GUF_DBUS_ENV) qmake qt4-guf-dbus.pro
	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

$(STATEDIR)/qt4-guf-dbus.compile:
	@$(call targetinfo)
	cd $(QT4_GUF_DBUS_DIR) && $(QT4_GUF_DBUS_PATH) $(MAKE) $(PARALLELMFLAGS)
	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

$(STATEDIR)/qt4-guf-dbus.install:
	@$(call targetinfo)
	@$(call install, QT4_GUF_DBUS)
	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/qt4-guf-dbus.targetinstall:
	@$(call targetinfo)

	@$(call install_init, qt4-guf-dbus)
	@$(call install_fixup, qt4-guf-dbus,PACKAGE,qt4-guf-dbus)
	@$(call install_fixup, qt4-guf-dbus,PRIORITY,optional)
	@$(call install_fixup, qt4-guf-dbus,VERSION,$(QT4_GUF_DBUS_VERSION))
	@$(call install_fixup, qt4-guf-dbus,SECTION,base)
	@$(call install_fixup, qt4-guf-dbus,AUTHOR,"Carsten Behling")
	@$(call install_fixup, qt4-guf-dbus,DEPENDS,)
	@$(call install_fixup, qt4-guf-dbus,DESCRIPTION,missing)

	#
	# Application
	#
	@$(call install_copy, qt4-guf-dbus, 0, 0, 0755, \
		$(QT4_GUF_DBUS_DIR)/qt4-guf-dbus, \
		/usr/bin/qt4-guf-dbus)

	@$(call install_finish, qt4-guf-dbus)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

#$(STATEDIR)/qt4-guf-dbus.clean:
#	@$(call targetinfo)
#	@$(call clean_pkg, QT4_GUF_DBUS)

# vim: syntax=make
