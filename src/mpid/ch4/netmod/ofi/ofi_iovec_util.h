/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2006 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Intel Corporation.
 *  Copyright (C) 2011-2016 Intel Corporation.  Intel provides this material
 *  to Argonne National Laboratory subject to Software Grant and Corporate
 *  Contributor License Agreement dated February 8, 2012.
 */
#ifndef OFI_IOVEC_UTIL_H_INCLUDED
#define OFI_IOVEC_UTIL_H_INCLUDED

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif /* HAVE_STDINT_H */
#ifdef HAVE_SYS_UIO_H
#include <sys/uio.h>
#endif /* HAVE_SYS_UIO_H */
#include <assert.h>
#include "ofi_types.h"

#define MPIDI_OFI_SEG_DONE     0
#define MPIDI_OFI_SEG_SUCCESS  0
#define MPIDI_OFI_SEG_EAGAIN   1
#define MPIDI_OFI_SEG_ERROR   -1

#define MPIDI_OFI_INIT_SEG_STATE(var, VAR)                              \
do {                                                                    \
    MPIDI_OFI_segment_next(iov_state,                                   \
                           &iov_state->var## _iov,                      \
                           MPIDI_OFI_SEGMENT_## VAR);                   \
    iov_state->var## _idx      = 0;                                       \
    iov_state->var## _addr    = (uintptr_t)iov_state->var## _iov.iov_base; \
    iov_state->var## _iov_len = (uintptr_t)iov_state->var## _iov.iov_len; \
    while (iov_state->var## _iov_len == 0) {                            \
        int done =                                                      \
        MPIDI_OFI_segment_next(iov_state,                               \
                               &iov_state->var## _iov,                  \
                               MPIDI_OFI_SEGMENT_## VAR);               \
        iov_state->var## _idx++;                                        \
        if (!done) {                                                    \
            iov_state->var## _addr = (uintptr_t)iov_state->var## _iov.iov_base; \
            iov_state->var## _iov_len = (uintptr_t)iov_state->var## _iov.iov_len;  \
        } else {                                                        \
            break;                                                      \
        }                                                               \
    }                                                                   \
} while (0)


#define MPIDI_OFI_NEXT_SEG_STATE(var,VAR)                               \
do {                                                                    \
    *var## _addr_next       = iov_state->var## _addr;                   \
    iov_state->var## _addr += buf_size;                                 \
    iov_state->var## _iov_len -= buf_size;                              \
    while (iov_state->var## _iov_len == 0) {                            \
        int done =                                                      \
        MPIDI_OFI_segment_next(iov_state,                               \
                               &iov_state->var## _iov,                  \
                               MPIDI_OFI_SEGMENT_## VAR);               \
        iov_state->var## _idx++;                                        \
        if (!done) {                                                    \
            iov_state->var## _addr    = (uintptr_t)iov_state->var## _iov.iov_base; \
            iov_state->var## _iov_len = (uintptr_t)iov_state->var## _iov.iov_len; \
        } else {                                                        \
            break;                                                      \
        }                                                               \
    }                                                                   \
} while (0)

#define MPIDI_OFI_INIT_SEG(var)                                         \
do {                                                                    \
    ((struct iovec*)(&var## _iov[0]))->iov_len  = last_len;                \
    ((struct iovec*)(&var## _iov[0]))->iov_base = (void*)var## _last_addr; \
    *var## _iovs_nout = 1;                          \
} while (0)

#define MPIDI_OFI_UPDATE_SEG(var,VAR)                                   \
do {                                                                    \
    MPIDI_OFI_segment_next(iov_state,                                   \
                           &iov_state->var## _iov,                      \
                           MPIDI_OFI_SEGMENT_## VAR);                   \
    var## _idx++;                                                       \
    ((struct iovec*)(&var## _iov[*var## _iovs_nout]))->iov_base = (void *)var## _addr; \
    ((struct iovec*)(&var## _iov[*var## _iovs_nout]))->iov_len  = len;  \
    (*var## _iovs_nout)++;                                              \
} while (0)

#define MPIDI_OFI_UPDATE_SEG_STATE1(var1,var2)                          \
do {                                                                    \
    if (*var2## _iovs_nout>=var2## _max_iovs) return MPIDI_OFI_SEG_EAGAIN; \
    ((struct iovec*)(&var1## _iov[var1## _idx]))->iov_len += len;       \
    ((struct iovec*)(&var2## _iov[*var2## _iovs_nout]))->iov_base = (void *)var2## _addr; \
    ((struct iovec*)(&var2## _iov[*var2## _iovs_nout]))->iov_len  = len; \
    (*var2## _iovs_nout)++;                                             \
    MPIDI_OFI_next_seg_state(iov_state,&origin_addr, &target_addr, &len); \
} while (0)

#define MPIDI_OFI_UPDATE_SEG_STATE2(var1,var2,var3)                          \
do {                                                                         \
    if (*var2## _iovs_nout>=var2## _max_iovs) return MPIDI_OFI_SEG_EAGAIN;   \
    if (*var3## _iovs_nout>=var3## _max_iovs) return MPIDI_OFI_SEG_EAGAIN;   \
    ((struct iovec*)(&var1## _iov[var1## _idx]))->iov_len += len;            \
    ((struct iovec*)(&var2## _iov[*var2## _iovs_nout]))->iov_base = (void *)var2## _addr; \
    ((struct iovec*)(&var2## _iov[*var2## _iovs_nout]))->iov_len  = len; \
    ((struct iovec*)(&var3## _iov[*var3## _iovs_nout]))->iov_base = (void *)var3## _addr; \
    ((struct iovec*)(&var3## _iov[*var3## _iovs_nout]))->iov_len  = len; \
    (*var2## _iovs_nout)++;                                             \
    (*var3## _iovs_nout)++;                                             \
    MPIDI_OFI_next_seg_state2(iov_state,&origin_addr, &result_addr, &target_addr,&len); \
  } while (0)


MPL_STATIC_INLINE_PREFIX
int MPIDI_OFI_segment_next(MPIDI_OFI_seg_state_t *state,
                           DLOOP_VECTOR                *out_vector,
                           MPIDI_OFI_Segment_side_t     side)
{
    DLOOP_VECTOR  dloop[2];
    DLOOP_Offset  last;
    MPID_Segment *seg;
    size_t       *cursor;
    int           num_contig = 1;

    switch (side) {
    case MPIDI_OFI_SEGMENT_ORIGIN:
        last   = state->origin_end;
        seg    = &state->origin_seg;
        cursor = &state->origin_cursor;
        break;
    case MPIDI_OFI_SEGMENT_TARGET:
        last   = state->target_end;
        seg    = &state->target_seg;
        cursor = &state->target_cursor;
        break;
    case MPIDI_OFI_SEGMENT_RESULT:
        break;
    default:
        MPIR_Assert(0);
        break;
    }
    if(*cursor >= last) return 1;
    MPIDU_Segment_pack_vector(seg,
                              *cursor,
                              &last,
                              &dloop[0],
                              &num_contig);
    MPIR_Assert(num_contig <= 1);
    *cursor     = last;
    *out_vector = dloop[0];
    return num_contig==0?1:0;
}

MPL_STATIC_INLINE_PREFIX
void MPIDI_OFI_init_seg_state(MPIDI_OFI_seg_state_t *iov_state,
                              const void    *origin,
                              const MPI_Aint target,
                              size_t         origin_count,
                              size_t         target_count,
                              size_t         buf_limit,
                              MPI_Datatype   origin_type,
                              MPI_Datatype   target_type)
{
    iov_state->buf_limit      = buf_limit;
    iov_state->buf_limit_left = buf_limit;

    iov_state->origin_cursor  = 0;
    MPIDI_Datatype_check_size(origin_type, origin_count, iov_state->origin_end);
    MPIDU_Segment_init(origin, origin_count, origin_type, &iov_state->origin_seg, 0);

    iov_state->target_cursor  = 0;
    MPIDI_Datatype_check_size(target_type, target_count,iov_state->target_end);
    MPIDU_Segment_init((const void*)target, target_count, target_type, &iov_state->target_seg, 0);

    MPIDI_OFI_INIT_SEG_STATE(target, TARGET);
    MPIDI_OFI_INIT_SEG_STATE(origin, ORIGIN);
}

MPL_STATIC_INLINE_PREFIX
void MPIDI_OFI_init_seg_state2(MPIDI_OFI_seg_state_t * iov_state,
                               const void *origin,
                               const void *result,
                               const MPI_Aint target,
                               size_t origin_count,
                               size_t result_count,
                               size_t target_count,
                               size_t buf_limit,
                               MPI_Datatype origin_type,
                               MPI_Datatype target_type,
                               MPI_Datatype result_type)
{
    iov_state->buf_limit = buf_limit;
    iov_state->buf_limit_left = buf_limit;

    iov_state->origin_cursor  = 0;
    MPIDI_Datatype_check_size(origin_type, origin_count, iov_state->origin_end);

    MPIDU_Segment_init(origin, origin_count, origin_type, &iov_state->origin_seg, 0);

    iov_state->target_cursor  = 0;
    MPIDI_Datatype_check_size(target_type, target_count,iov_state->target_end);
    MPIDU_Segment_init((const void*)target, target_count, target_type, &iov_state->target_seg, 0);

    iov_state->result_cursor  = 0;
    MPIDI_Datatype_check_size(result_type, result_count,iov_state->result_end);
    MPIDU_Segment_init((const void*)result, result_count, result_type, &iov_state->result_seg, 0);

    MPIDI_OFI_INIT_SEG_STATE(target,TARGET);
    MPIDI_OFI_INIT_SEG_STATE(origin,ORIGIN);
    MPIDI_OFI_INIT_SEG_STATE(result,RESULT);
}


MPL_STATIC_INLINE_PREFIX
int MPIDI_OFI_next_seg_state(MPIDI_OFI_seg_state_t * iov_state,
                             uintptr_t * origin_addr_next,
                             uintptr_t * target_addr_next,
                             size_t * buf_len)
{
    if ((iov_state->origin_iov_len != 0) && (iov_state->target_iov_len != 0)) {
        uintptr_t buf_size = MPL_MIN(MPL_MIN(iov_state->target_iov_len, iov_state->origin_iov_len),
                                     iov_state->buf_limit_left);
        *buf_len = buf_size;
        MPIDI_OFI_NEXT_SEG_STATE(target,TARGET);
        MPIDI_OFI_NEXT_SEG_STATE(origin,ORIGIN);
        return MPIDI_OFI_SEG_EAGAIN;
    }
    else {
        if (((iov_state->origin_iov_len != 0) || (iov_state->target_iov_len != 0)))
            return MPIDI_OFI_SEG_ERROR;
        return MPIDI_OFI_SEG_DONE;
    }
}

MPL_STATIC_INLINE_PREFIX
int MPIDI_OFI_next_seg_state2(MPIDI_OFI_seg_state_t * iov_state,
                              uintptr_t * origin_addr_next,
                              uintptr_t * result_addr_next,
                              uintptr_t * target_addr_next,
                              size_t    * buf_len)
{
    if ((iov_state->origin_iov_len != 0) && (iov_state->target_iov_len != 0) &&
        (iov_state->result_iov_len != 0)) {
        uintptr_t buf_size =
            MPL_MIN(MPL_MIN(MPL_MIN(iov_state->target_iov_len, iov_state->origin_iov_len),
                            iov_state->result_iov_len), iov_state->buf_limit_left);
        *buf_len = buf_size;
        MPIDI_OFI_NEXT_SEG_STATE(target,TARGET);
        MPIDI_OFI_NEXT_SEG_STATE(origin,ORIGIN);
        MPIDI_OFI_NEXT_SEG_STATE(result,RESULT);
        return MPIDI_OFI_SEG_EAGAIN;
    }
    else {
        if (((iov_state->origin_iov_len != 0) ||
             (iov_state->target_iov_len != 0) ||
             (iov_state->result_iov_len != 0)))
            return MPIDI_OFI_SEG_ERROR;

        return MPIDI_OFI_SEG_DONE;
    }
}


MPL_STATIC_INLINE_PREFIX
int MPIDI_OFI_peek_seg_state(MPIDI_OFI_seg_state_t * iov_state,
                             uintptr_t * next_origin_addr,
                             uintptr_t * next_target_addr,
                             size_t * buf_len)
{
    if ((iov_state->origin_iov_len != 0) && (iov_state->target_iov_len != 0)) {
        *next_origin_addr = iov_state->origin_addr;
        *next_target_addr = iov_state->target_addr;
        *buf_len =
            MPL_MIN(MPL_MIN(iov_state->target_iov_len,
                            iov_state->origin_iov_len),
                    iov_state->buf_limit_left);
        return MPIDI_OFI_SEG_EAGAIN;
    }
    else {
        if (((iov_state->origin_iov_len != 0) || (iov_state->target_iov_len != 0)))
            return MPIDI_OFI_SEG_ERROR;

        return MPIDI_OFI_SEG_DONE;
    }
}

MPL_STATIC_INLINE_PREFIX
int MPIDI_OFI_peek_seg_state2(MPIDI_OFI_seg_state_t * iov_state,
                              uintptr_t * next_origin_addr,
                              uintptr_t * next_result_addr,
                              uintptr_t * next_target_addr,
                              size_t * buf_len)
{
    if ((iov_state->origin_iov_len != 0) && (iov_state->target_iov_len != 0) &&
        (iov_state->result_iov_len != 0)) {
        *next_origin_addr = iov_state->origin_addr;
        *next_result_addr = iov_state->result_addr;
        *next_target_addr = iov_state->target_addr;
        *buf_len = MPL_MIN(MPL_MIN(MPL_MIN(iov_state->target_iov_len, iov_state->origin_iov_len),
                                   iov_state->result_iov_len), iov_state->buf_limit_left);
        return MPIDI_OFI_SEG_EAGAIN;
    }
    else {
        if (((iov_state->origin_iov_len != 0) ||
             (iov_state->target_iov_len != 0) ||
             (iov_state->result_iov_len != 0)))
            return MPIDI_OFI_SEG_ERROR;

        return MPIDI_OFI_SEG_DONE;
    }
}


MPL_STATIC_INLINE_PREFIX
int MPIDI_OFI_merge_segment(MPIDI_OFI_seg_state_t *iov_state,
                            struct iovec                *origin_iov,
                            size_t                       origin_max_iovs,
                            struct fi_rma_iov           *target_iov,
                            size_t                       target_max_iovs,
                            size_t                      *origin_iovs_nout,
                            size_t                      *target_iovs_nout)
{
    int rc;
    uintptr_t origin_addr = (uintptr_t) NULL, target_addr = (uintptr_t) NULL;
    uintptr_t origin_last_addr = 0, target_last_addr = 0;
    int origin_idx = 0, target_idx = 0;
    size_t len = 0, last_len = 0;

    CH4_COMPILE_TIME_ASSERT(offsetof(struct iovec, iov_base) == offsetof(struct fi_rma_iov, addr));
    CH4_COMPILE_TIME_ASSERT(offsetof(struct iovec, iov_len) == offsetof(struct fi_rma_iov, len));

    rc = MPIDI_OFI_next_seg_state(iov_state, &origin_last_addr, &target_last_addr, &last_len);
    assert(rc != MPIDI_OFI_SEG_ERROR);

    MPIDI_OFI_INIT_SEG(target);
    MPIDI_OFI_INIT_SEG(origin);
    iov_state->buf_limit_left -= last_len;

    while (rc > 0) {
        rc = MPIDI_OFI_peek_seg_state(iov_state, &origin_addr, &target_addr, &len);
        assert(rc != MPIDI_OFI_SEG_ERROR);

        if (rc == MPIDI_OFI_SEG_DONE) {
            iov_state->buf_limit_left = iov_state->buf_limit;
            return MPIDI_OFI_SEG_EAGAIN;
        }

        if (target_last_addr + last_len == target_addr) {
            MPIDI_OFI_UPDATE_SEG_STATE1(target, origin);
        }
        else if (origin_last_addr + last_len == origin_addr) {
            MPIDI_OFI_UPDATE_SEG_STATE1(origin, target);
        }
        else {
            if ((*origin_iovs_nout >= origin_max_iovs) || (*target_iovs_nout >= target_max_iovs)) {
                iov_state->buf_limit_left = iov_state->buf_limit;
                return MPIDI_OFI_SEG_EAGAIN;
            }

            MPIDI_OFI_UPDATE_SEG(target,TARGET);
            MPIDI_OFI_UPDATE_SEG(origin,ORIGIN);
            MPIDI_OFI_next_seg_state(iov_state, &origin_addr, &target_addr, &len);
        }

        origin_last_addr = origin_addr;
        target_last_addr = target_addr;
        last_len = len;
        iov_state->buf_limit_left -= len;
        if (iov_state->buf_limit_left == 0) {
            iov_state->buf_limit_left = iov_state->buf_limit;
            return MPIDI_OFI_SEG_EAGAIN;
        }
    }

    if (rc == MPIDI_OFI_SEG_DONE)
        return MPIDI_OFI_SEG_DONE;
    else {
        iov_state->buf_limit_left = iov_state->buf_limit;
        return MPIDI_OFI_SEG_EAGAIN;
    }
}


MPL_STATIC_INLINE_PREFIX
int MPIDI_OFI_merge_segment2(MPIDI_OFI_seg_state_t * iov_state,
                             struct iovec *origin_iov,
                             size_t origin_max_iovs,
                             struct iovec *result_iov,
                             size_t result_max_iovs,
                             struct fi_rma_iov *target_iov,
                             size_t target_max_iovs,
                             size_t * origin_iovs_nout,
                             size_t * result_iovs_nout,
                             size_t * target_iovs_nout)
{
    int rc;
    uintptr_t origin_addr = (uintptr_t) NULL, result_addr = (uintptr_t) NULL, target_addr =
        (uintptr_t) NULL;
    uintptr_t origin_last_addr = 0, result_last_addr = 0, target_last_addr = 0;
    int origin_idx = 0, result_idx = 0, target_idx = 0;
    size_t len = 0, last_len = 0;

    rc = MPIDI_OFI_next_seg_state2(iov_state, &origin_last_addr, &result_last_addr,
                                   &target_last_addr, &last_len);
    assert(rc != MPIDI_OFI_SEG_ERROR);
    MPIDI_OFI_INIT_SEG(target);
    MPIDI_OFI_INIT_SEG(origin);
    MPIDI_OFI_INIT_SEG(result);
    iov_state->buf_limit_left -= last_len;

    while (rc > 0) {
        rc = MPIDI_OFI_peek_seg_state2(iov_state, &origin_addr, &result_addr, &target_addr, &len);
        assert(rc != MPIDI_OFI_SEG_ERROR);

        if (rc == MPIDI_OFI_SEG_DONE) {
            iov_state->buf_limit_left = iov_state->buf_limit;
            return MPIDI_OFI_SEG_EAGAIN;
        }

        if (target_last_addr + last_len == target_addr) {
            MPIDI_OFI_UPDATE_SEG_STATE2(target, origin, result);
        }
        else if (origin_last_addr + last_len == origin_addr) {
            MPIDI_OFI_UPDATE_SEG_STATE2(origin, target, result);
        }
        else if (result_last_addr + last_len == result_addr) {
            MPIDI_OFI_UPDATE_SEG_STATE2(result, target, origin);
        }
        else {
            if ((*origin_iovs_nout >= origin_max_iovs) ||
                (*target_iovs_nout >= target_max_iovs) ||
                (*result_iovs_nout >= result_max_iovs)) {
                iov_state->buf_limit_left = iov_state->buf_limit;
                return MPIDI_OFI_SEG_EAGAIN;
            }

            MPIDI_OFI_UPDATE_SEG(target,TARGET);
            MPIDI_OFI_UPDATE_SEG(origin,ORIGIN);
            MPIDI_OFI_UPDATE_SEG(result,RESULT);
            MPIDI_OFI_next_seg_state2(iov_state, &origin_addr, &result_addr, &target_addr, &len);
        }

        origin_last_addr = origin_addr;
        result_last_addr = result_addr;
        target_last_addr = target_addr;
        last_len = len;
        iov_state->buf_limit_left -= len;
        if (iov_state->buf_limit_left == 0) {
            iov_state->buf_limit_left = iov_state->buf_limit;
            return MPIDI_OFI_SEG_EAGAIN;
        }

    }

    if (rc == MPIDI_OFI_SEG_DONE)
        return MPIDI_OFI_SEG_DONE;
    else {
        iov_state->buf_limit_left = iov_state->buf_limit;
        return MPIDI_OFI_SEG_EAGAIN;
    }
}

#endif /* OFI_IOVEC_UTIL_H_INCLUDED */
