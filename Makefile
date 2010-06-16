SRCDIR=src/

SUBDIRS=$(SRCDIR)

all: 	
	for i in $(SUBDIRS) ; do \
	( cd $$i ; make ) ; \
	done

clean:
	rm -f *~
	for i in $(SUBDIRS) ; do \
	( cd $$i ; make clean ) ; \
	done
