##########################################################################
##																		##
##  Copyright (C) 2011 Garz & Fricke GmbH								##
##																		##
##		No use or disclosure of this information in any form without	##
##		the written permission of the author							##
##																		##
##########################################################################

TARGET=touchmon


top_srcdir	?= $(SYSROOT)/usr

CFLAGS+=    -I$(top_srcdir)/include -Wall -Werror
LOADLIBES+= -L$(top_srcdir)/lib -lltp -lpthread

SRCS=$(TARGET).c event_thread.c server_thread.c requests.c utils.c filter.c

SRCS += filter_sync.c filter_raw.c filter_touchpress.c filter_toucharea.c

all: $(TARGET)

$(TARGET): $(SRCS)
	@if [ ! -d "../bin" ]; then \
		mkdir ../bin; \
	fi
	@$(CC) $(CFLAGS) $(LDFLAGS) $(SRCS) $(LOADLIBES) -o ../bin/$@

clean:
	rm -f $(TARGET) *.o *.bak *~
