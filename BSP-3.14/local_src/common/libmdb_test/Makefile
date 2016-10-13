APP := mdbtest

all: $(APP)

$(APP): $(APP).o
	$(CC) $(CFLAGS) $(LDFLAGS) $< $(LOADLIBES) -lmdb -o $@

clean:
	rm -f $(APP) *.o *~ *.bak
