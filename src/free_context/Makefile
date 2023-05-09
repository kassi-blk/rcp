CC = gcc
CFLAGS = \
	-g \
	-c \
	-Iinclude
INCDIR = include
SRCDIR = src
OBJDIR = obj
LIBNAME = libfc.a

all: $(LIBNAME)

$(LIBNAME): $(OBJDIR)/free_context.o
	ar rc $@ $<

$(OBJDIR)/free_context.o: $(SRCDIR)/free_context.c | $(OBJDIR)
	gcc $(CFLAGS) $< -o $@

$(OBJDIR):
	-mkdir $(OBJDIR)

clean:
	rm -rf obj $(LIBNAME)
