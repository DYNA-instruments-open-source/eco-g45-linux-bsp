# -*-makefile-*-
#
# Copyright (C) 2012 by Carsten Behling <carsten.behling@garz-fricke.com>
#
# See CREDITS for details about who has contributed to this project.
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_IMX_TEST) += imx-test

#
# Paths and names
#
IMX_TEST_VERSION	:= 11.09.01
IMX_TEST			:= imx-test-$(IMX_TEST_VERSION)
IMX_TEST_SUFFIX		:= 
IMX_TEST_URL		:= file://$(PTXDIST_WORKSPACE)/local_src/common/$(IMX_TEST)
IMX_TEST_SRCDIR		:= $(PTXDIST_WORKSPACE)/local_src/common/$(IMX_TEST)
IMX_TEST_DIR		:= $(BUILDDIR)/$(IMX_TEST)
IMX_TEST_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

$(STATEDIR)/imx-test.extract:
	@$(call targetinfo)
	@$(call clean, $(IMX_TEST_DIR))
	@rm -Rf $(IMX_TEST_DIR)
	@cp -R $(IMX_TEST_SRCDIR) $(IMX_TEST_DIR)
	@$(call patchin, IMX_TEST)
	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

$(STATEDIR)/imx-test.prepare:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

$(STATEDIR)/imx-test.compile:
	@$(call targetinfo)
	@cd $(IMX_TEST_DIR) && PLATFORM=$(PTXCONF_IMX_TEST_PLATFORM) \
		CROSS_COMPILE=$(KERNEL_CROSS_COMPILE) LINUXPATH=$(KERNEL_DIR) \
		LDFLAGS='-L$(SYSROOT)/usr/lib' \
		INCLUDE='-I$(SYSROOT)/usr/include -I$(KERNEL_HEADERS_DIR)/include' \
		$(MAKE) all
	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

$(STATEDIR)/imx-test.install:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/imx-test.targetinstall:
	@$(call targetinfo)

	@$(call install_init, imx-test)
	@$(call install_fixup, imx-test,PRIORITY,optional)
	@$(call install_fixup, imx-test,SECTION,base)
	@$(call install_fixup, imx-test,AUTHOR,"Carsten Behling <carsten.behling@garz-fricke.com>")
	@$(call install_fixup, imx-test,DESCRIPTION,missing)

	# VPU test
	@$(call install_copy, imx-test, 0, 0, 0755, \
		$(IMX_TEST_DIR)/platform/$(PTXCONF_IMX_TEST_PLATFORM)/mxc_vpu_test.out, \
		/opt/imx-test/mxc_vpu_test/mxc_vpu_test.out)
	@$(call install_copy, imx-test, 0, 0, 0755, \
		$(IMX_TEST_DIR)/platform/$(PTXCONF_IMX_TEST_PLATFORM)/akiyo.mp4, \
		/opt/imx-test/mxc_vpu_test/akiyo.mp4)
	@$(call install_copy, imx-test, 0, 0, 0755, \
		$(IMX_TEST_DIR)/platform/$(PTXCONF_IMX_TEST_PLATFORM)/autorun-vpu.sh, \
		/opt/imx-test/mxc_vpu_test/autorun-vpu.sh)
	@$(call install_copy, imx-test, 0, 0, 0644, \
		$(IMX_TEST_DIR)/platform/$(PTXCONF_IMX_TEST_PLATFORM)/config_dec, \
		/opt/imx-test/mxc_vpu_test/config_dec)
	@$(call install_copy, imx-test, 0, 0, 0644, \
		$(IMX_TEST_DIR)/platform/$(PTXCONF_IMX_TEST_PLATFORM)/config_enc, \
		/opt/imx-test/mxc_vpu_test/config_enc)
	@$(call install_copy, imx-test, 0, 0, 0644, \
		$(IMX_TEST_DIR)/platform/$(PTXCONF_IMX_TEST_PLATFORM)/config_encdec, \
		/opt/imx-test/mxc_vpu_test/config_encdec)
	@$(call install_copy, imx-test, 0, 0, 0644, \
		$(IMX_TEST_DIR)/platform/$(PTXCONF_IMX_TEST_PLATFORM)/config_net, \
		/opt/imx-test/mxc_vpu_test/config_net)
		
	# V4L2 test
	@$(call install_copy, imx-test, 0, 0, 0755, \
		$(IMX_TEST_DIR)/platform/$(PTXCONF_IMX_TEST_PLATFORM)/mxc_v4l2_output.out, \
		/opt/imx-test/mxc_v4l2_test/mxc_v4l2_output.out)
	@$(call install_copy, imx-test, 0, 0, 0755, \
		$(IMX_TEST_DIR)/platform/$(PTXCONF_IMX_TEST_PLATFORM)/mxc_v4l2_still.out, \
		/opt/imx-test/mxc_v4l2_test/mxc_v4l2_still.out)
	@$(call install_copy, imx-test, 0, 0, 0755, \
		$(IMX_TEST_DIR)/platform/$(PTXCONF_IMX_TEST_PLATFORM)/mxc_v4l2_capture.out, \
		/opt/imx-test/mxc_v4l2_test/mxc_v4l2_capture.out)
	@$(call install_copy, imx-test, 0, 0, 0755, \
		$(IMX_TEST_DIR)/platform/$(PTXCONF_IMX_TEST_PLATFORM)/mxc_v4l2_overlay.out, \
		/opt/imx-test/mxc_v4l2_test/mxc_v4l2_overlay.out)
	@$(call install_copy, imx-test, 0, 0, 0755, \
		$(IMX_TEST_DIR)/platform/$(PTXCONF_IMX_TEST_PLATFORM)/mxc_v4l2_tvin.out, \
		/opt/imx-test/mxc_v4l2_test/mxc_v4l2_tvin.out)
	@$(call install_copy, imx-test, 0, 0, 0755, \
		$(IMX_TEST_DIR)/platform/$(PTXCONF_IMX_TEST_PLATFORM)/autorun-v4l2.sh, \
		/opt/imx-test/mxc_v4l2_test/autorun-v4l2.sh)

	@$(call install_finish, imx-test)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

#$(STATEDIR)/imx-test.clean:
#	@$(call targetinfo)
#	@$(call clean_pkg, IMX_TEST)

# vim: syntax=make
