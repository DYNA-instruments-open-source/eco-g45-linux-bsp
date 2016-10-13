# -*-makefile-*-
#
# Copyright (C) 2010 by Tim Jaacks <tim.jaacks@garz-fricke.com>
#
# See CREDITS for details about who has contributed to this project.
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_LTP_GUF_TESTS) += ltp_guf_tests

#
# Paths and names
#
LTP_GUF_TESTS_VERSION	:= trunk
LTP_GUF_TESTS			:= ltp_guf_tests
LTP_GUF_TESTS_SUFFIX	:= 
LTP_GUF_TESTS_URL		:= file://$(PTXDIST_WORKSPACE)/local_src/common/$(LTP_GUF_TESTS)
LTP_GUF_TESTS_SRCDIR	:= $(PTXDIST_WORKSPACE)/local_src/common/$(LTP_GUF_TESTS)
LTP_GUF_TESTS_DIR		:= $(BUILDDIR)/$(LTP_GUF_TESTS)
LTP_GUF_TESTS_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

$(STATEDIR)/ltp_guf_tests.extract:
	@$(call targetinfo)
	@$(call clean, $(LTP_GUF_TESTS_DIR))
	@rm -Rf $(LTP_GUF_TESTS_DIR)
	@cp -R $(LTP_GUF_TESTS_SRCDIR) $(LTP_GUF_TESTS_DIR)
	@$(call patchin, LTP_GUF_TESTS)
	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

$(STATEDIR)/ltp_guf_tests.prepare:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

$(STATEDIR)/ltp_guf_tests.compile:
	@$(call targetinfo)
ifdef PTXCONF_LTP_GUF_TESTS_STATIC_LINKING
	@for dir in `find $(LTP_GUF_TESTS_DIR)/testcases/* -maxdepth 0 ! -name "bin" -type d|grep -v .svn`; do \
		cd $$dir && PATH=$(CROSS_PATH) $(CROSS_ENV) CFLAGS+='-I$(KERNEL_HEADERS_DIR)/include -static' LDFLAGS+='-static' $(MAKE) NEON=$(PTXCONF_ARCH_ARM_V6) || exit 1 ; \
	done
else
	@for dir in `find $(LTP_GUF_TESTS_DIR)/testcases/* -maxdepth 0 ! -name "bin" -type d|grep -v .svn`; do \
		cd $$dir && PATH=$(CROSS_PATH) $(CROSS_ENV) CFLAGS+='-I$(KERNEL_HEADERS_DIR)/include' $(MAKE) NEON=$(PTXCONF_ARCH_ARM_V6) || exit 1 ; \
	done
endif
	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

$(STATEDIR)/ltp_guf_tests.install:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/ltp_guf_tests.targetinstall:
	@$(call targetinfo)

	@$(call install_init,  ltp_guf_tests)
	@$(call install_fixup, ltp_guf_tests,PACKAGE,ltp-guf-tests)
	@$(call install_fixup, ltp_guf_tests,PRIORITY,optional)
	@$(call install_fixup, ltp_guf_tests,VERSION,$(LTP_GUF_TESTS_VERSION))
	@$(call install_fixup, ltp_guf_tests,SECTION,base)
	@$(call install_fixup, ltp_guf_tests,AUTHOR,"Tim Jaacks <tim.jaacks@garz-fricke.com>")
	@$(call install_fixup, ltp_guf_tests,DEPENDS,)
	@$(call install_fixup, ltp_guf_tests,DESCRIPTION,missing)

	@cd $(LTP_GUF_TESTS_DIR)/testcases/bin; \
	for file in `find * -type f`; do \
		$(call install_copy, ltp_guf_tests, 0, 0, 775, \
			$(LTP_GUF_TESTS_DIR)/testcases/bin/$$file, \
			/opt/ltp/testcases/bin/$$file) \
	done

	@cd $(LTP_GUF_TESTS_DIR); \
	for file in `find runtest -type f|grep -v .svn`; do \
		PER=`stat -c "%a" $$file` \
		$(call install_copy, ltp_guf_tests, 0, 0, $$PER, \
			$(LTP_GUF_TESTS_DIR)/$$file, \
			/opt/ltp/$$file) \
	done

	#
	# Startup script for camera
	#
	@$(call install_alternative, ltp_guf_tests, 0, 0, 0755, \
		/etc/init.d/camera)

	@$(call install_finish, ltp_guf_tests)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

#$(STATEDIR)/ltp_guf_tests.clean:
#	@$(call targetinfo)
#	@$(call clean_pkg, LTP_GUF_TESTS)

# vim: syntax=make
