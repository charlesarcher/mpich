/*
 *  (C) 2006 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Intel Corporation.
 *  Copyright (C) 2011-2012 Intel Corporation.  Intel provides this material
 *  to Argonne National Laboratory subject to Software Grant and Corporate
 *  Contributor License Agreement dated February 8, 2012.
 */
#ifndef MPICHTRANSPORT_H_INCLUDED
#define MPICHTRANSPORT_H_INCLUDED

#include <mpidimpl.h>
#include "../../../mpid_types.h"
#include "../../coll_loader.h"

#ifdef USE_DYNAMIC_COLL
#define __CSI__
#else
#define __CSI__ __attribute__((always_inline)) static inline
#endif


#endif
