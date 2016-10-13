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
PACKAGES-$(PTXCONF_QT4_GUF_DEMO) += qt4-guf-demo

#
# Paths and names
#
QT4_GUF_DEMO_VERSION	:= trunk
QT4_GUF_DEMO			:= qt4-guf-demo
QT4_GUF_DEMO_SUFFIX		:=
QT4_GUF_DEMO_SRCDIR		:= $(PTXDIST_WORKSPACE)/local_src/common/$(QT4_GUF_DEMO)
QT4_GUF_DEMO_DIR		:= $(BUILDDIR)/$(QT4_GUF_DEMO)

# ----------------------------------------------------------------------------
# Get
# ----------------------------------------------------------------------------

$(QT4_GUF_DEMO_SOURCE):
	@$(call targetinfo)
	@$(call get, QT4_GUF_DEMO)

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

$(STATEDIR)/qt4-guf-demo.extract:
	@$(call targetinfo)
	@$(call clean, $(QT4_GUF_DEMO_DIR))
#	@mkdir -p $(QT4_GUF_DEMO_DIR)
	@rm -Rf $(QT4_GUF_DEMO_DIR)
	@cp -R $(QT4_GUF_DEMO_SRCDIR) $(QT4_GUF_DEMO_DIR)
	@$(call patchin, QT4_GUF_DEMO)
	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

QT4_GUF_DEMO_PATH	:= PATH=$(CROSS_PATH)

QT4_GUF_DEMO_ENV = \
	$(CROSS_ENV) \
	QMAKEPATH=$(QT4_DIR) \
	INSTALL_ROOT=$(SYSROOT) \
	QMAKESPEC=$(QT4_DIR)/mkspecs/qws/linux-ptx-g++

#
# autoconf
#
QT4_GUF_DEMO_AUTOCONF := $(CROSS_AUTOCONF_USR)

$(STATEDIR)/qt4-guf-demo.prepare:
	@$(call targetinfo)
	cd $(QT4_GUF_DEMO_DIR) && \
		$(QT4_GUF_DEMO_PATH) $(QT4_GUF_DEMO_ENV) qmake qt4-guf-demo.pro
	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

$(STATEDIR)/qt4-guf-demo.compile:
	@$(call targetinfo)
	cd $(QT4_GUF_DEMO_DIR) && $(QT4_GUF_DEMO_PATH) $(MAKE) $(PARALLELMFLAGS)
	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

$(STATEDIR)/qt4-guf-demo.install:
	@$(call targetinfo)
	@$(call install, QT4_GUF_DEMO)
	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/qt4-guf-demo.targetinstall:
	@$(call targetinfo)

	@$(call install_init, qt4-guf-demo)
	@$(call install_fixup, qt4-guf-demo,PACKAGE,qt4-guf-demo)
	@$(call install_fixup, qt4-guf-demo,PRIORITY,optional)
	@$(call install_fixup, qt4-guf-demo,VERSION,$(QT4_GUF_DEMO_VERSION))
	@$(call install_fixup, qt4-guf-demo,SECTION,base)
	@$(call install_fixup, qt4-guf-demo,AUTHOR,"Carsten Behling")
	@$(call install_fixup, qt4-guf-demo,DEPENDS,)
	@$(call install_fixup, qt4-guf-demo,DESCRIPTION,missing)

	#
	# Application
	#
	@$(call install_copy, qt4-guf-demo, 0, 0, 0755, \
		$(QT4_GUF_DEMO_DIR)/qt4-guf-demo, \
		/usr/bin/qt4-guf-demo)

	#
	# Images
	#
ifeq ($(PTXCONF_PLATFORM), CUPID)
	@cd $(QT4_GUF_DEMO_DIR)/images/cupid; \
	for file in `find gui57 -type f `; do \
		PER=`stat -c "%a" $$file` \
		$(call install_copy, qt4-guf-demo, 0, 0, $$PER, \
			$(QT4_GUF_DEMO_DIR)/images/cupid/$$file, \
			/usr/share/qt4-guf-demo/images/$$file) \
	done
	@cd $(QT4_GUF_DEMO_DIR)/images/cupid; \
	for file in `find gui70 -type f `; do \
		PER=`stat -c "%a" $$file` \
		$(call install_copy, qt4-guf-demo, 0, 0, $$PER, \
			$(QT4_GUF_DEMO_DIR)/images/cupid/$$file, \
			/usr/share/qt4-guf-demo/images/$$file) \
	done
endif
ifeq ($(PTXCONF_PLATFORM), NESO)
	@cd $(QT4_GUF_DEMO_DIR)/images/neso; \
	for file in `find gui57 -type f `; do \
		PER=`stat -c "%a" $$file` \
		$(call install_copy, qt4-guf-demo, 0, 0, $$PER, \
			$(QT4_GUF_DEMO_DIR)/images/neso/$$file, \
			/usr/share/qt4-guf-demo/images/$$file) \
	done
	@cd $(QT4_GUF_DEMO_DIR)/images/neso; \
	for file in `find gui70 -type f `; do \
		PER=`stat -c "%a" $$file` \
		$(call install_copy, qt4-guf-demo, 0, 0, $$PER, \
			$(QT4_GUF_DEMO_DIR)/images/neso/$$file, \
			/usr/share/qt4-guf-demo/images/$$file) \
	done
endif
ifeq ($(PTXCONF_PLATFORM), NESO-LT)
	@cd $(QT4_GUF_DEMO_DIR)/images/nesolt; \
	for file in `find gui43 -type f `; do \
		PER=`stat -c "%a" $$file` \
		$(call install_copy, qt4-guf-demo, 0, 0, $$PER, \
			$(QT4_GUF_DEMO_DIR)/images/nesolt/$$file, \
			/usr/share/qt4-guf-demo/images/$$file) \
	done
endif
ifeq ($(PTXCONF_PLATFORM), VINCELL)
	@cd $(QT4_GUF_DEMO_DIR)/images/vincell; \
	for file in `find gui57 -type f `; do \
		PER=`stat -c "%a" $$file` \
		$(call install_copy, qt4-guf-demo, 0, 0, $$PER, \
			$(QT4_GUF_DEMO_DIR)/images/vincell/$$file, \
			/usr/share/qt4-guf-demo/images/$$file) \
	done
	@cd $(QT4_GUF_DEMO_DIR)/images/vincell; \
	for file in `find gui70 -type f `; do \
		PER=`stat -c "%a" $$file` \
		$(call install_copy, qt4-guf-demo, 0, 0, $$PER, \
			$(QT4_GUF_DEMO_DIR)/images/vincell/$$file, \
			/usr/share/qt4-guf-demo/images/$$file) \
	done
	@cd $(QT4_GUF_DEMO_DIR)/images/vincell; \
	for file in `find gui80 -type f `; do \
		PER=`stat -c "%a" $$file` \
		$(call install_copy, qt4-guf-demo, 0, 0, $$PER, \
			$(QT4_GUF_DEMO_DIR)/images/vincell/$$file, \
			/usr/share/qt4-guf-demo/images/$$file) \
	done
	@cd $(QT4_GUF_DEMO_DIR)/images/vincell; \
	for file in `find gui84 -type f `; do \
		PER=`stat -c "%a" $$file` \
		$(call install_copy, qt4-guf-demo, 0, 0, $$PER, \
			$(QT4_GUF_DEMO_DIR)/images/vincell/$$file, \
			/usr/share/qt4-guf-demo/images/$$file) \
	done
endif
ifeq ($(PTXCONF_PLATFORM), ECO-G45)
	@cd $(QT4_GUF_DEMO_DIR)/images/eco-g45; \
	for file in `find gui43 -type f `; do \
		PER=`stat -c "%a" $$file` \
		$(call install_copy, qt4-guf-demo, 0, 0, $$PER, \
			$(QT4_GUF_DEMO_DIR)/images/eco-g45/$$file, \
			/usr/share/qt4-guf-demo/images/$$file) \
	done
endif

	#
	# Startup script
	#
	@$(call install_alternative, qt4-guf-demo, 0, 0, 0755, \
		/etc/init.d/qt4-guf-demo)
	@$(call install_link, qt4-guf-demo, ../init.d/qt4-guf-demo, \
		/etc/rc.d/S95qt4-guf-demo)
		
	
	@$(call install_finish, qt4-guf-demo)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

#$(STATEDIR)/qt4-guf-demo.clean:
#	@$(call targetinfo)
#	@$(call clean_pkg, QT4_GUF_DEMO)

# vim: syntax=make
