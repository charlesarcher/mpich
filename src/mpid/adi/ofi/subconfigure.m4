[#] start of __file__

dnl _PREREQ handles the former role of mpichprereq, setup_device, etc
[#] expansion is: PAC_SUBCFG_PREREQ_[]PAC_SUBCFG_AUTO_SUFFIX
AC_DEFUN([PAC_SUBCFG_PREREQ_]PAC_SUBCFG_AUTO_SUFFIX,[

dnl Parse the device arguments
SAVE_IFS=$IFS
IFS=':'
args_array=$device_args
do_scalable_endpoints=false
do_direct_provider=false
do_mr_offset=false
do_av_table=false
echo "Parsing Arguments for ADI Device"
for arg in $args_array; do
  case ${arg} in
    scalable-endpoints)
      do_scalable_endpoints=true
      echo " ---> ADI::OFI Provider : $arg"
      ;;
      av_table)
      do_av_table=true
      echo " ---> ADI::OFI Provider AV table : $arg"
      ;;
    direct-provider)
      do_direct_provider=true
      echo " ---> ADI::OFI Direct OFI Provider requested : $arg"
      ;;
    mr-offset)
      do_mr_offset=true
      echo " ---> ADI::OFI Provider Memory Registration: $arg"
      ;;
esac
done
IFS=$SAVE_IFS

AM_CONDITIONAL([BUILD_OFI],[test "$do_inline_ofi" = "true"])
AM_CONDITIONAL([BUILD_OFI_DYNAMIC],[test "$do_dynamic_adi" = "true"])
AM_CONDITIONAL([BUILD_OFI_SCALABLE_ENDPOINTS],[test "$do_scalable_endpoints" = "true"])
AM_CONDITIONAL([BUILD_OFI_AV_TABLE],[test "$do_av_table" = "true"])
AM_CONDITIONAL([BUILD_OFI_MR_OFFSET],[test "$do_mr_offset" = "true"])

AM_COND_IF([BUILD_OFI],[

])dnl end AM_COND_IF(BUILD_OPOBJ,MPIR_THREAD_POBJ_FI_MUTEX...)
])dnl end PREREQ

AC_DEFUN([PAC_SUBCFG_BODY_]PAC_SUBCFG_AUTO_SUFFIX,[

############################ INLINE ######################################

AM_COND_IF([BUILD_OFI],[
AC_MSG_NOTICE([RUNNING CONFIGURE FOR INLINE ADI::OFI DEVICE])

PAC_SET_HEADER_LIB_PATH(fabric)
PAC_CHECK_HEADER_LIB_FATAL(fabric, rdma/fabric.h, fabric, fi_strerror)

PAC_APPEND_FLAG([-I${master_top_srcdir}/src/mpid/adi/ofi/include], [CPPFLAGS])
if [test "$enable_sharedlibs" = "osx-gcc" ]; then
    PAC_APPEND_FLAG([-Wl,-rpath -Wl,${with_fabric}/lib], [LDFLAGS])
    PAC_APPEND_FLAG([-Wl,-rpath -Wl,${prefix}/lib], [LDFLAGS])
else
    PAC_APPEND_FLAG([-Wl,-rpath -Wl,${with_fabric}/lib -Wl,--enable-new-dtags], [LDFLAGS])
    PAC_APPEND_FLAG([-Wl,-rpath -Wl,${prefix}/lib -Wl,--enable-new-dtags], [LDFLAGS])
fi
PAC_APPEND_FLAG([-ldl -lpthread],[WRAPPER_LDFLAGS])
PAC_APPEND_FLAG([-DADI_DEVICE_OFI], [CPPFLAGS])

if [test "$do_direct_provider" = "true"]; then
   PAC_APPEND_FLAG([-DFABRIC_DIRECT],[CPPFLAGS])
fi

])dnl end AM_COND_IF(BUILD_OPOBJ,MPIR_THREAD_POBJ_FI_MUTEX...)

############################ DYNAMIC ######################################
AM_COND_IF([BUILD_OFI_DYNAMIC],[
AC_MSG_NOTICE([RUNNING CONFIGURE FOR DYNAMIC ADI::OFI DEVICE])

PAC_SET_HEADER_LIB_PATH(fabric)
PAC_CHECK_HEADER_LIB_FATAL(fabric, rdma/fabric.h, fabric, fi_strerror)
PAC_APPEND_FLAG([-I${master_top_srcdir}/src/mpid/adi/ofi/include], [CPPFLAGS])
if [test "$enable_sharedlibs" = "osx-gcc" ]; then
    PAC_APPEND_FLAG([-Wl,-rpath -Wl,${with_fabric}/lib], [LDFLAGS])
    PAC_APPEND_FLAG([-Wl,-rpath -Wl,${prefix}/lib], [LDFLAGS])
else
    PAC_APPEND_FLAG([-Wl,-rpath -Wl,${with_fabric}/lib -Wl,--enable-new-dtags], [LDFLAGS])
    PAC_APPEND_FLAG([-Wl,-rpath -Wl,${prefix}/lib -Wl,--enable-new-dtags], [LDFLAGS])
fi
PAC_APPEND_FLAG([-ldl -lpthread],[WRAPPER_LDFLAGS])
PAC_APPEND_FLAG([-DMPIDI_USE_DYNAMIC_ADI], [CPPFLAGS])

if [test "$do_direct_provider" = "true"]; then
   PAC_APPEND_FLAG([-DFABRIC_DIRECT],[CPPFLAGS])
fi


])dnl end AM_COND_IF(BUILD_OFI_DYNAMIC,...)

############################ SCALABLE ENDPOINTS ##########################
AM_COND_IF([BUILD_OFI_SCALABLE_ENDPOINTS],[
AC_MSG_NOTICE([RUNNING CONFIGURE FOR SCALABLE ENDPOINTS::OFI DEVICE])

PAC_APPEND_FLAG([-DMPIDI_USE_SCALABLE_ENDPOINTS], [CPPFLAGS])

])dnl end AM_COND_IF(BUILD_OFI_SCALABLE_ENDPOINTS,...)

############################ AV_TABLE ##########################
AM_COND_IF([BUILD_OFI_AV_TABLE],[
AC_MSG_NOTICE([RUNNING CONFIGURE FOR AV_TABLE::OFI DEVICE])

PAC_APPEND_FLAG([-DMPIDI_USE_AV_TABLE], [CPPFLAGS])

])dnl end AM_COND_IF(BUILD_OFI_AV_TABLE,...)

############################ MR OFFSET ##########################
AM_COND_IF([BUILD_OFI_MR_OFFSET],[
AC_MSG_NOTICE([RUNNING CONFIGURE FOR MR OFFSET::OFI DEVICE])

PAC_APPEND_FLAG([-DMPIDI_USE_MR_OFFSET], [CPPFLAGS])

])dnl end AM_COND_IF(BUILD_OFI_MR_OFFSET,...)

])dnl end _BODY

[#] end of __file__
