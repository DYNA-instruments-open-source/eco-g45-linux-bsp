##########################################################################
##																		##
##  Copyright (C) 2010 Garz & Fricke GmbH								##
##																		##
##		No use or disclosure of this information in any form without	##
##		the written permission of the author							##
##																		##
##########################################################################

TARGET=mmc


SRCS=$(TARGET)


all: $(TARGET)

$(TARGET): $(SRCS).sh
	@if [ ! -d "../bin" ]; then \
		mkdir ../bin; \
	fi
	@cp $(TARGET).sh ../bin/$(TARGET)

clean:
	