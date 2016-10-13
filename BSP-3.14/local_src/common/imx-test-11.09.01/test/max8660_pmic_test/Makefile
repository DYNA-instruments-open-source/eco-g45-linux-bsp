# list of platforms which did not want this test case
EXCLUDE_LIST:="IMX51 IMX37_3STACK IMX31_3STACK IMX31ADS IMX27_3STACK IMX27ADS IMX25_3STACK IMX53_VINCELL"


LTP_DIR = $(TOPDIR)/test/mxc_pmic_test/
LOADLIBES+= -L$(LTP_DIR)lib -lltp

ifeq (,$(findstring $(PLATFORM), $(EXCLUDE_LIST)))
OBJS = autorun-mcu_pmic.sh libltp.a
else
OBJS =
endif

CFLAGS+= -Wall -I$(LTP_DIR)/include
CFLAGS+= $(INC) -I$(LINUXPATH)/include/ -D__CVS_TEST

CFLAGS+= -DCONFIG_MXC_PMIC_MC9SDZ60

export CFLAGS LTP_DIR LOADLIBES

SUBDIRS =       protocol_tests \
		pmic_testapp_rtc

all: $(OBJS)
ifeq (,$(findstring $(PLATFORM), $(EXCLUDE_LIST)))
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i ; done
endif

libltp.a:
ifeq (,$(findstring $(PLATFORM), $(EXCLUDE_LIST)))
	@$(MAKE) -C $(LTP_DIR)lib/ $@
	mkdir -p $(OBJDIR)
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i ; done
endif

install:
ifeq (,$(findstring $(PLATFORM), $(EXCLUDE_LIST)))
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i install ; done
endif

.PHONY: clean
clean:
ifeq (,$(findstring $(PLATFORM), $(EXCLUDE_LIST)))
	@$(MAKE) -C lib clean
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i clean ; done
endif

#
# include the Rules
#
include ../make.rules
