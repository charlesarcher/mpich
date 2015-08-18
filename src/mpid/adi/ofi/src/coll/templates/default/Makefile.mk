if BUILD_COLLDEFAULT

endif BUILD_COLLDEFAULT

if BUILD_COLLDEFAULT_DYNAMIC
COLLDEFAULT_SOURCES        = src/mpid/adi/ofi/src/coll/templates/default/coll.c
lib_LTLIBRARIES           += libcolldefault.la
libcolldefault_la_SOURCES  = ${COLLDEFAULT_SOURCES}
endif BUILD_COLLDEFAULT_DYNAMIC
