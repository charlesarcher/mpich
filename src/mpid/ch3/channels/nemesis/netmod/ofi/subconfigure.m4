[#] start of __file__
dnl MPICH_SUBCFG_AFTER=src/mpid/ch3/channels/nemesis

AC_DEFUN([PAC_SUBCFG_PREREQ_]PAC_SUBCFG_AUTO_SUFFIX,[
    AM_COND_IF([BUILD_CH3_NEMESIS],[
        for net in $nemesis_networks ; do
            AS_CASE([$net],[ofi],[build_nemesis_netmod_ofi=yes])
        done
    ])
    AM_CONDITIONAL([BUILD_NEMESIS_NETMOD_OFI],[test "X$build_nemesis_netmod_ofi" = "Xyes"])
])dnl

AC_DEFUN([PAC_SUBCFG_BODY_]PAC_SUBCFG_AUTO_SUFFIX,[
AM_COND_IF([BUILD_NEMESIS_NETMOD_OFI],[
    AC_MSG_NOTICE([RUNNING CONFIGURE FOR ch3:nemesis:ofi])

    ofisrcdir=""
    AC_SUBST([ofisrcdir])
    ofilib=""
    AC_SUBST([ofilib])

    PAC_SET_HEADER_LIB_PATH(libfabric)
    PAC_PUSH_FLAG(LIBS)
    PAC_CHECK_HEADER_LIB([rdma/fabric.h], [fabric], [fi_getinfo], [have_libfabric=yes], [have_libfabric=no])
    PAC_POP_FLAG(LIBS)
    if test "${have_libfabric}" = "yes" ; then
        AC_MSG_NOTICE([CH3 OFI Netmod:  Using an external libfabric])
        PAC_APPEND_FLAG([-lfabric],[WRAPPER_LIBS])
    elif test ! -z "${with_libfabric}" ; then
        AC_MSG_ERROR([Provided libfabric installation (--with-libfabric=${with_libfabric}) could not be configured.])
    else
        # fallback to embedded libfabric
        AC_MSG_NOTICE([CH3 OFI Netmod:  Using an embedded libfabric])
        PAC_CONFIG_SUBDIR_ARGS([src/mpid/ch4/netmod/ofi/libfabric],[],[],[AC_MSG_ERROR(libfabric configure failed)])
        PAC_APPEND_FLAG([-I${master_top_builddir}/src/mpid/ch4/netmod/ofi/libfabric/include], [CPPFLAGS])
        PAC_APPEND_FLAG([-I${use_top_srcdir}/src/mpid/ch4/netmod/ofi/libfabric/include], [CPPFLAGS])

        ofisrcdir="${master_top_builddir}/src/mpid/ch4/netmod/ofi/libfabric"
        ofilib="src/mpid/ch4/netmod/ofi/libfabric/src/libfabric.la"
    fi
    PAC_APPEND_FLAG([-lstdc++ -ldl -lrt],[WRAPPER_LIBS])

    AC_DEFINE([ENABLE_COMM_OVERRIDES], 1, [define to add per-vc function pointers to override send and recv functions])
])dnl end AM_COND_IF(BUILD_NEMESIS_NETMOD_OFI,...)
])dnl end _BODY

[#] end of __file__
