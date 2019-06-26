# -*-makefile-*-
#
# Copyright (C) 2011 by Carsten Behling <carsten.behling@garz-fricke.com>
#
# See CREDITS for details about who has contributed to this project.
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
HOST_PACKAGES-$(PTXCONF_HOST_SAM_BA) += host-sam-ba

#
# Paths and names
#
HOST_SAM_BA_VERSION	:= 2.11
HOST_SAM_BA_MD5		:=
HOST_SAM_BA		:= sam-ba_$(HOST_SAM_BA_VERSION)
HOST_SAM_BA_SUFFIX	:= 
HOST_SAM_BA_URL		:= file://$(PTXDIST_WORKSPACE)/local_src/common/$(HOST_SAM_BA)
HOST_SAM_BA_SRCDIR	:= $(PTXDIST_WORKSPACE)/local_src/common/$(HOST_SAM_BA)
HOST_SAM_BA_DIR		:= $(HOST_BUILDDIR)/$(HOST_SAM_BA)
HOST_SAM_BA_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

$(STATEDIR)/host-sam-ba.extract:
	@$(call targetinfo)
	@$(call clean, $(HOST_SAM_BA_DIR))
	@rm -Rf $(HOST_SAM_BA_DIR)
	@cp -R $(HOST_SAM_BA_SRCDIR) $(HOST_SAM_BA_DIR)
	@$(call patchin, HOST_SAM_BA)
	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

$(STATEDIR)/host-sam-ba.prepare:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

$(STATEDIR)/host-sam-ba.compile:
	@$(call targetinfo)
	# Compile the applets
ifdef PTXCONF_HOST_SAM_BA_APPLETS_DATAFLASH
	# ... Dataflash
	@cd $(HOST_SAM_BA_DIR)/applets/legacy/sam-ba_applets/dataflash && $(MAKE) \
		CHIP=$(PTXCONF_HOST_SAM_BA_CHIP) BOARD=$(PTXCONF_HOST_SAM_BA_BOARD) \
		CROSS_COMPILE=$(PTXCONF_HOST_SAM_BA_CROSS_COMPILE)
endif

ifdef PTXCONF_HOST_SAM_BA_APPLETS_EEPROM
	# ... EEPROM
	@cd $(HOST_SAM_BA_DIR)/applets/legacy/sam-ba_applets/eeprom && $(MAKE) \
		CHIP=$(PTXCONF_HOST_SAM_BA_CHIP) BOARD=$(PTXCONF_HOST_SAM_BA_BOARD) \
		CROSS_COMPILE=$(PTXCONF_HOST_SAM_BA_CROSS_COMPILE)
endif

ifdef PTXCONF_HOST_SAM_BA_APPLETS_EXTRAM
	# ... External RAM
	@cd $(HOST_SAM_BA_DIR)/applets/legacy/sam-ba_applets/extram && $(MAKE) \
		CHIP=$(PTXCONF_HOST_SAM_BA_CHIP) BOARD=$(PTXCONF_HOST_SAM_BA_BOARD) \
		CROSS_COMPILE=$(PTXCONF_HOST_SAM_BA_CROSS_COMPILE)
endif

ifdef PTXCONF_HOST_SAM_BA_APPLETS_LOWLEVELINIT
	# ... LOWLEVELINIT
	@cd $(HOST_SAM_BA_DIR)/applets/legacy/sam-ba_applets/lowlevelinit && $(MAKE) \
		CHIP=$(PTXCONF_HOST_SAM_BA_CHIP) BOARD=$(PTXCONF_HOST_SAM_BA_BOARD) \
		CROSS_COMPILE=$(PTXCONF_HOST_SAM_BA_CROSS_COMPILE)
endif

ifdef PTXCONF_HOST_SAM_BA_APPLETS_NANDFLASH
	# ... NANDFLASH
	@cd $(HOST_SAM_BA_DIR)/applets/legacy/sam-ba_applets/nandflash && $(MAKE) ddram\
		CHIP=$(PTXCONF_HOST_SAM_BA_CHIP) BOARD=$(PTXCONF_HOST_SAM_BA_BOARD) \
		CROSS_COMPILE=$(PTXCONF_HOST_SAM_BA_CROSS_COMPILE)
endif

ifdef PTXCONF_HOST_SAM_BA_APPLETS_NORFLASH
	# ... NORFLASH
	@cd $(HOST_SAM_BA_DIR)/applets/legacy/sam-ba_applets/norflash && $(MAKE) \
		CHIP=$(PTXCONF_HOST_SAM_BA_CHIP) BOARD=$(PTXCONF_HOST_SAM_BA_BOARD) \
		CROSS_COMPILE=$(PTXCONF_HOST_SAM_BA_CROSS_COMPILE)
endif

ifdef PTXCONF_HOST_SAM_BA_APPLETS_SERIALFLASH
	# ... SERIALFLASH
	@cd $(HOST_SAM_BA_DIR)/applets/legacy/sam-ba_applets/serialflash && $(MAKE) \
		CHIP=$(PTXCONF_HOST_SAM_BA_CHIP) BOARD=$(PTXCONF_HOST_SAM_BA_BOARD) \
		CROSS_COMPILE=$(PTXCONF_HOST_SAM_BA_CROSS_COMPILE)
endif

	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

$(STATEDIR)/host-sam-ba.install:
	@$(call targetinfo)
	cd $(HOST_BUILDDIR) && tar -cjf $(IMAGEDIR)/$(HOST_SAM_BA).tar.bz2 $(HOST_SAM_BA)
	@$(call touch)

# ----------------------------------------------------------------------------
# Install Package
# ----------------------------------------------------------------------------

$(STATEDIR)/host-sam-ba.install.pack:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

#$(STATEDIR)/host-sam-ba.clean:
#	@$(call targetinfo)
#	@$(call clean_pkg, HOST_SAM_BA)

# vim: syntax=make
