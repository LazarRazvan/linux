##
# All subdirectories.
##
SUBDIRS := $(wildcard */.)

##
# Targets for each subdirectory.
##
TARGETS := all clean


$(TARGETS): $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: $(TARGETS) $(SUBDIRS)
