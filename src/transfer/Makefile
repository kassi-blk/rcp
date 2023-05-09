CC = gcc
CFLAGS = \
	-g \
	-c \
	-Iinclude
INCDIR = include
SRCDIR = src
OBJDIR = obj
LIBNAME = libtrans.a

all: $(LIBNAME)

$(LIBNAME): $(OBJDIR)/transfer.o
	ar rc $@ $<

$(OBJDIR)/transfer.o: $(SRCDIR)/transfer.c | $(OBJDIR)
	gcc $(CFLAGS) $< -o $@

$(OBJDIR):
	-mkdir $(OBJDIR)

clean:
	rm -rf obj $(LIBNAME)
