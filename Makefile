

SUBDIRS := char-in-set thumbsize texify sadd pg-human pg-coding natural_sex mule max


.DEFAULT:	all


all clean install:
	for d in $(SUBDIRS); do $(MAKE) -C $$d $@; done
