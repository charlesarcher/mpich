if BUILD_COLLTREE

endif BUILD_COLLTREE

if BUILD_COLLTREE_DYNAMIC
COLLTREE_SOURCES        = src/mpid/adi/ofi/src/coll/templates/tree/coll.c \
	                  src/mpid/adi/ofi/src/coll/templates/tree/coll_globals.c
lib_LTLIBRARIES        += libcolltree.la
libcolltree_la_SOURCES  = ${COLLTREE_SOURCES}
endif BUILD_COLLTREE_DYNAMIC
