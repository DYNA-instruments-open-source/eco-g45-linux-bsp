#
# Makefile for OSS sound driver test app.
#
# list of platforms which did not want this test case
EXCLUDE_LIST:= IMX21ADS IMX27ADS IMX31ADS IMX32ADS IMX37_3STACK IMX25_3STACK IMX53_VINCELL

ifeq (,$(findstring $(PLATFORM), $(EXCLUDE_LIST)))
OBJS = \
	$(OBJDIR)/mxc_asrc_test.out \
	$(OBJDIR)/autorun-asrc.sh
else
OBJS =
endif

all : $(OBJS)
ifeq (,$(findstring $(PLATFORM), $(EXCLUDE_LIST)))
	cp audio8k16S.wav $(OBJDIR)
endif

.PHONY: clean
clean :
	rm -f $(OBJS)

#
# include the Rules
#
include ../make.rules
