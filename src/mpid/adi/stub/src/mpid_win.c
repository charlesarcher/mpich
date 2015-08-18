/*
 *  (C) 2006 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Intel Corporation.
 *  Copyright (C) 2011-2012 Intel Corporation.  Intel provides this material
 *  to Argonne National Laboratory subject to Software Grant and Corporate
 *  Contributor License Agreement dated February 8, 2012.
 */
#include <mpidimpl.h>

__ADI_INLINE__ int MPIDI_Win_set_info(MPIR_Win     *win,
                                      MPIR_Info    *info)
{
  assert(0);
  return MPI_SUCCESS;
}

__ADI_INLINE__ int MPIDI_Put(const void   *origin_addr,
                             int           origin_count,
                             MPI_Datatype  origin_datatype,
                             int           target_rank,
                             MPI_Aint      target_disp,
                             int           target_count,
                             MPI_Datatype  target_datatype,
                             MPIR_Win     *win)
{
  assert(0);
  return MPI_SUCCESS;
}

__ADI_INLINE__ int MPIDI_Win_start(MPIR_Group *group,
                                   int         assert,
                                   MPIR_Win   *win)
{
  assert(0);
  return MPI_SUCCESS;
}


__ADI_INLINE__ int MPIDI_Win_complete(MPIR_Win *win)
{
  assert(0);
  return MPI_SUCCESS;
}

__ADI_INLINE__ int MPIDI_Win_post(MPIR_Group *group,
                                  int         assert,
                                  MPIR_Win   *win)
{
  assert(0);
  return MPI_SUCCESS;
}


__ADI_INLINE__ int MPIDI_Win_wait(MPIR_Win *win)
{
  assert(0);
  return MPI_SUCCESS;
}


__ADI_INLINE__ int MPIDI_Win_test(MPIR_Win *win,
                                  int      *flag)
{
  assert(0);
  return MPI_SUCCESS;
}

__ADI_INLINE__ int MPIDI_Win_lock(int       lock_type,
                                  int       rank,
                                  int       assert,
                                  MPIR_Win *win)
{
  assert(0);
  return MPI_SUCCESS;
}


__ADI_INLINE__ int MPIDI_Win_unlock(int       rank,
                                    MPIR_Win *win)
{
  assert(0);
  return MPI_SUCCESS;
}

__ADI_INLINE__ int MPIDI_Win_get_info(MPIR_Win     *win,
                                      MPIR_Info   **info_p_p)
{
  assert(0);
  return MPI_SUCCESS;
}

__ADI_INLINE__ int MPIDI_Get(void         *origin_addr,
                             int           origin_count,
                             MPI_Datatype  origin_datatype,
                             int           target_rank,
                             MPI_Aint      target_disp,
                             int           target_count,
                             MPI_Datatype  target_datatype,
                             MPIR_Win     *win)
{
  assert(0);
  return MPI_SUCCESS;
}

__ADI_INLINE__ int MPIDI_Win_free(MPIR_Win **win_ptr)
{
  assert(0);
  return MPI_SUCCESS;
}

__ADI_INLINE__ int MPIDI_Win_fence(int       assert,
                                   MPIR_Win *win)
{
  assert(0);
  return MPI_SUCCESS;
}

__ADI_INLINE__ int MPIDI_Win_create(void        *base,
                                    MPI_Aint     length,
                                    int          disp_unit,
                                    MPIR_Info   *info,
                                    MPIR_Comm   *comm_ptr,
                                    MPIR_Win   **win_ptr)
{
  assert(0);
  return MPI_SUCCESS;
}

__ADI_INLINE__ int MPIDI_Accumulate(const void   *origin_addr,
                                    int           origin_count,
                                    MPI_Datatype  origin_datatype,
                                    int           target_rank,
                                    MPI_Aint      target_disp,
                                    int           target_count,
                                    MPI_Datatype  target_datatype,
                                    MPI_Op        op,
                                    MPIR_Win     *win)
{
  assert(0);
  return MPI_SUCCESS;
}

__ADI_INLINE__ int MPIDI_Win_attach(MPIR_Win *win,
                                    void     *base,
                                    MPI_Aint  size)
{
  assert(0);
  return MPI_SUCCESS;
}

__ADI_INLINE__ int MPIDI_Win_allocate_shared(MPI_Aint     size,
                                             int          disp_unit,
                                             MPIR_Info   *info_ptr,
                                             MPIR_Comm   *comm_ptr,
                                             void       **base_ptr,
                                             MPIR_Win   **win_ptr)
{
  assert(0);
  return MPI_SUCCESS;
}

__ADI_INLINE__ int MPIDI_Rput(const void    *origin_addr,
                              int            origin_count,
                              MPI_Datatype   origin_datatype,
                              int            target_rank,
                              MPI_Aint       target_disp,
                              int            target_count,
                              MPI_Datatype   target_datatype,
                              MPIR_Win      *win,
                              MPIR_Request **request)
{
  assert(0);
  return MPI_SUCCESS;
}

__ADI_INLINE__ int MPIDI_Win_flush_local(int       rank,
                                         MPIR_Win *win)
{
  assert(0);
  return MPI_SUCCESS;
}

__ADI_INLINE__ int MPIDI_Win_detach(MPIR_Win   *win,
                                    const void *base)
{
  assert(0);
  return MPI_SUCCESS;
}

__ADI_INLINE__ int MPIDI_Compare_and_swap(const void   *origin_addr,
                                          const void   *compare_addr,
                                          void         *result_addr,
                                          MPI_Datatype  datatype,
                                          int           target_rank,
                                          MPI_Aint      target_disp,
                                          MPIR_Win     *win)
{
  assert(0);
  return MPI_SUCCESS;
}

__ADI_INLINE__ int MPIDI_Raccumulate(const void    *origin_addr,
                                     int            origin_count,
                                     MPI_Datatype   origin_datatype,
                                     int            target_rank,
                                     MPI_Aint       target_disp,
                                     int            target_count,
                                     MPI_Datatype   target_datatype,
                                     MPI_Op         op,
                                     MPIR_Win      *win,
                                     MPIR_Request **request)
{
  assert(0);
  return MPI_SUCCESS;
}

__ADI_INLINE__ int MPIDI_Rget_accumulate(const void    *origin_addr,
                                         int            origin_count,
                                         MPI_Datatype   origin_datatype,
                                         void          *result_addr,
                                         int            result_count,
                                         MPI_Datatype   result_datatype,
                                         int            target_rank,
                                         MPI_Aint       target_disp,
                                         int            target_count,
                                         MPI_Datatype   target_datatype,
                                         MPI_Op         op,
                                         MPIR_Win      *win,
                                         MPIR_Request **request)
{
  assert(0);
  return MPI_SUCCESS;
}

__ADI_INLINE__ int MPIDI_Fetch_and_op(const void   *origin_addr,
                                      void         *result_addr,
                                      MPI_Datatype  datatype,
                                      int           target_rank,
                                      MPI_Aint      target_disp,
                                      MPI_Op        op,
                                      MPIR_Win     *win)
{
  assert(0);
  return MPI_SUCCESS;
}

__ADI_INLINE__ int MPIDI_Win_shared_query(MPIR_Win *win,
                                          int       rank,
                                          MPI_Aint *size,
                                          int      *disp_unit,
                                          void     *baseptr)
{
  assert(0);
  return MPI_SUCCESS;
}

__ADI_INLINE__ int MPIDI_Win_allocate(MPI_Aint    size,
                                      int         disp_unit,
                                      MPIR_Info  *info,
                                      MPIR_Comm  *comm,
                                      void       *baseptr,
                                      MPIR_Win  **win)
{
  assert(0);
  return MPI_SUCCESS;
}

__ADI_INLINE__ int MPIDI_Win_flush(int       rank,
                                   MPIR_Win *win)
{
  assert(0);
  return MPI_SUCCESS;
}

__ADI_INLINE__ int MPIDI_Win_flush_local_all(MPIR_Win *win)
{
  assert(0);
  return MPI_SUCCESS;
}

__ADI_INLINE__ int MPIDI_Win_unlock_all(MPIR_Win *win)
{
  assert(0);
  return MPI_SUCCESS;
}

__ADI_INLINE__ int MPIDI_Win_create_dynamic(MPIR_Info  *info,
                                            MPIR_Comm  *comm,
                                            MPIR_Win   **win)
{
  assert(0);
  return MPI_SUCCESS;
}

__ADI_INLINE__ int MPIDI_Rget(void          *origin_addr,
                              int            origin_count,
                              MPI_Datatype   origin_datatype,
                              int            target_rank,
                              MPI_Aint       target_disp,
                              int            target_count,
                              MPI_Datatype   target_datatype,
                              MPIR_Win      *win,
                              MPIR_Request **request)
{
  assert(0);
  return MPI_SUCCESS;
}

__ADI_INLINE__ int MPIDI_Win_sync(MPIR_Win *win)
{
  assert(0);
  return MPI_SUCCESS;
}

__ADI_INLINE__ int MPIDI_Win_flush_all(MPIR_Win *win)
{
  assert(0);
  return MPI_SUCCESS;
}

__ADI_INLINE__ int MPIDI_Get_accumulate(const void   *origin_addr,
                                        int           origin_count,
                                        MPI_Datatype  origin_datatype,
                                        void         *result_addr,
                                        int           result_count,
                                        MPI_Datatype  result_datatype,
                                        int           target_rank,
                                        MPI_Aint      target_disp,
                                        int           target_count,
                                        MPI_Datatype  target_datatype,
                                        MPI_Op        op,
                                        MPIR_Win     *win)
{
  assert(0);
  return MPI_SUCCESS;
}

__ADI_INLINE__ int MPIDI_Win_lock_all(int assert, MPIR_Win *win)
{
  assert(0);
  return MPI_SUCCESS;
}
