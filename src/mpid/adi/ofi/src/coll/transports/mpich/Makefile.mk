if BUILD_TSPMPICH

endif BUILD_TSPMPICH

if BUILD_TSPMPICH_DYNAMIC
TSPMPICH_SOURCES        = src/mpid/adi/ofi/src/coll/transports/mpich/transport.c
lib_LTLIBRARIES        += libtspmpich.la
libtspmpich_la_SOURCES  = ${TSPMPICH_SOURCES}
endif BUILD_TSPMPICH_DYNAMIC
