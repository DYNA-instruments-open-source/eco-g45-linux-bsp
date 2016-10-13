# -*-makefile-*-
#
# Copyright (C) 2009 by Carsten Behling
#
# See CREDITS for details about who has contributed to this project.
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_MICROWINDOWS) += microwindows

#
# Paths and names
#
MICROWINDOWS_VERSION	:= 0.91-20090128
MICROWINDOWS		:= microwindows-$(MICROWINDOWS_VERSION)
MICROWINDOWS_SUFFIX	:= tar.gz
MICROWINDOWS_URL	:= http://download.armbedded.eu/software/$(MICROWINDOWS).$(MICROWINDOWS_SUFFIX)
MICROWINDOWS_SOURCE	:= $(SRCDIR)/$(MICROWINDOWS).$(MICROWINDOWS_SUFFIX)
MICROWINDOWS_DIR	:= $(BUILDDIR)/$(MICROWINDOWS)

# ----------------------------------------------------------------------------
# Get
# ----------------------------------------------------------------------------

microwindows_get: $(STATEDIR)/microwindows.get

$(STATEDIR)/microwindows.get:
	@$(call targetinfo, $@)
	@$(call touch, $@)

$(MICROWINDOWS_SOURCE):
	@$(call targetinfo, $@)
	@$(call get, MICROWINDOWS)


# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

microwindows_prepare: $(STATEDIR)/microwindows.prepare

# With the introduction of platformconfigs PTXCONF_ARCH was
# renamed to PTXCONF_ARCH_STRING.
ifdef PTXCONF_ARCH
MICROWINDOWS_ARCH = $(call remove_quotes, $(PTXCONF_ARCH))
else
MICROWINDOWS_ARCH = $(call remove_quotes, $(PTXCONF_ARCH_STRING))
endif

$(STATEDIR)/microwindows.prepare:
	@$(call targetinfo)

# TODO:
#
# Add targets for SH, MIPS ...

	@if [ $(MICROWINDOWS_ARCH) = arm ]; then \
		sed -i~ -e "s;@PTXARCH@;LINUX-ARM;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx; \
		sed -i~ -e "s;@PTXBIGENDIAN@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx; \
		sed -i~ -e "s;@PTXARMTOOLSPREFIX@;$(COMPILER_PREFIX);" $(MICROWINDOWS_DIR)/src/Configs/config.ptx; \
		sed -i~ -e "s;@PTXMIPSTOOLSPREFIX@;;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx; \
		sed -i~ -e "s;@PTXPOWERPCTOOLSPREFIX@;;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx; \
		sed -i~ -e "s;@PTXSHTOOLSPREFIX@;;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx; \
		sed -i~ -e "s;@PTXRTEMSTOOLSPREFIX@;;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx; \
	elif [ $(MICROWINDOWS_ARCH) = powerpc ]; then \
		sed -i~ -e "s;@PTXARCH@;LINUX-POWERPC;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx; \
		sed -i~ -e "s;@PTXBIGENDIAN@;Y;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx; \
		sed -i~ -e "s;@PTXPOWERPCTOOLSPREFIX@;$(COMPILER_PREFIX);" $(MICROWINDOWS_DIR)/src/Configs/config.ptx; \
		sed -i~ -e "s;@PTXMIPSTOOLSPREFIX@;;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx; \
		sed -i~ -e "s;@PTXARMTOOLSPREFIX@;;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx; \
		sed -i~ -e "s;@PTXSHTOOLSPREFIX@;;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx; \
		sed -i~ -e "s;@PTXRTEMSTOOLSPREFIX@;;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx; \
	fi

ifdef PTXCONF_MICROWINDOWS_OPTIMIZE
	@sed -i~ -e "s;@PTXOPTIMIZE@;Y;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
else
	@sed -i~ -e "s;@PTXOPTIMIZE@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
endif

ifdef PTXCONF_MICROWINDOWS_DEBUG
	@sed -i~ -e "s;@PTXDEBUG@;Y;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
else
	@sed -i~ -e "s;@PTXDEBUG@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
endif

ifdef PTXCONF_MICROWINDOWS_VERBOSE
	@sed -i~ -e "s;@PTXVERBOSE@;Y;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
else
	@sed -i~ -e "s;@PTXVERBOSE@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
endif

ifdef PTXCONF_MICROWINDOWS_THREADSAFE
	@sed -i~ -e "s;@PTXTHREADSAFE@;Y;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
else
	@sed -i~ -e "s;@PTXTHREADSAFE@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
endif

ifdef PTXCONF_MICROWINDOWS_MICROWIN
	@sed -i~ -e "s;@PTXMICROWIN@;Y;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
else
	@sed -i~ -e "s;@PTXMICROWIN@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
endif

ifdef PTXCONF_MICROWINDOWS_NANOX
	@sed -i~ -e "s;@PTXNANOX@;Y;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
else
	@sed -i~ -e "s;@PTXNANOX@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
endif

ifdef PTXCONF_MICROWINDOWS_NWIDGET
	@sed -i~ -e "s;@PTXNWIDGET@;Y;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
else
	@sed -i~ -e "s;@PTXNWIDGET@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
endif

ifdef PTXCONF_MICROWINDOWS_OBJECTFRAMEWORK
	@sed -i~ -e "s;@PTXOBJFRAMEWORK@;Y;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
else
	@sed -i~ -e "s;@PTXOBJFRAMEWORK@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
endif

ifdef PTXCONF_MICROWINDOWS_BMP
	@sed -i~ -e "s;@PTXHAVEBMPSUPPORT@;Y;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
else
	@sed -i~ -e "s;@PTXHAVEBMPSUPPORT@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
endif

ifdef PTXCONF_MICROWINDOWS_GIF
	@sed -i~ -e "s;@PTXHAVEGIFSUPPORT@;Y;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
else
	@sed -i~ -e "s;@PTXHAVEGIFSUPPORT@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
endif

ifdef PTXCONF_MICROWINDOWS_PNM
	@sed -i~ -e "s;@PTXHAVEPNMSUPPORT@;Y;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
else
	@sed -i~ -e "s;@PTXHAVEPNMSUPPORT@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
endif

ifdef PTXCONF_MICROWINDOWS_XPM
	@sed -i~ -e "s;@PTXHAVEXPMSUPPORT@;Y;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
else
	@sed -i~ -e "s;@PTXHAVEXPMSUPPORT@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
endif

ifdef PTXCONF_MICROWINDOWS_JPEG
	@sed -i~ -e "s;@PTXHAVEJPEGSUPPORT@;Y;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
	@sed -i~ -e "s;@PTXLIBJPEG@;$(SYSROOT)/usr/lib/libjpeg.so;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
	@sed -i~ -e "s;@PTXINCJPEG@;$(SYSROOT)/usr/include;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
else
	@sed -i~ -e "s;@PTXHAVEJPEGSUPPORT@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
	@sed -i~ -e "s;@PTXLIBJPEG@;;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
	@sed -i~ -e "s;@PTXINCJPEG@;;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
endif

ifdef PTXCONF_MICROWINDOWS_PNG
	@sed -i~ -e "s;@PTXHAVEPNGSUPPORT@;Y;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
	@sed -i~ -e "s;@PTXLIBPNG@;$(SYSROOT)/usr/lib/libpng.so;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
	@sed -i~ -e "s;@PTXLIBZ@;$(SYSROOT)/usr/lib/libz.so;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
	@sed -i~ -e "s;@PTXINCPNG@;$(SYSROOT)/usr/include;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
else
	@sed -i~ -e "s;@PTXHAVEPNGSUPPORT@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
	@sed -i~ -e "s;@PTXLIBPNG@;;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
	@sed -i~ -e "s;@PTXINCPNG@;;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
	@sed -i~ -e "s;@PTXLIBZ@;;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
endif

#
# currently not supported
#
#ifdef PTXCONF_MICROWINDOWS_TIFF
#	@sed -i~ -e "s;@PTXHAVETIFFSUPPORT@;Y;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
#	@sed -i~ -e "s;@PTXLIBTIFF@;$(SYSROOT)/usr/lib/libtiff.so;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
#	@sed -i~ -e "s;@PTXINCTIFF@;$(SYSROOT)/usr/include;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
#else
	@sed -i~ -e "s;@PTXHAVETIFFSUPPORT@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
	@sed -i~ -e "s;@PTXLIBTIFF@;;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
	@sed -i~ -e "s;@PTXINCTIFF@;;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
#endif

ifdef PTXCONF_MICROWINDOWS_FONT_NATIVE
	@sed -i~ -e "s;@PTXHAVEFONTSUPPORT@;Y;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
ifdef PTXCONF_MICROWINDOWS_FONT_NATIVE_GZIP
	@sed -i~ -e "s;@PTXHAVEFONTGZSUPPORT@;Y;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
else
	@sed -i~ -e "s;@PTXHAVEFONTGZSUPPORT@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
endif
	@sed -i~ -e "s;@PTXFONTDIR@;\"$(PTXCONF_MICROWINDOWS_FONT_NATIVE_DIRECTORY)\";" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
	@sed -i~ -e "s;@PTXNOFONTSORCLIPPING@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
else
	@sed -i~ -e "s;@PTXHAVEFONTSUPPORT@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
	@sed -i~ -e "s;@PTXHAVEFONTGZSUPPORT@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
	@sed -i~ -e "s;@PTXFONTDIR@;;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
endif

#
# currently not supported
#
#ifdef PTXCONF_MICROWINDOWS_FONT_T1
#	@sed -i~ -e "s;@PTXHAVET1LIBSUPPORT@;Y;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
#	@sed -i~ -e "s;@PTXLIBT1LIB@;$(SYSROOT)/usr/lib/libt1.so;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
#	@sed -i~ -e "s;@PTXINCT1LIB@;$(SYSROOT)/usr/include;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
#	@sed -i~ -e "s;@PTXNOFONTSORCLIPPING@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
#else
	@sed -i~ -e "s;@PTXHAVET1LIBSUPPORT@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
	@sed -i~ -e "s;@PTXLIBT1LIB@;;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
	@sed -i~ -e "s;@PTXINCT1LIB@;;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
#endif

#
# currently not supported
#
#ifdef PTXCONF_MICROWINDOWS_FONT_FREETYPE
#	@sed -i~ -e "s;@PTXHAVEFREETYPESUPPORT@;Y;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
#	@sed -i~ -e "s;@PTXLIBFREETYPE@;$(SYSROOT)/usr/lib/libttf.so;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
#	@sed -i~ -e "s;@PTXINCFREETYPE@;$(SYSROOT)/usr/include;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
#	@sed -i~ -e "s;@PTXFONTDIRFREETYPE@;\"$(PTXCONF_MICROWINDOWS_FONT_FREETYPE_DIRECTORY)\";" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
#	@sed -i~ -e "s;@PTXNOFONTSORCLIPPING@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
#else
	@sed -i~ -e "s;@PTXHAVEFREETYPESUPPORT@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
	@sed -i~ -e "s;@PTXLIBFREETYPE@;;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
	@sed -i~ -e "s;@PTXINCFREETYPE@;;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
#	@sed -i~ -e "s;@PTXFONTDIRFREETYPE@;;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
#endif

ifdef PTXCONF_MICROWINDOWS_FONT_FREETYPE2
	@sed -i~ -e "s;@PTXHAVEFREETYPE2SUPPORT@;Y;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
	@sed -i~ -e "s;@PTXLIBFREETYPE2@;$(SYSROOT)/usr/lib/libfreetype.so;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
	@sed -i~ -e "s;@PTXINCFREETYPE2@;$(SYSROOT)/usr/include;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
	@sed -i~ -e "s;@PTXFONTDIRFREETYPE@;\"$(PTXCONF_MICROWINDOWS_FONT_FREETYPE2_DIRECTORY)\";" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
	@sed -i~ -e "s;@PTXNOFONTSORCLIPPING@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
else
	@sed -i~ -e "s;@PTXHAVEFREETYPE2SUPPORT@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
	@sed -i~ -e "s;@PTXLIBFREETYPE2@;;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
	@sed -i~ -e "s;@PTXINCFREETYPE2@;;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
	@sed -i~ -e "s;@PTXFONTDIRFREETYPE@;;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
endif

ifdef PTXCONF_MICROWINDOWS_FONT_PCF
	@sed -i~ -e "s;@PTXHAVEPCFSUPPORT@;Y;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
ifdef PTXCONF_MICROWINDOWS_FONT_PCF_GZIP
	@sed -i~ -e "s;@PTXHAVEPCFGZSUPPORT@;Y;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
else
	@sed -i~ -e "s;@PTXHAVEPCFGZSUPPORT@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
endif
	@sed -i~ -e "s;@PTXFONTDIRPCF@;\"$(PTXCONF_MICROWINDOWS_FONT_PCF_DIRECTORY)\";" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
	@sed -i~ -e "s;@PTXNOFONTSORCLIPPING@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
else
	@sed -i~ -e "s;@PTXHAVEPCFSUPPORT@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
	@sed -i~ -e "s;@PTXHAVEPCFGZSUPPORT@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
	@sed -i~ -e "s;@PTXFONTDIRPCF@;;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
endif

ifdef PTXCONF_MICROWINDOWS_FONT_HZK
	@sed -i~ -e "s;@PTXHAVEHZKSUPPORT@;Y;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
	@sed -i~ -e "s;@PTXFONTDIRHZK@;\"$(PTXCONF_MICROWINDOWS_FONT_HZK_DIRECTORY)\";" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
	@sed -i~ -e "s;@PTXNOFONTSORCLIPPING@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
else
	@sed -i~ -e "s;@PTXHAVEHZKSUPPORT@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
	@sed -i~ -e "s;@PTXFONTDIRHZK@;;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
endif

ifdef PTXCONF_MICROWINDOWS_FONT_BIG5
	@sed -i~ -e "s;@PTXHAVEBIG5SUPPORT@;Y;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
	@sed -i~ -e "s;@PTXNOFONTSORCLIPPING@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
else
	@sed -i~ -e "s;@PTXHAVEBIG5SUPPORT@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
endif

ifdef PTXCONF_MICROWINDOWS_FONT_GB2312
	@sed -i~ -e "s;@PTXHAVEGB2312SUPPORT@;Y;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
	@sed -i~ -e "s;@PTXNOFONTSORCLIPPING@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
else
	@sed -i~ -e "s;@PTXHAVEGB2312SUPPORT@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
endif

ifdef PTXCONF_MICROWINDOWS_FONT_JISX0213
	@sed -i~ -e "s;@PTXHAVEJISX0213SUPPORT@;Y;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
	@sed -i~ -e "s;@PTXNOFONTSORCLIPPING@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
else
	@sed -i~ -e "s;@PTXHAVEJISX0213SUPPORT@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
endif

ifdef PTXCONF_MICROWINDOWS_FONT_KSC5601
	@sed -i~ -e "s;@PTXHAVEKSC5601SUPPORT@;Y;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
	@sed -i~ -e "s;@PTXNOFONTSORCLIPPING@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
else
	@sed -i~ -e "s;@PTXHAVEKSC5601SUPPORT@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
endif

ifdef PTXCONF_MICROWINDOWS_FONT_EUCJP
	@sed -i~ -e "s;@PTXHAVEEUCJPSUPPORT@;Y;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
	@sed -i~ -e "s;@PTXFONTDIREUCJP@;\"$(PTXCONF_MICROWINDOWS_FONT_EUCJP_DIRECTORY)\";" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
	@sed -i~ -e "s;@PTXNOFONTSORCLIPPING@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
else
	@sed -i~ -e "s;@PTXHAVEEUCJPSUPPORT@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
	@sed -i~ -e "s;@PTXFONTDIREUCJP@;;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
endif

	@sed -i~ -e "s;@PTXNOFONTSORCLIPPING@;Y;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx

ifdef PTXCONF_MICROWINDOWS_SHARED
	@sed -i~ -e "s;@PTXSHARED@;Y;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
else
	@sed -i~ -e "s;@PTXSHARED@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
endif

ifdef PTXCONF_MICROWINDOWS_SHAREDMEM
	@sed -i~ -e "s;@PTXHAVESHAREDMEM@;Y;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
else
	@sed -i~ -e "s;@PTXHAVESHAREDMEM@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
endif

ifdef PTXCONF_MICROWINDOWS_FILEIO
	@sed -i~ -e "s;@PTXHAVEFILEIO@;Y;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
else
	@sed -i~ -e "s;@PTXHAVEFILEIO@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
endif

ifdef PTXCONF_MICROWINDOWS_ERASEMOVE
	@sed -i~ -e "s;@PTXERASEMOVE@;Y;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
else
	@sed -i~ -e "s;@PTXERASEMOVE@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
endif

ifdef PTXCONF_MICROWINDOWS_UPDATEREGIONS
	@sed -i~ -e "s;@PTXUPDATEREGIONS@;Y;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
else
	@sed -i~ -e "s;@PTXUPDATEREGIONS@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
endif

ifdef PTXCONF_MICROWINDOWS_MICROWINDEMO
	@sed -i~ -e "s;@PTXBUILDMWDEMO@;Y;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
else
	@sed -i~ -e "s;@PTXBUILDMWDEMO@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
endif

ifdef PTXCONF_MICROWINDOWS_NANOXDEMO
	@sed -i~ -e "s;@PTXBUILDNXDEMO@;Y;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
else
	@sed -i~ -e "s;@PTXBUILDNXDEMO@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
endif

ifdef PTXCONF_MICROWINDOWS_NANOWM
	@sed -i~ -e "s;@PTXBUILDNANOWM@;Y;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
else
	@sed -i~ -e "s;@PTXBUILDNANOWM@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
endif

ifdef PTXCONF_MICROWINDOWS_SCREENPIXELTYPE_PALETTE
	@sed -i~ -e "s;@PTXSCREENPIXTYPE@;MWPF_PALETTE;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
endif

ifdef PTXCONF_MICROWINDOWS_SCREENPIXELTYPE_TRUECOLOR8888
	@sed -i~ -e "s;@PTXSCREENPIXTYPE@;MWPF_TRUECOLOR8888;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
endif

ifdef PTXCONF_MICROWINDOWS_SCREENPIXELTYPE_TRUECOLOR0888
	@sed -i~ -e "s;@PTXSCREENPIXTYPE@;MWPF_TRUECOLOR0888;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
endif

ifdef PTXCONF_MICROWINDOWS_SCREENPIXELTYPE_TRUECOLOR888
	@sed -i~ -e "s;@PTXSCREENPIXTYPE@;MWPF_TRUECOLOR888;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
endif

ifdef PTXCONF_MICROWINDOWS_SCREENPIXELTYPE_TRUECOLOR565
	@sed -i~ -e "s;@PTXSCREENPIXTYPE@;MWPF_TRUECOLOR565;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
endif

ifdef PTXCONF_MICROWINDOWS_SCREENPIXELTYPE_TRUECOLOR555
	@sed -i~ -e "s;@PTXSCREENPIXTYPE@;MWPF_TRUECOLOR555;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
endif

ifdef PTXCONF_MICROWINDOWS_SCREENPIXELTYPE_TRUECOLOR332
	@sed -i~ -e "s;@PTXSCREENPIXTYPE@;MWPF_TRUECOLOR332;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
endif

	@sed -i~ -e "s;@PTXLINKAPPINTOSERVER@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
	@sed -i~ -e "s;@PTXGRAYPALETTE@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
	@sed -i~ -e "s;@PTXX11@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
	@sed -i~ -e "s;@PTXX11SCREENWITH@;;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
	@sed -i~ -e "s;@PTXX11SCREENHIGHT@;;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
	@sed -i~ -e "s;@PTXX11SCREENDEPTH@;;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
	@sed -i~ -e "s;@PTXX11USEEXPOSURE@;;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
	@sed -i~ -e "s;@PTXFRAMEBUFFER@;Y;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
	@sed -i~ -e "s;@PTXFRAMEBUFFERVGA@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
	@sed -i~ -e "s;@PTXFRAMEBUFFERVTSWITCH@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
	@sed -i~ -e "s;@PTXFRAMEBUFFERREVERSE@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
	@sed -i~ -e "s;@PTXHAVETEXTMODE@;Y;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
	@sed -i~ -e "s;@PTXSVGALIB@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
	@sed -i~ -e "s;@PTXHWVGA@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx

ifdef PTXCONF_MICROWINDOWS_MOUSE_NOMOUSE
	@sed -i~ -e "s;@PTXNOMOUSE@;Y;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
else
	@sed -i~ -e "s;@PTXNOMOUSE@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
endif

ifdef PTXCONF_MICROWINDOWS_MOUSE_GPM
	@sed -i~ -e "s;@PTXGPMMOUSE@;Y;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
else
	@sed -i~ -e "s;@PTXGPMMOUSE@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
endif

ifdef PTXCONF_MICROWINDOWS_MOUSE_SER
	@sed -i~ -e "s;@PTXSERMOUSE@;Y;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
else
	@sed -i~ -e "s;@PTXSERMOUSE@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
endif

ifdef PTXCONF_MICROWINDOWS_MOUSE_SUN
	@sed -i~ -e "s;@PTXSUNMOUSE@;Y;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
else
	@sed -i~ -e "s;@PTXSUNMOUSE@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
endif

ifdef PTXCONF_MICROWINDOWS_MOUSE_IPAQ
	@sed -i~ -e "s;@PTXIPAQMOUSE@;Y;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
else
	@sed -i~ -e "s;@PTXIPAQMOUSE@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
endif

ifdef PTXCONF_MICROWINDOWS_MOUSE_ZAURUS
	@sed -i~ -e "s;@PTXZAURUSMOUSE@;Y;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
else
	@sed -i~ -e "s;@PTXZAURUSMOUSE@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
endif

ifdef PTXCONF_MICROWINDOWS_MOUSE_TUX
	@sed -i~ -e "s;@PTXTUXMOUSE@;Y;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
else
	@sed -i~ -e "s;@PTXTUXMOUSE@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
endif

ifdef PTXCONF_MICROWINDOWS_MOUSE_ADS
	@sed -i~ -e "s;@PTXADSMOUSE@;Y;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
else
	@sed -i~ -e "s;@PTXADSMOUSE@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
endif

ifdef PTXCONF_MICROWINDOWS_MOUSE_EP
	@sed -i~ -e "s;@PTXEPMOUSE@;Y;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
else
	@sed -i~ -e "s;@PTXEPMOUSE@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
endif

ifdef PTXCONF_MICROWINDOWS_MOUSE_VH
	@sed -i~ -e "s;@PTXVHMOUSE@;Y;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
else
	@sed -i~ -e "s;@PTXVHMOUSE@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
endif

ifdef PTXCONF_MICROWINDOWS_MOUSE_MT
	@sed -i~ -e "s;@PTXMTMOUSE@;Y;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
else
	@sed -i~ -e "s;@PTXMTMOUSE@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
endif

ifdef PTXCONF_MICROWINDOWS_MOUSE_PSION
	@sed -i~ -e "s;@PTXPSIONMOUSE@;Y;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
else
	@sed -i~ -e "s;@PTXPSIONMOUSE@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
endif

ifdef PTXCONF_MICROWINDOWS_MOUSE_YOPY
	@sed -i~ -e "s;@PTXYOPYMOUSE@;Y;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
else
	@sed -i~ -e "s;@PTXYOPYMOUSE@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
endif

ifdef PTXCONF_MICROWINDOWS_MOUSE_HARRIER
	@sed -i~ -e "s;@PTXHARRIERMOUSE@;Y;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
else
	@sed -i~ -e "s;@PTXHARRIERMOUSE@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
endif

ifdef PTXCONF_MICROWINDOWS_MOUSE_TSLIB
	@sed -i~ -e "s;@PTXTSLIBMOUSE@;Y;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
else
	@sed -i~ -e "s;@PTXTSLIBMOUSE@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
endif

ifdef PTXCONF_MICROWINDOWS_KBD_NOKBD
	@sed -i~ -e "s;@PTXNOKBD@;Y;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
else
	@sed -i~ -e "s;@PTXNOKBD@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
endif

ifdef PTXCONF_MICROWINDOWS_KBD_TTYKBD
	@sed -i~ -e "s;@PTXTTYKBD@;Y;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
else
	@sed -i~ -e "s;@PTXTTYKBD@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
endif

ifdef PTXCONF_MICROWINDOWS_KBD_SCANKBD
	@sed -i~ -e "s;@PTXSCANKBD@;Y;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
else
	@sed -i~ -e "s;@PTXSCANKBD@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
endif

ifdef PTXCONF_MICROWINDOWS_KBD_ZAURUSKBD
	@sed -i~ -e "s;@PTXZAURUSKBD@;Y;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
else
	@sed -i~ -e "s;@PTXZAURUSKBD@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
endif

ifdef PTXCONF_MICROWINDOWS_KBD_PIPEKBD
	@sed -i~ -e "s;@PTXPIPEKBD@;Y;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
else
	@sed -i~ -e "s;@PTXPIPEKBD@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
endif

ifdef PTXCONF_MICROWINDOWS_KBD_IPAQKBD
	@sed -i~ -e "s;@PTXIPAQKBD@;Y;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
else
	@sed -i~ -e "s;@PTXIPAQKBD@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
endif

ifdef PTXCONF_MICROWINDOWS_KBD_LIRCKBD
	@sed -i~ -e "s;@PTXLIRCKBD@;Y;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
else
	@sed -i~ -e "s;@PTXLIRCKBD@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
endif

	@sed -i~ -e "s;@PTXSA1100LCDLTLEND@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
	@sed -i~ -e "s;@PTXSA1100INVERT4BPP@;N;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
	@sed -i~ -e "s;@PTXRTEMSBUILD@;;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
	@sed -i~ -e "s;@PTXRTEMSBSP@;;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
	@sed -i~ -e "s;@PTXASMVGADRIVER@;;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
	@sed -i~ -e "s;@PTXCVGADRIVER@;;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
	@sed -i~ -e "s;@PTXHERCDRIVER@;;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx
	@sed -i~ -e "s;@PTXDBGDRIVER@;;" $(MICROWINDOWS_DIR)/src/Configs/config.ptx

	@cp $(MICROWINDOWS_DIR)/src/Configs/config.ptx $(MICROWINDOWS_DIR)/src/config

	@sed -i~ -e "s;TSLIB_DIR	= .;TSLIB_DIR	= $(SYSROOT)/usr;" $(MICROWINDOWS_DIR)/src/Makefile.rules
	@sed -i~ -e "s;INSTALL_PREFIX  = /;INSTALL_PREFIX  = $(SYSROOT);" $(MICROWINDOWS_DIR)/src/Makefile.rules

	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

microwindows_compile: $(STATEDIR)/microwindows.compile

$(STATEDIR)/microwindows.compile:
	@$(call targetinfo)
	@cd $(MICROWINDOWS_DIR)/src && $(MAKE)
	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

microwindows_install: $(STATEDIR)/microwindows.install

$(STATEDIR)/microwindows.install:
	@$(call targetinfo)
	cd $(MICROWINDOWS_DIR)/src && $(MAKE) install
	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/microwindows.targetinstall:
	@$(call targetinfo)
	@$(call install_init, microwindows)
	@$(call install_fixup,microwindows,PACKAGE,microwindows)
	@$(call install_fixup,microwindows,PRIORITY,optional)
	@$(call install_fixup,microwindows,VERSION,$(MICROWINDOWS_VERSION))
	@$(call install_fixup,microwindows,SECTION,base)
	@$(call install_fixup,microwindows,AUTHOR,"Carsten Behling")
	@$(call install_fixup,microwindows,DEPENDS,)
	@$(call install_fixup,microwindows,DESCRIPTION,missing)

ifdef PTXCONF_MICROWINDOWS_MICROWINDEMO
	@$(call install_copy, microwindows, 0, 0, 0755, $(MICROWINDOWS_DIR)/src/bin/malpha, /usr/bin/malpha)
	@$(call install_copy, microwindows, 0, 0, 0755, $(MICROWINDOWS_DIR)/src/bin/mdemo, /usr/bin/mdemo)
	@$(call install_copy, microwindows, 0, 0, 0755, $(MICROWINDOWS_DIR)/src/bin/mine, /usr/bin/mine)
	@$(call install_copy, microwindows, 0, 0, 0755, $(MICROWINDOWS_DIR)/src/bin/mtest, /usr/bin/mtest)
	@$(call install_copy, microwindows, 0, 0, 0755, $(MICROWINDOWS_DIR)/src/bin/mtest2, /usr/bin/mtest2)
	@$(call install_copy, microwindows, 0, 0, 0755, $(MICROWINDOWS_DIR)/src/bin/muserfd, /usr/bin/muserfd)
	@$(call install_copy, microwindows, 0, 0, 0755, $(MICROWINDOWS_DIR)/src/bin/mterm, /usr/bin/mterm)

endif

ifdef PTXCONF_MICROWINDOWS_SHARED
	@$(call install_copy, microwindows, 0, 0, 0644, $(MICROWINDOWS_DIR)/src/lib/libmwin.so, /usr/lib/libmwin.so)
	@$(call install_copy, microwindows, 0, 0, 0644, $(MICROWINDOWS_DIR)/src/lib/libmwinlib.so, /usr/lib/libmwinlib.so)
	@$(call install_copy, microwindows, 0, 0, 0644, $(MICROWINDOWS_DIR)/src/lib/libmwimages.so, /usr/lib/libmwimages.so)
endif

ifdef PTXCONF_MICROWINDOWS_FONT_NATIVE
	@$(call install_copy, microwindows, 0, 0, 0755, \
	$(PTXCONF_MICROWINDOWS_FONT_NATIVE_DIRECTORY))
endif

ifdef PTXCONF_MICROWINDOWS_FONT_FREETYPE
	@$(call install_copy, microwindows, 0, 0, 0755, \
	$(PTXCONF_MICROWINDOWS_FONT_FREETYPE_DIRECTORY))
endif

#ifdef PTXCONF_MICROWINDOWS_FONT_FREETYPE2
#	@$(call install_copy, microwindows, 0, 0, 0644, \
#	$(PTXDIST_WORKSPACE)/projectroot/usr/share/fonts/tahoma.ttf, #$(PTXCONF_MICROWINDOWS_FONT_FREETYPE2_DIRECTORY)/tahoma.ttf)
#endif

ifdef PTXCONF_MICROWINDOWS_FONT_PCF
	@$(call install_copy, microwindows, 0, 0, 0755, \
	$(PTXCONF_MICROWINDOWS_FONT_PCF_DIRECTORY))
endif

ifdef PTXCONF_MICROWINDOWS_FONT_HZK
	@$(call install_copy, microwindows, 0, 0, 0755, \
	$(PTXCONF_MICROWINDOWS_FONT_HZK_DIRECTORY))
endif

ifdef PTXCONF_MICROWINDOWS_FONT_EUCJP
	@$(call install_copy, microwindows, 0, 0, 0755, \
	$(PTXCONF_MICROWINDOWS_FONT_EUCJP_DIRECTORY))
endif

	@$(call install_finish,microwindows)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

microwindows_clean:
	rm -rf $(STATEDIR)/microwindows.*
	rm -rf $(PKGDIR)/microwindows_*
	rm -rf $(MICROWINDOWS_DIR)

# vim: syntax=make
