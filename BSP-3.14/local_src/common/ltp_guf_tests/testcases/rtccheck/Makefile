##########################################################################
##																		##
##  Copyright (C) 2010 Garz & Fricke GmbH								##
##																		##
##		No use or disclosure of this information in any form without	##
##		the written permission of the author							##
##																		##
##########################################################################

TARGET=rtccheck


top_srcdir	?= $(SYSROOT)/usr

CFLAGS+=    -I$(top_srcdir)/include
LOADLIBES+= -L$(top_srcdir)/lib -lltp

SRCS=$(TARGET).c


all: $(TARGET)

$(TARGET): $(SRCS)
	@if [ ! -d "../bin" ]; then \
		mkdir ../bin; \
	fi
	@$(CC) $(CFLAGS) $(LDFLAGS) $< $(LOADLIBES) -o ../bin/$@

clean:
	rm -f $(TARGET) *.o *.bak *~