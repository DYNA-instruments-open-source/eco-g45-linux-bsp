APP := fbiopan_display_test
CFLAGS += -Wall -Werror

all: $(APP)

$(APP): $(APP).o
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(APP) *.o *~ *.bak
