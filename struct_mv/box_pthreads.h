
/* cliches.h

 m4 definitions.
*/



/* ================================================================
 Self-scheduled parallelism macros.
*/

/*
Usage:  BARRIER(number_of_threads)
Argument:  $1 -- integer containing the number of threads being used
*/



/*
Usage:  FBARRIER(number_of_threads)
Argument:  $1 -- integer containing the number of threads being used
*/



/* PLOOPEND waits until the last proc. resets the loop -1 to zero.
Usage:  PLOOPEND(index_array_name, array_index)
Arguments:  $1 -- the name of the array of indices
            $2 -- an -1 of the given array
To wait until element n of array "indx" is set to zero, enter 
   PLOOPEND(indx,n)
*/



/* IWAIT is used to examine values of the global flag ipencil in calchyd
 If there is only one thread, this is a noop.
Usage:  IWAIT(flag_array_name, array_index)
Arguments:  $1 -- the name of an array of flags
            $2 -- an -1 of the given array
*/



/* PLOOP parallel loop macro.
 Example:
     PLOOP(z,lz,mz,indx,3,body)
 The indx used ($5) must not be reused for a loop
 until a synch. point. guarantees all threads have exited.

Usage: PLOOP(increment_variable, loop_initial_value, loop_stopping value,
             index_array_name, array_index, thread_counter, mutex_object,
             cond_object loop_body)

NUM_THREADS must either be #defined as the number of threads being used or
be an integer variable containing the number of threads.

Arguments:  $1 -- the name of the increment variable for the loop
            $2 -- the initial value for the increment variable
            $3 -- the stopping value for the increment variable
                  (loop will not be entered when increment reaches this value)
            $4 -- the name of an array of indices
            $6 -- an integer counter to count each thread as it passes
            $7 -- a pthread_mutex_t object (must be initialized
                  externally)
            $8 -- a pthread_cond_t object (must be initialized externally)
            $9 -- The body of the loop (enclose between  and  )
*/







/*BHEADER**********************************************************************
 * (c) 1997   The Regents of the University of California
 *
 * See the file COPYRIGHT_and_DISCLAIMER for a complete copyright
 * notice, contact person, and disclaimer.
 *
 * $Revision$
 *********************************************************************EHEADER*/
/******************************************************************************
 *
 * Header info for the Box structures
 *
 *****************************************************************************/

#ifndef hypre_BOX_PTHREADS_HEADER
#define hypre_BOX_PTHREADS_HEADER

#ifdef HYPRE_USE_PTHREADS

#include <pthread.h>
#include "threading.h"


extern int hypre_thread_counter;
extern int iteration_counter;

/*--------------------------------------------------------------------------
 * Threaded Looping macros:
 *--------------------------------------------------------------------------*/

#ifndef NUM_THREADS
#define NUM_THREADS 4
#endif
#ifndef MAX_ISIZE
#define MAX_ISIZE 5
#endif
#ifndef MAX_JSIZE
#define MAX_JSIZE 5
#endif
#ifndef MAX_KSIZE
#define MAX_KSIZE 5
#endif

#define hypre_ChunkLoopExternalSetup(hypre__nx, hypre__ny, hypre__nz)\
   int hypre__cx = min(hypre__nx / 4 + !!(hypre__nx % 4), MAX_ISIZE);\
   int hypre__cy = min(hypre__ny / 4 + !!(hypre__ny % 4), MAX_JSIZE);\
   int hypre__cz = min(hypre__nz / 4 + !!(hypre__nz % 4), MAX_KSIZE);\
   int znumchunk = hypre__nz / hypre__cz + !!(hypre__nz % hypre__cz);\
   int ynumchunk = hypre__ny / hypre__cy + !!(hypre__ny % hypre__cy);\
   int xnumchunk = hypre__nx / hypre__cx + !!(hypre__nx % hypre__cx);\
   int numchunks = znumchunk * ynumchunk * xnumchunk;\
   int freq[3], reset[3];\
   int start[3];\
   int finish[3];\
   int chunkcount;\
   freq[0] = 1;\
   reset[0] = xnumchunk;\
   freq[1] = reset[0];\
   reset[1] = ynumchunk * znumchunk;\
   freq[2] = reset[1];\
   reset[2] = znumchunk * ynumchunk * xnumchunk
 
#define hypre_ChunkLoopInternalSetup(start, finish, reset, freq,\
                                     hypre__nx, hypre__ny, hypre__nz,\
                                     hypre__cx, hypre__cy, hypre__cz,\
                                     chunkcount)\
      start[0] = ((chunkcount % reset[0]) / freq[0]) * hypre__cx;\
      if (start[0] < hypre__nx - hypre__cx)\
         finish[0] = start[0] + hypre__cx;\
      else\
         finish[0] = hypre__nx;\
      start[1] = ((chunkcount % reset[1]) / freq[1]) * hypre__cy;\
      if (start[1] < hypre__ny - hypre__cy)\
         finish[1] = start[1] + hypre__cy;\
      else\
         finish[1] = hypre__ny;\
      start[2] = ((chunkcount % reset[2]) / freq[2]) * hypre__cz;\
      if (start[2] < hypre__nz - hypre__cz)\
         finish[2] = start[2] + hypre__cz;\
      else\
         finish[2] = hypre__nz

#define hypre_BoxLoop0(i, j, k,loop_size,\
                       body)\
{\
   int hypre__nx = hypre_IndexX(loop_size);\
   int hypre__ny = hypre_IndexY(loop_size);\
   int hypre__nz = hypre_IndexZ(loop_size);\
   hypre_ChunkLoopExternalSetup(hypre__nx, hypre__ny, hypre__nz);\
   for (chunkcount = ifetchadd(&iteration_counter, &hypre_mutex_boxloops) + 0;\
        chunkcount <  numchunks;\
        chunkcount = ifetchadd(&iteration_counter, &hypre_mutex_boxloops) + 0) {\
      hypre_ChunkLoopInternalSetup(start, finish, reset, freq,\
                                   hypre__nx, hypre__ny, hypre__nz,\
                                   hypre__cx, hypre__cy, hypre__cz,\
                                   chunkcount);\
      for (k = start[2]; k < finish[2]; k++) {\
         for (j = start[1]; j < finish[1]; j++)\
         {\
            for (i = start[0]; i < finish[0]; i++)\
            {\
                body;\
            }\
         }\
      }\
    \
   }\
   pthread_mutex_lock(&hypre_mutex_boxloops);\
      hypre_thread_counter++;\
   if (hypre_thread_counter < NUM_THREADS) {\
      pthread_mutex_unlock(&hypre_mutex_boxloops);\
      while (!hypre_thread_release);\
      pthread_mutex_lock(&hypre_mutex_boxloops);\
      hypre_thread_counter--;\
      pthread_mutex_unlock(&hypre_mutex_boxloops);\
      while (hypre_thread_release);\
   }\
   else if (hypre_thread_counter == NUM_THREADS) {\
      hypre_thread_counter--;\
      iteration_counter = 0;\
      pthread_mutex_unlock(&hypre_mutex_boxloops);\
      hypre_thread_release=1;\
      while (hypre_thread_counter);\
      hypre_thread_release=0;\
   }\
}



#define hypre_BoxLoop1(i, j, k, loop_size,\
                       data_box1, start1, stride1, i1,\
                       body)\
{\
   hypre_BoxLoopDeclare(loop_size, data_box1, stride1,\
                        hypre__iinc1, hypre__jinc1, hypre__kinc1);\
   int hypre__nx = hypre_IndexX(loop_size);\
   int hypre__ny = hypre_IndexY(loop_size);\
   int hypre__nz = hypre_IndexZ(loop_size);\
   int orig_i1 = hypre_BoxIndexRank(data_box1, start1);\
   hypre_ChunkLoopExternalSetup(hypre__nx, hypre__ny, hypre__nz);\
   for (chunkcount = ifetchadd(&iteration_counter, &hypre_mutex_boxloops) + 0;\
        chunkcount <  numchunks;\
        chunkcount = ifetchadd(&iteration_counter, &hypre_mutex_boxloops) + 0) {\
      hypre_ChunkLoopInternalSetup(start, finish, reset, freq,\
                                   hypre__nx, hypre__ny, hypre__nz,\
                                   hypre__cx, hypre__cy, hypre__cz,\
                                   chunkcount);\
      i1 = orig_i1 + start[2]*(hypre__kinc1 + start[1]*(hypre__jinc1 +\
                                                    start[0]*hypre__iinc1)) +\
                     start[1]*(hypre__jinc1 + start[0]*hypre__iinc1) +\
                     start[0]*hypre__iinc1;\
      for (k = start[2]; k < finish[2]; k++) {\
         for (j = start[1]; j < finish[1]; j++)\
         {\
            for (i = start[0]; i < finish[0]; i++)\
            {\
               body;\
               i1 += hypre__iinc1;\
            }\
            i1 += hypre__jinc1;\
         }\
         i1 += hypre__kinc1;\
      }\
   }\
   pthread_mutex_lock(&hypre_mutex_boxloops);\
      hypre_thread_counter++;\
   if (hypre_thread_counter < NUM_THREADS) {\
      pthread_mutex_unlock(&hypre_mutex_boxloops);\
      while (!hypre_thread_release);\
      pthread_mutex_lock(&hypre_mutex_boxloops);\
      hypre_thread_counter--;\
      pthread_mutex_unlock(&hypre_mutex_boxloops);\
      while (hypre_thread_release);\
   }\
   else if (hypre_thread_counter == NUM_THREADS) {\
      hypre_thread_counter--;\
      iteration_counter = 0;\
      pthread_mutex_unlock(&hypre_mutex_boxloops);\
      hypre_thread_release=1;\
      while (hypre_thread_counter);\
      hypre_thread_release=0;\
   }\
}



#define hypre_BoxLoop2(i, j, k, loop_size,\
                       data_box1, start1, stride1, i1,\
                       data_box2, start2, stride2, i2,\
                       body)\
{\
   hypre_BoxLoopDeclare(loop_size, data_box1, stride1,\
                        hypre__iinc1, hypre__jinc1, hypre__kinc1);\
   hypre_BoxLoopDeclare(loop_size, data_box2, stride2,\
                        hypre__iinc2, hypre__jinc2, hypre__kinc2);\
   int hypre__nx = hypre_IndexX(loop_size);\
   int hypre__ny = hypre_IndexY(loop_size);\
   int hypre__nz = hypre_IndexZ(loop_size);\
   int orig_i1 = hypre_BoxIndexRank(data_box1, start1);\
   int orig_i2 = hypre_BoxIndexRank(data_box2, start2);\
   hypre_ChunkLoopExternalSetup(hypre__nx, hypre__ny, hypre__nz);\
   for (chunkcount = ifetchadd(&iteration_counter, &hypre_mutex_boxloops) + 0;\
        chunkcount <  numchunks;\
        chunkcount = ifetchadd(&iteration_counter, &hypre_mutex_boxloops) + 0) {\
      hypre_ChunkLoopInternalSetup(start, finish, reset, freq,\
                                   hypre__nx, hypre__ny, hypre__nz,\
                                   hypre__cx, hypre__cy, hypre__cz,\
                                   chunkcount);\
      i1 = orig_i1 + start[2]*(hypre__kinc1 + start[1]*(hypre__jinc1 +\
                                                    start[0]*hypre__iinc1)) +\
                     start[1]*(hypre__jinc1 + start[0]*hypre__iinc1) +\
                     start[0]*hypre__iinc1;\
      i2 = orig_i2 + start[2]*(hypre__kinc2 + start[1]*(hypre__jinc2 +\
                                                    start[0]*hypre__iinc2)) +\
                     start[1]*(hypre__jinc2 + start[0]*hypre__iinc2) +\
                     start[0]*hypre__iinc2;\
      for (k = start[2]; k < finish[2]; k++) {\
         for (j = start[1]; j < finish[1]; j++)\
         {\
            for (i = start[0]; i < finish[0]; i++)\
            {\
               body;\
               i1 += hypre__iinc1;\
               i2 += hypre__iinc2;\
            }\
            i1 += hypre__jinc1;\
            i2 += hypre__jinc2;\
         }\
         i1 += hypre__kinc1;\
         i2 += hypre__kinc2;\
      }\
    \
   }\
   pthread_mutex_lock(&hypre_mutex_boxloops);\
      hypre_thread_counter++;\
   if (hypre_thread_counter < NUM_THREADS) {\
      pthread_mutex_unlock(&hypre_mutex_boxloops);\
      while (!hypre_thread_release);\
      pthread_mutex_lock(&hypre_mutex_boxloops);\
      hypre_thread_counter--;\
      pthread_mutex_unlock(&hypre_mutex_boxloops);\
      while (hypre_thread_release);\
   }\
   else if (hypre_thread_counter == NUM_THREADS) {\
      hypre_thread_counter--;\
      iteration_counter = 0;\
      pthread_mutex_unlock(&hypre_mutex_boxloops);\
      hypre_thread_release=1;\
      while (hypre_thread_counter);\
      hypre_thread_release=0;\
   }\
}



#define hypre_BoxLoop3(i, j, k, loop_size,\
                       data_box1, start1, stride1, i1,\
                       data_box2, start2, stride2, i2,\
                       data_box3, start3, stride3, i3,\
                       body)\
{\
   hypre_BoxLoopDeclare(loop_size, data_box1, stride1,\
                        hypre__iinc1, hypre__jinc1, hypre__kinc1);\
   hypre_BoxLoopDeclare(loop_size, data_box2, stride2,\
                        hypre__iinc2, hypre__jinc2, hypre__kinc2);\
   hypre_BoxLoopDeclare(loop_size, data_box3, stride3,\
                        hypre__iinc3, hypre__jinc3, hypre__kinc3);\
   int hypre__nx = hypre_IndexX(loop_size);\
   int hypre__ny = hypre_IndexY(loop_size);\
   int hypre__nz = hypre_IndexZ(loop_size);\
   int orig_i1 = hypre_BoxIndexRank(data_box1, start1);\
   int orig_i2 = hypre_BoxIndexRank(data_box2, start2);\
   int orig_i3 = hypre_BoxIndexRank(data_box3, start3);\
   hypre_ChunkLoopExternalSetup(hypre__nx, hypre__ny, hypre__nz);\
   for (chunkcount = ifetchadd(&iteration_counter, &hypre_mutex_boxloops) + 0;\
        chunkcount <  numchunks;\
        chunkcount = ifetchadd(&iteration_counter, &hypre_mutex_boxloops) + 0) {\
      hypre_ChunkLoopInternalSetup(start, finish, reset, freq,\
                                   hypre__nx, hypre__ny, hypre__nz,\
                                   hypre__cx, hypre__cy, hypre__cz,\
                                   chunkcount);\
      i1 = orig_i1 + start[2]*(hypre__kinc1 + start[1]*(hypre__jinc1 +\
                                                    start[0]*hypre__iinc1)) +\
                     start[1]*(hypre__jinc1 + start[0]*hypre__iinc1) +\
                     start[0]*hypre__iinc1;\
      i2 = orig_i2 + start[2]*(hypre__kinc2 + start[1]*(hypre__jinc2 +\
                                                    start[0]*hypre__iinc2)) +\
                     start[1]*(hypre__jinc2 + start[0]*hypre__iinc2) +\
                     start[0]*hypre__iinc2;\
      i3 = orig_i3 + start[2]*(hypre__kinc3 + start[1]*(hypre__jinc3 +\
                                                    start[0]*hypre__iinc3)) +\
                     start[1]*(hypre__jinc3 + start[0]*hypre__iinc3) +\
                     start[0]*hypre__iinc3;\
      for (k = start[2]; k < finish[2]; k++) {\
         for (j = start[1]; j < finish[1]; j++)\
         {\
            for (i = start[0]; i < finish[0]; i++)\
            {\
               body;\
               i1 += hypre__iinc1;\
               i2 += hypre__iinc2;\
               i3 += hypre__iinc3;\
            }\
            i1 += hypre__jinc1;\
            i2 += hypre__jinc2;\
            i3 += hypre__jinc3;\
         }\
         i1 += hypre__kinc1;\
         i2 += hypre__kinc2;\
         i3 += hypre__kinc3;\
      }\
    \
   }\
   pthread_mutex_lock(&hypre_mutex_boxloops);\
      hypre_thread_counter++;\
   if (hypre_thread_counter < NUM_THREADS) {\
      pthread_mutex_unlock(&hypre_mutex_boxloops);\
      while (!hypre_thread_release);\
      pthread_mutex_lock(&hypre_mutex_boxloops);\
      hypre_thread_counter--;\
      pthread_mutex_unlock(&hypre_mutex_boxloops);\
      while (hypre_thread_release);\
   }\
   else if (hypre_thread_counter == NUM_THREADS) {\
      hypre_thread_counter--;\
      iteration_counter = 0;\
      pthread_mutex_unlock(&hypre_mutex_boxloops);\
      hypre_thread_release=1;\
      while (hypre_thread_counter);\
      hypre_thread_release=0;\
   }\
}



#define hypre_BoxLoop4(i, j, k, loop_size,\
                       data_box1, start1, stride1, i1,\
                       data_box2, start2, stride2, i2,\
                       data_box3, start3, stride3, i3,\
                       data_box4, start4, stride4, i4,\
                       body)\
{\
   hypre_BoxLoopDeclare(loop_size, data_box1, stride1,\
                        hypre__iinc1, hypre__jinc1, hypre__kinc1);\
   hypre_BoxLoopDeclare(loop_size, data_box2, stride2,\
                        hypre__iinc2, hypre__jinc2, hypre__kinc2);\
   hypre_BoxLoopDeclare(loop_size, data_box3, stride3,\
                        hypre__iinc3, hypre__jinc3, hypre__kinc3);\
   hypre_BoxLoopDeclare(loop_size, data_box4, stride4,\
                        hypre__iinc4, hypre__jinc4, hypre__kinc4);\
   int hypre__nx = hypre_IndexX(loop_size);\
   int hypre__ny = hypre_IndexY(loop_size);\
   int hypre__nz = hypre_IndexZ(loop_size);\
   int orig_i1 = hypre_BoxIndexRank(data_box1, start1);\
   int orig_i2 = hypre_BoxIndexRank(data_box2, start2);\
   int orig_i3 = hypre_BoxIndexRank(data_box3, start3);\
   int orig_i4 = hypre_BoxIndexRank(data_box4, start4);\
   hypre_ChunkLoopExternalSetup(hypre__nx, hypre__ny, hypre__nz);\
   for (chunkcount = ifetchadd(&iteration_counter, &hypre_mutex_boxloops) + 0;\
        chunkcount <  numchunks;\
        chunkcount = ifetchadd(&iteration_counter, &hypre_mutex_boxloops) + 0) {\
      hypre_ChunkLoopInternalSetup(start, finish, reset, freq,\
                                   hypre__nx, hypre__ny, hypre__nz,\
                                   hypre__cx, hypre__cy, hypre__cz,\
                                   chunkcount);\
      i1 = orig_i1 + start[2]*(hypre__kinc1 + start[1]*(hypre__jinc1 +\
                                                    start[0]*hypre__iinc1)) +\
                     start[1]*(hypre__jinc1 + start[0]*hypre__iinc1) +\
                     start[0]*hypre__iinc1;\
      i2 = orig_i2 + start[2]*(hypre__kinc2 + start[1]*(hypre__jinc2 +\
                                                    start[0]*hypre__iinc2)) +\
                     start[1]*(hypre__jinc2 + start[0]*hypre__iinc2) +\
                     start[0]*hypre__iinc2;\
      i3 = orig_i3 + start[2]*(hypre__kinc3 + start[1]*(hypre__jinc3 +\
                                                    start[0]*hypre__iinc3)) +\
                     start[1]*(hypre__jinc3 + start[0]*hypre__iinc3) +\
                     start[0]*hypre__iinc3;\
      i4 = orig_i4 + start[2]*(hypre__kinc4 + start[1]*(hypre__jinc4 +\
                                                    start[0]*hypre__iinc4)) +\
                     start[1]*(hypre__jinc4 + start[0]*hypre__iinc4) +\
                     start[0]*hypre__iinc4;\
      for (k = start[2]; k < finish[2]; k++) {\
         for (j = start[1]; j < finish[1]; j++)\
         {\
            for (i = start[0]; i < finish[0]; i++)\
            {\
               body;\
               i1 += hypre__iinc1;\
               i2 += hypre__iinc2;\
               i3 += hypre__iinc3;\
               i4 += hypre__iinc4;\
            }\
            i1 += hypre__jinc1;\
            i2 += hypre__jinc2;\
            i3 += hypre__jinc3;\
            i4 += hypre__jinc4;\
         }\
         i1 += hypre__kinc1;\
         i2 += hypre__kinc2;\
         i3 += hypre__kinc3;\
         i4 += hypre__kinc4;\
      }\
    \
   }\
   pthread_mutex_lock(&hypre_mutex_boxloops);\
      hypre_thread_counter++;\
   if (hypre_thread_counter < NUM_THREADS) {\
      pthread_mutex_unlock(&hypre_mutex_boxloops);\
      while (!hypre_thread_release);\
      pthread_mutex_lock(&hypre_mutex_boxloops);\
      hypre_thread_counter--;\
      pthread_mutex_unlock(&hypre_mutex_boxloops);\
      while (hypre_thread_release);\
   }\
   else if (hypre_thread_counter == NUM_THREADS) {\
      hypre_thread_counter--;\
      iteration_counter = 0;\
      pthread_mutex_unlock(&hypre_mutex_boxloops);\
      hypre_thread_release=1;\
      while (hypre_thread_counter);\
      hypre_thread_release=0;\
   }\
}



#endif

#endif

