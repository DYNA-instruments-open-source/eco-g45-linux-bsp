# list of platforms which did not want this test case
EXCLUDE_LIST:= IMX25_3STACK IMX53_VINCELL

ifeq (,$(findstring $(PLATFORM), $(EXCLUDE_LIST)))
OBJS = $(OBJDIR)/rtc_timesetnotification_test.out $(OBJDIR)/rtctest.out $(OBJDIR)/rtcwakeup.out $(OBJDIR)/autorun-rtc.sh
else
OBJS =
endif

all : $(OBJS)

.PHONY: clean
clean :
	rm -f $(OBJS)

#
# include the Rules
#
include ../make.rules

