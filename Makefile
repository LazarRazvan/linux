##
# All subdirectories.
##
SUBDIRS := $(wildcard src/*/.)

##
# Targets for each subdirectory.
##
TARGETS := all clean


$(TARGETS): $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: $(TARGETS) $(SUBDIRS)
