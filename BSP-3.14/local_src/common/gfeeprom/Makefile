APP := gfeeprom

all: $(APP)

$(APP): $(APP).o
	$(CC) -o $@ $<

clean:
	rm -f $(APP) *.o *~ *.bak
