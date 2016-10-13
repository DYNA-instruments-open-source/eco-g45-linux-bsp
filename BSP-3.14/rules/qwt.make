# -*-makefile-*-
#
# Copyright (C) 2009,2011 by Michael Olbrich <m.olbrich@pengutronix.de>
#
# See CREDITS for details about who has contributed to this project.
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_QWT) += qwt

#
# Paths and names
#
QWT_VERSION	:= 6.0.1
QWT_MD5		:= ace68558eab873e2da7e641179c4ef0c
QWT		:= qwt-$(QWT_VERSION)
QWT_SUFFIX	:= tar.bz2
QWT_URL		:= $(PTXCONF_SETUP_SFMIRROR)/qwt/$(QWT).$(QWT_SUFFIX)
QWT_SOURCE	:= $(SRCDIR)/$(QWT).$(QWT_SUFFIX)
QWT_DIR		:= $(BUILDDIR)/$(QWT)
QWT_MAKE_PAR	:= NO

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

QWT_CONF_OPT	:= $(CROSS_QMAKE_OPT)


ifdef PTXCONF_QWT_SVG
QWT_CONF_OPT += QWT_CONFIG+=QwtSvg
endif
ifdef PTXCONF_QWT_EXAMPLES
QWT_CONF_OPT += QWT_CONFIG+=QwtExamples
endif

$(STATEDIR)/qwt.prepare:
	@$(call targetinfo)
	@rm -f $(SYSROOT)/usr/lib/libqwt.*
	@$(call world/prepare, QWT)
	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/qwt.targetinstall:
	@$(call targetinfo)

	@$(call install_init, qwt)
	@$(call install_fixup, qwt,PRIORITY,optional)
	@$(call install_fixup, qwt,SECTION,base)
	@$(call install_fixup, qwt,AUTHOR,"Michael Olbrich <m.olbrich@pengutronix.de>")
	@$(call install_fixup, qwt,DESCRIPTION,missing)

	@$(call install_lib, qwt, 0, 0, 0644, libqwt)

ifdef PTXCONF_QWT_EXAMPLES
	@$(call install_copy, qwt, 0, 0, 0755, \
		$(QWT_DIR)/examples/bin/bode, \
		/usr/bin/qwt-examples/bode)
	@$(call install_copy, qwt, 0, 0, 0755, \
		$(QWT_DIR)/examples/bin/cpuplot, \
		/usr/bin/qwt-examples/cpuplot)
	@$(call install_copy, qwt, 0, 0, 0755, \
		$(QWT_DIR)/examples/bin/curvdemo1, \
		/usr/bin/qwt-examples/curvdemo1)
	@$(call install_copy, qwt, 0, 0, 0755, \
		$(QWT_DIR)/examples/bin/curvdemo2, \
		/usr/bin/qwt-examples/curvdemo2)
	@$(call install_copy, qwt, 0, 0, 0755, \
		$(QWT_DIR)/examples/bin/dials, \
		/usr/bin/qwt-examples/dials)
	@$(call install_copy, qwt, 0, 0, 0755, \
		$(QWT_DIR)/examples/bin/event_filter, \
		/usr/bin/qwt-examples/event_filter)
	@$(call install_copy, qwt, 0, 0, 0755, \
		$(QWT_DIR)/examples/bin/friedberg, \
		/usr/bin/qwt-examples/friedberg)
	@$(call install_copy, qwt, 0, 0, 0755, \
		$(QWT_DIR)/examples/bin/navigation, \
		/usr/bin/qwt-examples/navigation)
	@$(call install_copy, qwt, 0, 0, 0755, \
		$(QWT_DIR)/examples/bin/oscilloscope, \
		/usr/bin/qwt-examples/oscilloscope)
	@$(call install_copy, qwt, 0, 0, 0755, \
		$(QWT_DIR)/examples/bin/plotmatrix, \
		/usr/bin/qwt-examples/plotmatrix)
	@$(call install_copy, qwt, 0, 0, 0755, \
		$(QWT_DIR)/examples/bin/radio, \
		/usr/bin/qwt-examples/radio)
	@$(call install_copy, qwt, 0, 0, 0755, \
		$(QWT_DIR)/examples/bin/rasterview, \
		/usr/bin/qwt-examples/rasterview)
	@$(call install_copy, qwt, 0, 0, 0755, \
		$(QWT_DIR)/examples/bin/realtime, \
		/usr/bin/qwt-examples/realtime)
	@$(call install_copy, qwt, 0, 0, 0755, \
		$(QWT_DIR)/examples/bin/refreshtest, \
		/usr/bin/qwt-examples/refreshtest)
	@$(call install_copy, qwt, 0, 0, 0755, \
		$(QWT_DIR)/examples/bin/sinusplot, \
		/usr/bin/qwt-examples/sinusplot)
	@$(call install_copy, qwt, 0, 0, 0755, \
		$(QWT_DIR)/examples/bin/sliders, \
		/usr/bin/qwt-examples/sliders)
	@$(call install_copy, qwt, 0, 0, 0755, \
		$(QWT_DIR)/examples/bin/spectrogram, \
		/usr/bin/qwt-examples/spectrogram)
	@$(call install_copy, qwt, 0, 0, 0755, \
		$(QWT_DIR)/examples/bin/svgmap, \
		/usr/bin/qwt-examples/svgmap)
	@$(call install_copy, qwt, 0, 0, 0755, \
		$(QWT_DIR)/examples/bin/sysinfo, \
		/usr/bin/qwt-examples/sysinfo)
	@$(call install_copy, qwt, 0, 0, 0755, \
		$(QWT_DIR)/examples/bin/tvplot, \
		/usr/bin/qwt-examples/tvplot)

endif

	@$(call install_finish, qwt)

	@$(call touch)

# vim: syntax=make
