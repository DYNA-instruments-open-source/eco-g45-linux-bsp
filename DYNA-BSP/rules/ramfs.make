# -*-makefile-*-
#
# Copyright (C) 2011 by Carsten behling <carsten.behling@garz-fricke.com>
#
# See CREDITS for details about who has contributed to this project.
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_RAMFS) += ramfs

#
# Paths and names
#
RAMFS_VERSION	:= trunk
RAMFS_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Get
# ----------------------------------------------------------------------------

$(STATEDIR)/ramfs.get:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

$(STATEDIR)/ramfs.prepare:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

$(STATEDIR)/ramfs.compile:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

$(STATEDIR)/ramfs.install:
	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/ramfs.targetinstall:
	@$(call targetinfo)

	@$(call install_init, ramfs)
	@$(call install_fixup, ramfs,PRIORITY,optional)
	@$(call install_fixup, ramfs,SECTION,base)
	@$(call install_fixup, ramfs,AUTHOR,"Carsten behling <carsten.behling@garz-fricke.com>")
	@$(call install_fixup, ramfs,DESCRIPTION,missing)

	#
	# Startup script
	#
	@$(call install_alternative, ramfs, 0, 0, 0755, \
		/etc/init.d/ramfs)
	@$(call install_link, ramfs, ../init.d/ramfs, \
		/etc/rc.d/S97ramfs)

	@$(call install_finish, ramfs)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

#$(STATEDIR)/ramfs.clean:
#	@$(call targetinfo)
#	@$(call clean_pkg, SRAM)

# vim: syntax=make
