# -*-makefile-*-
#
# Copyright (C) 2012 by Tim Jaacks <tim.jaacks@garz-fricke.com>
#
# See CREDITS for details about who has contributed to this project.
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_LIBMDB) += libmdb

#
# Paths and names
#
LIBMDB_VERSION	:= 1.0
LIBMDB			:= libmdb
LIBMDB_SUFFIX	:= 
LIBMDB_URL		:= file://$(PTXDIST_WORKSPACE)/local_src/common/$(LIBMDB)
LIBMDB_SRCDIR	:= $(PTXDIST_WORKSPACE)/local_src/common/$(LIBMDB)
LIBMDB_DIR		:= $(BUILDDIR)/$(LIBMDB)
LIBMDB_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

$(STATEDIR)/libmdb.extract:
	@$(call targetinfo)
	@$(call clean, $(LIBMDB_DIR))
	@rm -Rf $(LIBMDB_DIR)
	@cp -R $(LIBMDB_SRCDIR) $(LIBMDB_DIR)
	@$(call patchin, LIBMDB)
	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

$(STATEDIR)/libmdb.prepare:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

$(STATEDIR)/libmdb.compile:
	@$(call targetinfo)
	@cd $(LIBMDB_DIR) && PATH=$(CROSS_PATH) $(CROSS_ENV) \
		CROSS_COMPILE=$(KERNEL_CROSS_COMPILE) $(MAKE)
	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

$(STATEDIR)/libmdb.install:
	@$(call targetinfo)
	@cd $(LIBMDB_DIR) && DEST_DIR=$(SYSROOT) $(MAKE) install
	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/libmdb.targetinstall:
	@$(call targetinfo)

	@$(call install_init, libmdb)
	@$(call install_fixup, libmdb, PRIORITY, optional)
	@$(call install_fixup, libmdb, SECTION, base)
	@$(call install_fixup, libmdb, AUTHOR, "Tim Jaacks <tim.jaacks@garz-fricke.com>")
	@$(call install_fixup, libmdb, DESCRIPTION, missing)

	@$(call install_copy, libmdb, 0, 0, 0644, $(SYSROOT)/usr/lib/libmdb.so.0, /usr/lib/libmdb.so.0)
	@$(call install_link, libmdb, libmdb.so.0, /usr/lib/libmdb.so)
	@$(call install_copy, libmdb, 0, 0, 0644, $(SYSROOT)/usr/include/libmdb.h, /usr/include/libmdb.h)

	@$(call install_finish, libmdb)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

#$(STATEDIR)/libmdb.clean:
#	@$(call targetinfo)
#	@$(call clean_pkg, LIBMDB)

# vim: syntax=make
