# list of platforms which did not want this test case
EXCLUDE_LIST:="IMX27ADS IMX37_3STACK IMX35_3STACK IMX51 IMX25_3STACK IMX53_VINCELL"

# list of platforms which do not have either SIR or FIR
IR_EXCLUDE_LIST:= IMX53_VINCELL

ifeq (,$(findstring $(PLATFORM), $(EXCLUDE_LIST)))
OBJS = $(OBJDIR)/autorun-firi.sh
else
OBJS =
endif

ifeq (,$(findstring $(PLATFORM), $(IR_EXCLUDE_LIST)))
SCRIPTS = $(OBJDIR)/mxc_irlan_test.sh
else
SCRIPTS =
endif

all : $(OBJS) $(SCRIPTS)

.PHONY: clean
clean :
	rm -f $(OBJS) $(SCRIPTS)

#
# include the Rules
#
include ../make.rules

