# note that the includes always happen but the effects of their contents are
# affected by "if BUILD_ADI"
if BUILD_ADI

include $(top_srcdir)/src/mpid/adi/stub/Makefile.mk
include $(top_srcdir)/src/mpid/adi/ofi/Makefile.mk

# Function loader, used for dynamic modules
mpi_core_sources += src/mpid/adi/mpid_func.c


endif BUILD_ADI
