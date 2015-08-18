/*
 *  (C) 2006 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Intel Corporation.
 *  Copyright (C) 2011-2012 Intel Corporation.  Intel provides this material
 *  to Argonne National Laboratory subject to Software Grant and Corporate
 *  Contributor License Agreement dated February 8, 2012.
 */
#if !defined(MPICH_MPIDIMPL_H_INCLUDED)
#define MPICH_MPIDIMPL_H_INCLUDED

#ifdef __cplusplus
#define EXTERN_C_BEGIN extern "C" {
#define EXTERN_C_END }
#else
#define EXTERN_C_BEGIN
#define EXTERN_C_END
#endif


#include "mpichconf.h"

#include <stdio.h>
#define DPRINT(x) fprintf x
/*#define DPRINT(x) */


#define __SHORT_FILE__                          \
  (strrchr(__FILE__,'/')                        \
   ? strrchr(__FILE__,'/')+1                    \
   : __FILE__                                   \
   )

#define BEGIN_FUNC(FUNCNAME)  MPIR_FUNC_VERBOSE_STATE_DECL(FUNCNAME);       \
                              MPIR_FUNC_TERSE_ENTER(FUNCNAME);
#define END_FUNC(FUNCNAME)    MPIR_FUNC_TERSE_EXIT(FUNCNAME);
#define DECL_FUNC(FUNCNAME)   MPL_QUOTE(FUNCNAME)


#if defined(HAVE_ASSERT_H)
#include <assert.h>
#endif

EXTERN_C_BEGIN
#define MPICH_SKIP_MPICXX
#include "mpiimpl.h"
#include "mpidu_sched.h"

EXTERN_C_END


#if !defined(MPICH_MPIDPRE_H_INCLUDED)
#include "mpidpre.h"
#endif

#include <mpidfunc.h>

#endif /* !defined(MPICH_MPIDIMPL_H_INCLUDED) */
