##########################################################################
##																		##
##  Copyright (C) 2010 Garz & Fricke GmbH								##
##																		##
##		No use or disclosure of this information in any form without	##
##		the written permission of the author							##
##																		##
##########################################################################
	
TARGET=iptest


top_srcdir	?= $(SYSROOT)/usr

CFLAGS+=    -I$(top_srcdir)/include -Wall -Wextra -Werror
LOADLIBES+= -L$(top_srcdir)/lib -lltp -pthread -lrt -lm

SRCS=iptest.c stack.c initThread.c sendThread.c recvThread.c statThread.c

all: $(TARGET)

$(TARGET): $(SRCS)
	@if [ ! -d "../bin" ]; then \
		mkdir ../bin; \
	fi
	@$(CC) $(CFLAGS) $(LDFLAGS) $(SRCS) $(LOADLIBES) -o ../bin/$@

clean:
	rm -f $(TARGET) *.o *.bak *~
