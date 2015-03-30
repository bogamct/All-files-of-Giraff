
###### Define directories to build ######

SUBDIRS =	a2d			\
			ads7828		\
			ads7870		\
			basic_io	\
			cmdline		\
			ds1631		\
			encoder		\
			extint		\
			glcd		\
			gps			\
			i2c			\
			lcd			\
			mmc			\
			netstack	\
			pulse		\
			rprintf		\
			servo		\
			stxetx		\
			timer		\

#SUBDIRS = $(dir ./*)

###### Define build rules ######

.PHONY: examples $(SUBDIRS)
examples: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY : all
all : examples

.PHONY : clean
clean : examples
	rm -f ../*.lst
	rm -f ../rsl/*.lst
	rm -f ../megaio/*.lst
	rm -f ../net/*.lst

