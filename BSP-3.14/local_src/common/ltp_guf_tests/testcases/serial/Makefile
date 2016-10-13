##########################################################################
##																		##
##  Copyright (C) 2011 Garz & Fricke GmbH								##
##																		##
##		No use or disclosure of this information in any form without	##
##		the written permission of the author							##
##																		##
##########################################################################

TARGET=serial


top_srcdir	?= $(SYSROOT)/usr

CFLAGS+=    -I$(top_srcdir)/include -Wall -Werror
LOADLIBES+= -L$(top_srcdir)/lib -lltp -lpthread

SRCS=$(TARGET).c


all: $(TARGET)

$(TARGET): $(SRCS)
	@if [ ! -d "../bin" ]; then \
		mkdir ../bin; \
	fi
	@$(CC) $(CFLAGS) $(LDFLAGS) $< $(LOADLIBES) -o ../bin/$@

clean:
	rm -f $(TARGET) *.o *.bak *~
