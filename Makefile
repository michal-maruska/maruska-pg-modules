

SUBDIRS := char-in-set thumbsize texify sadd human coding natural-sex mule max


.DEFAULT:	all


all clean install:
	for d in $(SUBDIRS); do $(MAKE) -C $$d $@; done
