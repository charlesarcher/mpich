# note that the includes always happen but the effects of their contents are
# affected by "if BUILD_OFID"
if BUILD_OFI
# Inlined ADI(No objects built)
include $(top_srcdir)/src/mpid/adi/ofi/src/coll/Makefile.mk
mpi_core_sources                += src/mpid/adi/ofi/src/mpid_util.cc \
                                   src/mpid/adi/ofi/src/coll/coll_globals.c
external_ldflags                += -lstdc++ -ldl -lpthread
lib_lib@MPILIBNAME@_la_CXXFLAGS  = -fno-rtti -fno-exceptions

endif BUILD_OFI

if BUILD_OFI_DYNAMIC
# Dynamically Loadable ADI
include $(top_srcdir)/src/mpid/adi/ofi/src/coll/Makefile.mk
OFI_SOURCES =                                \
    src/mpid/adi/ofi/src/mpid_init.c         \
    src/mpid/adi/ofi/src/mpid_probe.c        \
    src/mpid/adi/ofi/src/mpid_send.c         \
    src/mpid/adi/ofi/src/mpid_recv.c         \
    src/mpid/adi/ofi/src/mpid_request.c      \
    src/mpid/adi/ofi/src/mpid_comm.c         \
    src/mpid/adi/ofi/src/mpid_win.c          \
    src/mpid/adi/ofi/src/mpid_coll.c         \
    src/mpid/adi/ofi/src/mpid_progress.c     \
    src/mpid/adi/ofi/src/mpid_globals.c      \
    src/mpid/adi/ofi/src/mpid_dynproc.c      \
    src/mpid/adi/ofi/src/mpid_util.cc        \
    src/mpid/adi/ofi/src/coll/coll_globals.c \
    src/mpid/adi/ofi/src/mpid_unimpl.cc

lib_LTLIBRARIES                 += libadiofi.la
libadiofi_la_SOURCES             = ${OFI_SOURCES}
lib_lib@MPILIBNAME@_la_CXXFLAGS  = -fno-rtti -fno-exceptions
external_ldflags                += -ldl -lpthread

endif BUILD_OFI_DYNAMIC
