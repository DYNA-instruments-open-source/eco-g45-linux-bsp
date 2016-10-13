# -*-makefile-*-
#
# Copyright (C) 2010 by Carsten Behling <carsten.behling@garz-fricke.com>
#
# See CREDITS for details about who has contributed to this project.
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_LTP) += ltp

#
# Paths and names
#
LTP_VERSION	:= full-20110915
LTP_MD5		:= 582fb78d7bf78a624a4387f29327d166
LTP			:= ltp-$(LTP_VERSION)
LTP_SUFFIX	:= bz2
LTP_URL		:= http://sourceforge.net/projects/ltp/files/$(LTP).$(LTP_SUFFIX)
LTP_SOURCE	:= $(SRCDIR)/$(LTP).$(LTP_SUFFIX)
LTP_DIR		:= $(BUILDDIR)/$(LTP)
LTP_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Get
# ----------------------------------------------------------------------------

$(LTP_SOURCE):
	@$(call targetinfo)
	@$(call get, LTP)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#LTP_CONF_ENV	:= $(CROSS_ENV)

#
# autoconf
#
LTP_CONF_TOOL	:= autoconf
LTP_CONF_OPT	:= $(CROSS_AUTOCONF_USR) \
                   --disable-dependency-tracking
LTP_ENV += SKIP_IDCHECK=1

$(STATEDIR)/ltp.prepare:
	@$(call targetinfo)
	@$(call clean, $(LTP_DIR)/config.cache)
	cd $(LTP_DIR) && \
		$(LTP_PATH) $(LTP_ENV) \
		./configure $(LTP_CONF_OPT)
	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

$(STATEDIR)/ltp.compile:
	@$(call targetinfo)
ifdef PTXCONF_LTP_STATIC_LINKING
	@cd $(LTP_DIR) && $(MAKE) WLDFLAGS='-static'
else
	@cd $(LTP_DIR) && $(MAKE) LTP_TESTCASES=$(PTXCONF_LTP_TESTCASES)
endif
	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

$(STATEDIR)/ltp.install:
	@$(call targetinfo)
	@cd $(LTP_DIR) && $(MAKE) install LTP_TESTCASES=$(PTXCONF_LTP_TESTCASES) DESTDIR=$(SYSROOT)
	@cp $(LTP_DIR)/lib/libltp.a $(SYSROOT)/usr/lib/libltp.a
	@cp -R $(LTP_DIR)/include/* $(SYSROOT)/usr/include
	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/ltp.targetinstall:
	@$(call targetinfo)

	@$(call install_init,  ltp)
	@$(call install_fixup, ltp,PACKAGE,ltp)
	@$(call install_fixup, ltp,PRIORITY,optional)
	@$(call install_fixup, ltp,VERSION,$(LTP_VERSION))
	@$(call install_fixup, ltp,SECTION,base)
	@$(call install_fixup, ltp,AUTHOR,"Carsten Behling <carsten.behling@garz-fricke.com>")
	@$(call install_fixup, ltp,DEPENDS,)
	@$(call install_fixup, ltp,DESCRIPTION,missing)

	@$(call install_copy, ltp, 0, 0, 0755, $(SYSROOT)/usr/runltp, /opt/ltp/runltp)
	@$(call install_copy, ltp, 0, 0, 0755, $(SYSROOT)/usr/bin/ltp-pan, /opt/ltp/bin/ltp-pan)
	@$(call install_copy, ltp, 0, 0, 0755, $(SYSROOT)/usr/Version, /opt/ltp/Version)
	@$(call install_copy, ltp, 0, 0, 0755, $(SYSROOT)/usr/ver_linux, /opt/ltp/ver_linux)

	@cd $(SYSROOT)/usr; \
	for file in `find testcases -type f -name tst_*`; do \
		PER=`stat -c "%a" $$file` \
		$(call install_copy, ltp, 0, 0, $$PER, \
			$(SYSROOT)/usr/$$file, \
			/opt/ltp/$$file) \
	done

ifdef PTXCONF_LTP_TESTCASES
	@cd $(SYSROOT)/usr; \
	for file in `find testcases -type f ! -name tst_*`; do \
		PER=`stat -c "%a" $$file` \
		$(call install_copy, ltp, 0, 0, $$PER, \
			$(SYSROOT)/usr/$$file, \
			/opt/ltp/$$file) \
	done

	@cd $(SYSROOT)/usr; \
	for file in `find runtest -type f`; do \
		PER=`stat -c "%a" $$file` \
		$(call install_copy, ltp, 0, 0, $$PER, \
			$(SYSROOT)/usr/$$file, \
			/opt/ltp/$$file) \
	done
endif

	@$(call install_finish, ltp)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

#$(STATEDIR)/ltp.clean:
#	@$(call targetinfo)
#	@$(call clean_pkg, LTP)

# vim: syntax=make
