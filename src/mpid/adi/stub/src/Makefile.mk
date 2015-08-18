
# note that the includes always happen but the effects of their contents are
# affected by "if BUILD_STUB"

if BUILD_STUB
# Inlined ADI(No objects built)
lib_lib@MPILIBNAME@_la_CXXFLAGS = -fno-rtti -fno-exceptions
endif BUILD_STUB

if BUILD_STUB_DYNAMIC
# Dynamically Loadable ADI
STUB_SOURCES =                              \
    src/mpid/adi/stub/src/mpid_init.c       \
    src/mpid/adi/stub/src/mpid_probe.c      \
    src/mpid/adi/stub/src/mpid_send.c       \
    src/mpid/adi/stub/src/mpid_recv.c       \
    src/mpid/adi/stub/src/mpid_request.c    \
    src/mpid/adi/stub/src/mpid_comm.c       \
    src/mpid/adi/stub/src/mpid_win.c        \
    src/mpid/adi/stub/src/mpid_progress.c   \
    src/mpid/adi/stub/src/mpid_globals.c    \
    src/mpid/adi/stub/src/mpid_unimpl.cc

lib_LTLIBRARIES                 += libadistub.la
libadistub_la_SOURCES            = ${STUB_SOURCES}
lib_lib@MPILIBNAME@_la_CXXFLAGS  = -fno-rtti -fno-exceptions

endif BUILD_STUB_DYNAMIC
