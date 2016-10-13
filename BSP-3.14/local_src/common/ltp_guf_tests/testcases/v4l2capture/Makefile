##########################################################################
##																		##
##  Copyright (C) 2011 Garz & Fricke GmbH								##
##																		##
##		No use or disclosure of this information in any form without	##
##		the written permission of the author							##
##																		##
##########################################################################

TARGET=v4l2capture


top_srcdir	?= $(SYSROOT)/usr

CFLAGS+=    -I$(top_srcdir)/include -Wall -Werror -O3
LOADLIBES+= -L$(top_srcdir)/lib -lltp

SRCS=$(TARGET).c framebuffer.c capture.c format.c

ifdef NEON
ADDITIONAL_FLAGS = -DNEON=$(NEON)
endif

all: $(TARGET)

$(TARGET): $(SRCS)
	@if [ ! -d "../bin" ]; then \
		mkdir ../bin; \
	fi
	@$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LOADLIBES) -o ../bin/$@ $(ADDITIONAL_FLAGS)

clean:
	rm -f $(TARGET) *.o *.bak *~