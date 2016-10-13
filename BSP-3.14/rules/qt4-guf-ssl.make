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
PACKAGES-$(PTXCONF_QT4_GUF_SSL) += qt4-guf-ssl

#
# Paths and names
#
QT4_GUF_SSL_VERSION	:= trunk
QT4_GUF_SSL			:= qt4-guf-ssl
QT4_GUF_SSL_SUFFIX		:=
QT4_GUF_SSL_SRCDIR		:= $(PTXDIST_WORKSPACE)/local_src/common/$(QT4_GUF_SSL)
QT4_GUF_SSL_DIR		:= $(BUILDDIR)/$(QT4_GUF_SSL)

# ----------------------------------------------------------------------------
# Get
# ----------------------------------------------------------------------------

$(QT4_GUF_SSL_SOURCE):
	@$(call targetinfo)
	@$(call get, QT4_GUF_SSL)

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

$(STATEDIR)/qt4-guf-ssl.extract:
	@$(call targetinfo)
	@$(call clean, $(QT4_GUF_SSL_DIR))
#	@mkdir -p $(QT4_GUF_SSL_DIR)
	@rm -Rf $(QT4_GUF_SSL_DIR)
	@cp -R $(QT4_GUF_SSL_SRCDIR) $(QT4_GUF_SSL_DIR)
	@$(call patchin, QT4_GUF_SSL)
	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

QT4_GUF_SSL_PATH	:= PATH=$(CROSS_PATH)

QT4_GUF_SSL_ENV = \
	$(CROSS_ENV) \
	QMAKEPATH=$(QT4_DIR) \
	INSTALL_ROOT=$(SYSROOT) \
	QMAKESPEC=$(QT4_DIR)/mkspecs/qws/linux-ptx-g++

#
# autoconf
#
QT4_GUF_SSL_AUTOCONF := $(CROSS_AUTOCONF_USR)

$(STATEDIR)/qt4-guf-ssl.prepare:
	@$(call targetinfo)
	cd $(QT4_GUF_SSL_DIR) && \
		$(QT4_GUF_SSL_PATH) $(QT4_GUF_SSL_ENV) qmake qt4-guf-ssl.pro
	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

$(STATEDIR)/qt4-guf-ssl.compile:
	@$(call targetinfo)
	cd $(QT4_GUF_SSL_DIR) && $(QT4_GUF_SSL_PATH) $(MAKE) $(PARALLELMFLAGS)
	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

$(STATEDIR)/qt4-guf-ssl.install:
	@$(call targetinfo)
	@$(call install, QT4_GUF_SSL)
	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/qt4-guf-ssl.targetinstall:
	@$(call targetinfo)

	@$(call install_init, qt4-guf-ssl)
	@$(call install_fixup, qt4-guf-ssl,PACKAGE,qt4-guf-ssl)
	@$(call install_fixup, qt4-guf-ssl,PRIORITY,optional)
	@$(call install_fixup, qt4-guf-ssl,VERSION,$(QT4_GUF_SSL_VERSION))
	@$(call install_fixup, qt4-guf-ssl,SECTION,base)
	@$(call install_fixup, qt4-guf-ssl,AUTHOR,"Carsten Behling")
	@$(call install_fixup, qt4-guf-ssl,DEPENDS,)
	@$(call install_fixup, qt4-guf-ssl,DESCRIPTION,missing)

	#
	# Application
	#
	@$(call install_copy, qt4-guf-ssl, 0, 0, 0755, \
		$(QT4_GUF_SSL_DIR)/qt4-guf-ssl, \
		/usr/bin/qt4-guf-ssl)

	#
	# CA certificates
	#
	@$(call install_copy, qt4-guf-ssl, 0, 0, 0644, \
		$(QT4_GUF_SSL_DIR)/VeriSignClass3PublicPrimaryCertificationAuthority-G5.crt, \
		/usr/lib/ssl/certs/VeriSignClass3PublicPrimaryCertificationAuthority-G5.crt)
	@$(call install_copy, qt4-guf-ssl, 0, 0, 0644, \
		$(QT4_GUF_SSL_DIR)/VeriSignClass3ExtendedValidationSSLCA.crt, \
		/usr/lib/ssl/certs/VeriSignClass3ExtendedValidationSSLCA.crt)
	
	@$(call install_finish, qt4-guf-ssl)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

#$(STATEDIR)/qt4-guf-ssl.clean:
#	@$(call targetinfo)
#	@$(call clean_pkg, QT4_GUF_SSL)

# vim: syntax=make
