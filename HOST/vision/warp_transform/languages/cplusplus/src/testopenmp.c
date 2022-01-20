#include <stdio.h>
#include <omp.h>

void hello(int thr_nr){
    #pragma omp parallel num_threads(thr_nr)
    //#pragma omp parallel default(shared) private(iam,nt,ipoints,istart)
        {
        printf("Hello, world thread %d.\n", omp_get_thread_num());
        }
}
int main(void)
{
//   #pragma omp parallel
//     printf("Hello, world.\n");
//  return 0;
omp_set_dynamic(0);
int thr_nr = 10;
hello(thr_nr);
printf("\n\n");
hello(thr_nr*thr_nr);
return 0;
}



// OMP_NUM_THREADS
// omp_set_num_threads()
// 1 )num_threads


// Example ploop.1.c --> // loop
// Example parallel.1.c -->