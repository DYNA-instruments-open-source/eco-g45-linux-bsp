# list of platforms which did not want this test case
EXCLUDE_LIST:= IMX53_VINCELL

ifeq (,$(findstring $(PLATFORM), $(EXCLUDE_LIST)))
OBJS = $(OBJDIR)/autorun-mmc.sh
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

