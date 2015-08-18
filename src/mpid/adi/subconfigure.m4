[#] start of __file__

dnl _PREREQ handles the former role of mpichprereq, setup_device, etc
[#] expansion is: PAC_SUBCFG_PREREQ_[]PAC_SUBCFG_AUTO_SUFFIX
AC_DEFUN([PAC_SUBCFG_PREREQ_]PAC_SUBCFG_AUTO_SUFFIX,[
AM_CONDITIONAL([BUILD_ADI],[test "$device_name" = "adi"])

AM_COND_IF([BUILD_ADI],[
dnl Add code to sub-select adi modules to build
dnl Add code to sub-select, but only inline one

# Set a value for the maximum processor name.
MPID_MAX_PROCESSOR_NAME=128

# Set a value for the maximum error string.
MPID_MAX_ERROR_STRING=1024
MPID_DEVICE_TIMER_TYPE=double
MPID_MAX_THREAD_LEVEL=MPI_THREAD_MULTIPLE

# Build common code
build_mpid_common_sched=yes
build_mpid_common_datatype=yes
build_mpid_common_thread=yes

dnl Parse the device arguments
SAVE_IFS=$IFS
IFS=':'
args_array=$device_args
do_dynamic_adi=true
do_inline_stub=false
do_inline_ofi=false
echo "Parsing Arguments for ADI Device"
for arg in $args_array; do
  case ${arg} in
    inline-stub)
      do_inline_stub=true
      do_dynamic_adi=false
      echo " ---> ADI::STUB Inlined STUB Provider"
      ;;
    inline-ofi)
      do_inline_ofi=true
      do_dynamic_adi=false
      echo " ---> ADI::OFI Inlined OFI Provider"
      ;;
  esac
done
IFS=$SAVE_IFS

])dnl end AM_COND_IF(BUILD_ADI,...)
])dnl end PREREQ

AC_DEFUN([PAC_SUBCFG_BODY_]PAC_SUBCFG_AUTO_SUFFIX,[

AM_COND_IF([BUILD_ADI],[
AC_MSG_NOTICE([RUNNING CONFIGURE FOR ADI DEVICES])

PAC_APPEND_FLAG([-I${master_top_srcdir}/src/include],              [CPPFLAGS])
PAC_APPEND_FLAG([-I${master_top_srcdir}/src/util/wrappers],        [CPPFLAGS])
PAC_APPEND_FLAG([-I${master_top_srcdir}/src/mpid/common/datatype], [CPPFLAGS])
PAC_APPEND_FLAG([-I${master_top_srcdir}/src/mpid/common/locks],    [CPPFLAGS])
PAC_APPEND_FLAG([-I${master_top_srcdir}/src/mpid/common/thread],   [CPPFLAGS])
PAC_APPEND_FLAG([-I${master_top_srcdir}/src/mpid/common/sched],    [CPPFLAGS])
PAC_APPEND_FLAG([-I${master_top_srcdir}/src/mpid/adi/include],     [CPPFLAGS])

])dnl end AM_COND_IF(BUILD_,...)
])dnl end _BODY

[#] end of __file__
