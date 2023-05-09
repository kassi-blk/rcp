CC = gcc
CFLAGS = \
	-g \
	-Wall \
	-Iinclude \
	-Isrc/free_context/include \
	-Isrc/transfer/include
LDFLAGS = \
	-g \
	-Llibs \
	-ltrans \
	-lfc
INCDIR = include
SRCDIR = src
LIBDIR = libs
OBJDIR = obj
BINDIR = bin
LIBS = $(addprefix $(LIBDIR)/,libtrans.a libfc.a)

all: $(BINDIR)/server $(BINDIR)/client
	ln -srf $(BINDIR)/server server
	ln -srf $(BINDIR)/client client

# Build server
$(BINDIR)/server: $(OBJDIR)/server.o $(LIBS) | $(BINDIR)
	$(CC) -o $@ $< $(LDFLAGS)
$(OBJDIR)/server.o: $(SRCDIR)/server.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Build client
$(BINDIR)/client: $(OBJDIR)/client.o $(LIBS) | $(BINDIR)
	$(CC) -o $@ $< $(LDFLAGS)
$(OBJDIR)/client.o: $(SRCDIR)/client.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile common libraries
.PHONY: $(LIBDIR)/libtrans.a
$(LIBDIR)/libtrans.a: | $(LIBDIR)
	make -C $(SRCDIR)/transfer
	ln -srf $(SRCDIR)/transfer/libtrans.a $@
.PHONY: $(LIBDIR)/libfc.a
$(LIBDIR)/libfc.a: | $(LIBDIR)
	make -C $(SRCDIR)/free_context
	ln -srf $(SRCDIR)/free_context/libfc.a $@

$(BINDIR):
	mkdir $(BINDIR)

$(LIBDIR):
	mkdir $(LIBDIR)

$(OBJDIR):
	mkdir $(OBJDIR)

clean-all: clean
	make -C $(SRCDIR)/free_context clean
	make -C $(SRCDIR)/transfer clean
	rm -rf $(LIBDIR)

clean:
	rm -rf $(OBJDIR) $(BINDIR) server client
