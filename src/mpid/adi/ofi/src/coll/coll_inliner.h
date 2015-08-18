/*
 *  (C) 2006 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Intel Corporation.
 *  Copyright (C) 2011-2012 Intel Corporation.  Intel provides this material
 *  to Argonne National Laboratory subject to Software Grant and Corporate
 *  Contributor License Agreement dated February 8, 2012.
 */
#ifndef COLL_INLINER_H_INCLUDED
#define COLL_INLINER_H_INCLUDED

#ifdef USE_TSPMPICH
#include "transports/mpich/transport.c"
#endif

#ifdef USE_COLLSTUB
#include "templates/stub/coll.c"
#endif

#ifdef USE_COLLTREE
#include "templates/tree/coll.c"
#endif

#ifdef USE_COLLDEFAULT
#error "Coll Default is not inlineable.  Use dynamic collectives libarary"
#endif

#ifdef USE_DYNAMIC_COLL
#error "Fatal : Cannot use the dynamic collectives loader with the inliner"
#endif

#endif /* COLL_INLINER_H_INCLUDED */
