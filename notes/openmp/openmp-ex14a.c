#include <stdio.h>
#include <omp.h>

int main(void)
{
  int N = 10;
  int i;

  /* Thus far the first thread has always received the start of the loop, but
   * we can control this with the schedule() clause: schedule(static,5) means
   * that we use the static scheduling methods with a block size of 5.
   *
   * scheduling methods (from [[https://software.intel.com/en-us/articles/openmp-loop-scheduling]]):
   * static:  divide the loop into equal-sized chunks or as equal as possible
   *          in the case where the number of loop iterations is not evenly divisible
   *          by the number of threads multiplied by the chunk size. By default, chunk
   *          size is loop_count/number_of_threads.Set chunk to 1 to interleave the
   *          iterations.
   * dynamic: Use the internal work queue to give a chunk-sized block of loop
   *          iterations to each thread. When a thread is finished, it retrieves the
   *          next block of loop iterations from the top of the work queue. By default,
   *          the chunk size is 1. Be careful when using this scheduling type
   *          because of the extra overhead involved.
   * guided:  Similar to dynamic scheduling, but the chunk size starts off
   *          large and decreases to better handle load imbalance between iterations.
   *          The optional chunk parameter specifies them minimum size chunk to use. By
   *          default the chunk size is approximately loop_count/number_of_threads.
   * auto:    When schedule (auto) is specified, the decision regarding
   *          scheduling is delegated to the compiler. The programmer gives the
   *          compiler the freedom to choose any possible mapping of iterations to
   *          threads in the team.
   * runtime: Uses the OMP_schedule environment variable to specify which one
   *          of the three loop-scheduling types should be used. OMP_SCHEDULE is a
   *          string formatted exactly the same as would appear on the parallel
   *          construct.
   * */
#pragma omp parallel for schedule(static,5)
  for (i = 0; i < N; i++) {
    int my_thread = omp_get_thread_num();

    printf("iteration %d, thread %d\n", i, my_thread);
  }

  return 0;
}
