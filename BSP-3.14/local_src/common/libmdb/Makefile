OBJ = libmdb.o
HEADERS = libmdb.h

LIBNAME = libmdb
SONAMEVERSION=0

all: $(LIBNAME).so $(LIBNAME).a

install: install_headers
	@mkdir -p $(DEST_DIR)/usr/lib
	cp -P $(LIBNAME).* $(DEST_DIR)/usr/lib

install_headers:
	@mkdir -p $(DEST_DIR)/usr/include
	cp $(HEADERS) $(DEST_DIR)/usr/include

%.o: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) -fPIC -g -c $^ -o $@

$(LIBNAME).so.$(SONAMEVERSION): $(OBJ)
	$(CC) -shared -Wl,-soname,$@ $^ -o $@ -lc

$(LIBNAME).so: $(LIBNAME).so.$(SONAMEVERSION)
	ln -s $< $@

$(LIBNAME).a: $(OBJ)
	$(AR) -rc $@  $^

.PHONY: clean
clean:
	rm -f $(LIBNAME).* $(OBJ)
