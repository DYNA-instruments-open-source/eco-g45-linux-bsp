# -*-makefile-*-
#
# Copyright (C) 2010 by Carsten Behling
#
# See CREDITS for details about who has contributed to this project.
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_QT4_GUF_SLIDESHOW) += qt4-guf-slideshow

#
# Paths and names
#
QT4_GUF_SLIDESHOW_VERSION	:= trunk
QT4_GUF_SLIDESHOW			:= qt4-guf-slideshow
QT4_GUF_SLIDESHOW_SUFFIX	:=
QT4_GUF_SLIDESHOW_SRCDIR	:= $(PTXDIST_WORKSPACE)/local_src/common/$(QT4_GUF_SLIDESHOW)
QT4_GUF_SLIDESHOW_DIR		:= $(BUILDDIR)/$(QT4_GUF_SLIDESHOW)

# ----------------------------------------------------------------------------
# Get
# ----------------------------------------------------------------------------

$(QT4_GUF_SLIDESHOW_SOURCE):
	@$(call targetinfo)
	@$(call get, QT4_GUF_SLIDESHOW)

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

$(STATEDIR)/qt4-guf-slideshow.extract:
	@$(call targetinfo)
	@$(call clean, $(QT4_GUF_SLIDESHOW_DIR))
#	@mkdir -p $(QT4_GUF_SLIDESHOW_DIR)
	@rm -Rf $(QT4_GUF_SLIDESHOW_DIR)
	@cp -R $(QT4_GUF_SLIDESHOW_SRCDIR) $(QT4_GUF_SLIDESHOW_DIR)
	@$(call patchin, QT4_GUF_SLIDESHOW)
	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

QT4_GUF_SLIDESHOW_PATH	:= PATH=$(CROSS_PATH)

QT4_GUF_SLIDESHOW_ENV = \
	$(CROSS_ENV) \
	QMAKEPATH=$(QT4_DIR) \
	INSTALL_ROOT=$(SYSROOT) \
	QMAKESPEC=$(QT4_DIR)/mkspecs/qws/linux-ptx-g++

#
# autoconf
#
QT4_GUF_SLIDESHOW_AUTOCONF := $(CROSS_AUTOCONF_USR)

$(STATEDIR)/qt4-guf-slideshow.prepare:
	@$(call targetinfo)
	cd $(QT4_GUF_SLIDESHOW_DIR) && \
		$(QT4_GUF_SLIDESHOW_PATH) $(QT4_GUF_SLIDESHOW_ENV) qmake qt4-guf-slideshow.pro
	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

$(STATEDIR)/qt4-guf-slideshow.compile:
	@$(call targetinfo)
	cd $(QT4_GUF_SLIDESHOW_DIR) && $(QT4_GUF_SLIDESHOW_PATH) $(MAKE) $(PARALLELMFLAGS)
	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

$(STATEDIR)/qt4-guf-slideshow.install:
	@$(call targetinfo)
	@$(call install, QT4_GUF_SLIDESHOW)
	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/qt4-guf-slideshow.targetinstall:
	@$(call targetinfo)

	@$(call install_init, qt4-guf-slideshow)
	@$(call install_fixup, qt4-guf-slideshow,PACKAGE,qt4-guf-slideshow)
	@$(call install_fixup, qt4-guf-slideshow,PRIORITY,optional)
	@$(call install_fixup, qt4-guf-slideshow,VERSION,$(QT4_GUF_SLIDESHOW_VERSION))
	@$(call install_fixup, qt4-guf-slideshow,SECTION,base)
	@$(call install_fixup, qt4-guf-slideshow,AUTHOR,"Carsten Behling")
	@$(call install_fixup, qt4-guf-slideshow,DEPENDS,)
	@$(call install_fixup, qt4-guf-slideshow,DESCRIPTION,missing)

	#
	# Application
	#
	@$(call install_copy, qt4-guf-slideshow, 0, 0, 0755, \
		$(QT4_GUF_SLIDESHOW_DIR)/qt4-guf-slideshow, \
		/usr/bin/qt4-guf-slideshow)
		
	#
	# Default slides
	#
	@$(call install_copy, qt4-guf-slideshow, 0, 0, 0755, \
		$(QT4_GUF_SLIDESHOW_DIR)/images/1.jpg, \
		/usr/share/qt4-guf-slideshow/images/1.jpg)
		
	@$(call install_copy, qt4-guf-slideshow, 0, 0, 0755, \
		$(QT4_GUF_SLIDESHOW_DIR)/images/2.jpg, \
		/usr/share/qt4-guf-slideshow/images/2.jpg)
		
	@$(call install_copy, qt4-guf-slideshow, 0, 0, 0755, \
		$(QT4_GUF_SLIDESHOW_DIR)/images/3.jpg, \
		/usr/share/qt4-guf-slideshow/images/3.jpg)
	
	@$(call install_copy, qt4-guf-slideshow, 0, 0, 0755, \
		$(QT4_GUF_SLIDESHOW_DIR)/images/4.jpg, \
		/usr/share/qt4-guf-slideshow/images/4.jpg)

	@$(call install_copy, qt4-guf-slideshow, 0, 0, 0755, \
		$(QT4_GUF_SLIDESHOW_DIR)/images/5.jpg, \
		/usr/share/qt4-guf-slideshow/images/5.jpg)	
	
	@$(call install_copy, qt4-guf-slideshow, 0, 0, 0755, \
		$(QT4_GUF_SLIDESHOW_DIR)/images/6.jpg, \
		/usr/share/qt4-guf-slideshow/images/6.jpg)

	@$(call install_copy, qt4-guf-slideshow, 0, 0, 0755, \
		$(QT4_GUF_SLIDESHOW_DIR)/images/7.jpg, \
		/usr/share/qt4-guf-slideshow/images/7.jpg)		
		
	#
	# Startup script
	#
	@$(call install_alternative, qt4-guf-slideshow, 0, 0, 0755, \
		/etc/init.d/qt4-guf-slideshow)
	#@$(call install_link, qt4-guf-slideshow, ../init.d/qt4-guf-slideshow, \
	#	/etc/rc.d/S95qt4-guf-slideshow)

	@$(call install_finish, qt4-guf-slideshow)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

#$(STATEDIR)/qt4-guf-slideshow.clean:
#	@$(call targetinfo)
#	@$(call clean_pkg, QT4_GUF_SLIDESHOW)

# vim: syntax=make
