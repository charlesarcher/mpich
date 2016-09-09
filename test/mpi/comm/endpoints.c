/* Copyright (c) 2014, Intel Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* Simple endpoints test program
 * Author: James Dinan
 * Date: November, 2014
 *
 * This program includes simple implementation of the proposed endpoints
 * extension to MPI and includes several tests and example of API usage.
 *
 * TODO: Test group comparison
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <mpi.h>
#include "mpitest.h"

static int errs = 0;
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

void * ep_test(void *comm_ptr)
{
    MPI_Request req[2];
    MPI_Status  status[2];
    int in, out, me, nproc;
    MPI_Comm comm = * (MPI_Comm *) comm_ptr;

    MPI_Comm_rank(comm, &me);
    MPI_Comm_size(comm, &nproc);

    /* Test some collectives */
    MPI_Barrier(comm);

    /* Test send/recv with neighbor exchange */
    out = me;
    in  = -1;
    MPI_Irecv(&in, 1, MPI_INT, (me+nproc-1) % nproc, 0, comm, &req[0]);
    MPI_Isend(&out, 1, MPI_INT, (me+1) % nproc, 0, comm, &req[1]);
    MPI_Waitall(2, req, status);
    if (in != (me+nproc-1) % nproc || in != status[0].MPI_SOURCE) {
        printf("%2d -- neighbor exchange received %d, expected %d SOURCE=%d\n", me, in, (me+nproc-1) % nproc, status[0].MPI_SOURCE);
        pthread_mutex_lock(&mtx);
        errs++;
        pthread_mutex_unlock(&mtx);
    }

    /* Done: test freeing comm */
    MPI_Comm_free(&comm);

    return NULL;
}


int main(int argc, char **argv)
{
    int tl, me, nproc, result, err, err_class;
    int num_ep = -1, test_rank, test_size;
    MPI_Comm *ep_comm;

    MTest_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &tl);
    if (tl != MPI_THREAD_MULTIPLE) {
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    MPI_Comm_rank(MPI_COMM_WORLD, &me);
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);

    if (me == 0) {
        if (argc < 2) {
            printf("Usage: %s num_ep\n", argv[0]);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        num_ep = atoi(argv[1]);
    }

    MPI_Bcast(&num_ep, 1, MPI_INT, 0, MPI_COMM_WORLD);
    ep_comm = (MPI_Comm *) malloc(sizeof(MPI_Comm) * num_ep);

    MPI_Comm_set_errhandler(MPI_COMM_WORLD, MPI_ERRORS_RETURN);

    /* Test err arg */
    err = MPI_Comm_create_endpoints(MPI_COMM_WORLD, -1, MPI_INFO_NULL, ep_comm);
    MPI_Error_class(err, &err_class);
    if (err_class != MPI_ERR_ARG) {
        printf("%2d -- MPI_ERR_ARG test failed got class %d, expected %d\n", me, err_class, MPI_ERR_ARG);
        errs++;
    }

    /* Test dup */
    err = MPI_Comm_create_endpoints(MPI_COMM_WORLD, 1, MPI_INFO_NULL, ep_comm);
    if (err != MPI_SUCCESS) {
        printf("%2d -- Dup test failed to create EP comm\n", me);
        errs++;
    }

    MPI_Comm_compare(MPI_COMM_WORLD, ep_comm[0], &result);
    if (result != MPI_CONGRUENT) {
        printf("%2d -- Error: dup test did not result in MPI_CONGRUENT\n", me);
        errs++;
    }

    MPI_Comm_free(ep_comm);

    /* Test subset */
    err = MPI_Comm_create_endpoints(MPI_COMM_WORLD, me % 2 == 0, MPI_INFO_NULL, me % 2 == 0 ? ep_comm : NULL);
    if (err != MPI_SUCCESS) {
        printf("%2d -- Subset test failed to create EP comm\n", me);
        errs++;
    }

    if (me % 2 == 0) {
        MPI_Comm_rank(ep_comm[0], &test_rank);
        MPI_Comm_size(ep_comm[0], &test_size);
        if (test_rank != me / 2) {
            printf("%2d -- Error: subset test rank was %d, expected %d\n", me, test_rank, me / 2);
            errs++;
        }
        if (test_size != (nproc + 1) / 2) {
            printf("%2d -- Error: subset test size was %d, expected %d\n", me, test_size, (nproc + 1) / 2);
            errs++;
        }
        MPI_Comm_free(ep_comm);
    }

    /* Test multi-endpoint communicator creation */
    err = MPI_Comm_create_endpoints(MPI_COMM_WORLD, num_ep, MPI_INFO_NULL, ep_comm);
    MPI_Error_class(err, &err_class);

    {
        int was_err_ep = 0, err_len;
        char err_str[MPI_MAX_ERROR_STRING];

        if (err_class == MPI_ERR_ENDPOINTS) {
            was_err_ep = 1;
        }

        MPI_Allreduce(MPI_IN_PLACE, &was_err_ep, 1, MPI_INT, MPI_LOR, MPI_COMM_WORLD);

        if (was_err_ep) {
            /* We assume that the MPI implementation can continue after
             * MPI_ERR_ENDPOINTS, although this is not required by the
             * standard. */
        }
        else if (err != MPI_SUCCESS) {
            MPI_Error_string(err, err_str, &err_len);
            printf("%2d -- Multi-EP test failed to create EP comm or return MPI_ERR_ENDPOINTS (%s)\n", me, err_str);
            errs++;
        }
        else {
            /* Back each endpoint by a thread and run the ep_test() */
            if (err_class == MPI_SUCCESS) {
                int i;
                pthread_t pthreads[num_ep];
                for (i = 0; i < num_ep; i++) {
                    pthread_create(&pthreads[i], NULL, &ep_test, &ep_comm[i]);
                }
                for (i = 0; i < num_ep; i++) {
                    pthread_join(pthreads[i], NULL);
                }
            }
        }
    }

    MTest_Finalize(errs);

    return 0;
}
