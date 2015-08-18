[#] start of __file__

dnl _PREREQ handles the former role of mpichprereq, setup_device, etc
[#] expansion is: PAC_SUBCFG_PREREQ_[]PAC_SUBCFG_AUTO_SUFFIX
AC_DEFUN([PAC_SUBCFG_PREREQ_]PAC_SUBCFG_AUTO_SUFFIX,[

dnl Parse the device arguments
SAVE_IFS=$IFS
IFS=':'
args_array=$device_args
do_coll_dynamic=true
do_inline_collstub=false
do_inline_colldefault=false
do_inline_colltree=false
do_inline_tspmpich=false
did_inline_tsp=0
did_inline_template=0
for arg in $args_array; do
  case ${arg} in
    inline-collstub)
      do_inline_collstub=true
      do_coll_dynamic=false
      did_inline_template=`expr $did_inline_template + 1`
      echo " ---> ADI::OFI::COLL : Inlined Collective Stub Library"
      ;;
    inline-colltree)
      do_inline_colltree=true
      do_coll_dynamic=false
      did_inline_template=`expr $did_inline_template + 1`
      echo " ---> ADI::OFI::COLL : Inlined Collective Tree Library"
      ;;
    inline-colldefault)
      do_inline_colldefault=true
      do_coll_dynamic=false
      did_inline_template=`expr $did_inline_template + 1`
      echo " ---> ADI::OFI::COLL : Inlined Collective Default Library"
      ;;
    inline-tspmpich)
      do_inline_tspmpich=true
      do_coll_dynamic=false
      did_inline_tsp=`expr $did_inline_tsp + 1`
      echo " ---> ADI::OFI::COLL : Inlined Collective Transport via MPICH Library"
      ;;
  esac
done
IFS=$SAVE_IFS

if [test "$do_coll_dynamic" = "false" ]; then
   if [test $did_inline_tsp -ne 1 -o $did_inline_template -ne 1 ]; then
      AC_ERROR([To inline collectives, pick one transport and one template])
   fi
fi



])dnl end PREREQ

AC_DEFUN([PAC_SUBCFG_BODY_]PAC_SUBCFG_AUTO_SUFFIX,[

])dnl end _BODY

[#] end of __file__
