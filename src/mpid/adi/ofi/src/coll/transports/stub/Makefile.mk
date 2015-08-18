if BUILD_TSPSTUB

endif BUILD_TSPSTUB

if BUILD_TSPSTUB_DYNAMIC
TSPSTUB_SOURCES         = src/mpid/adi/ofi/src/coll/transports/stub/transport.c
lib_LTLIBRARIES        += libtspstub.la
libtspstub_la_SOURCES   = ${TSPSTUB_SOURCES}
endif BUILD_TSPSTUB_DYNAMIC
