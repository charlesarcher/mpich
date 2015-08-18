if BUILD_COLLSTUB

endif BUILD_COLLSTUB

if BUILD_COLLSTUB_DYNAMIC
COLLSTUB_SOURCES        = src/mpid/adi/ofi/src/coll/templates/stub/coll.c
lib_LTLIBRARIES        += libcollstub.la
libcollstub_la_SOURCES  = ${COLLSTUB_SOURCES}
endif BUILD_COLLSTUB_DYNAMIC
