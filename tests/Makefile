include ../config.make

FIX_LD = LD_LIBRARY_PATH=../c/lib2:${LD_LIBRARY_PATH}
ARCH_CFLAGS += -Wall -Wno-unused
ARCH_CFLAGS += -fno-unroll-all-loops -funroll-loops -g
LDSOFLAGS = $(GRIDFLOW_LDSOFLAGS)

### for hardcore malloc debugging; standalone only
# LDSOFLAGS += -lefence

LIB=../c/lib2/lib$(PNAME).so

$(LIB)::
	(cd ../c/src; $(MAKE) -k standalone)

test: test1 test2
	$(FIX_LD) ./test2
.PHONY: test

test1: test1.c $(LIB)
	gcc $(ARCH_CFLAGS) $(LDSOFLAGS) -DSTANDALONE test1.c $(LIB) -o $@

test2: test2.c $(LIB)
	gcc $(ARCH_CFLAGS) $(LDSOFLAGS) -DSTANDALONE test2.c $(LIB) -o $@

clean:
	rm -f test1 test2
.PHONY: clean

#mpeg_test: mpeg_test.c
#	gcc $(ARCH_CFLAGS) mpeg_test.c $(LIB) -o $@ -lmpeg

