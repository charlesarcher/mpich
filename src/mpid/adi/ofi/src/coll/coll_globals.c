#ifndef USE_DYNAMIC_COLL

#ifdef USE_COLLTREE
#include "templates/tree/coll_globals.c"
#endif

#ifdef USE_COLLDEFAULT
#error "Coll Default is not inlineable.  Use dynamic collectives libarary"
#endif

#ifdef USE_DYNAMIC_COLL
#error "Fatal : Cannot use the dynamic collectives loader with the inliner"
#endif


#endif /* USE_DYNAMIC_COLL */
