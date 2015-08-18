[#] start of __file__

dnl _PREREQ handles the former role of mpichprereq, setup_device, etc
[#] expansion is: PAC_SUBCFG_PREREQ_[]PAC_SUBCFG_AUTO_SUFFIX
AC_DEFUN([PAC_SUBCFG_PREREQ_]PAC_SUBCFG_AUTO_SUFFIX,[

AM_CONDITIONAL([BUILD_STUB],[test "$do_inline_stub" = "true"])
AM_CONDITIONAL([BUILD_STUB_DYNAMIC],[test "$do_dynamic_adi" = "true"])

AM_COND_IF([BUILD_STUB],[


])dnl end AM_COND_IF(BUILD_STUB,...)
])dnl end PREREQ

AC_DEFUN([PAC_SUBCFG_BODY_]PAC_SUBCFG_AUTO_SUFFIX,[

############################ INLINE ######################################

AM_COND_IF([BUILD_STUB],[
AC_MSG_NOTICE([RUNNING CONFIGURE FOR INLINE ADI::STUB DEVICE])
PAC_APPEND_FLAG([-DADI_DEVICE_STUB], [CPPFLAGS])

])dnl end AM_COND_IF(BUILD_STUB,...)

############################ DYNAMIC ######################################
AM_COND_IF([BUILD_STUB_DYNAMIC],[
AC_MSG_NOTICE([RUNNING CONFIGURE FOR DYNAMIC ADI::STUB DEVICE])
PAC_APPEND_FLAG([-DMPIDI_USE_DYNAMIC_ADI], [CPPFLAGS])

])dnl end AM_COND_IF(BUILD_STUB_DYNAMIC,...)


])dnl end _BODY

[#] end of __file__
