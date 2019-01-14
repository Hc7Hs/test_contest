#include <unistd.h>
#include <signal.h>

#include <stdlib.h>
#include <stdio.h>

#include <sys/sysinfo.h>
#include <time.h>
 
#define __USE_GNU
#include <sched.h>
#include <pthread.h>
#include <errno.h>
 
// include/linux/compiler.h
#define likely(x)    __builtin_expect(!!(x), 1)
#define unlikely(x)  __builtin_expect(!!(x), 0)
 
 
unsigned long long lock_counter = 0;
 
int nr_parallel_threads = 0;
 
pthread_barrier_t global_barrier;
 
int thread_stop=0;
 
#include "tony_pthread_data.h"
//#include "tony_misc.c"
#include "nop_delay.h"
#ifndef __CYGWIN__
// Error is introduced as a header file, tony_misc.c is directly introduced to 	author:hh
#include <sys/syscall.h>

pid_t gettid(void)

{

            return syscall(__NR_gettid);

}

#endif

void nsleep(double seconds, _Bool sleep_with_abs_time) 
	// The parameter type was changed from "bool" to _Bool type. Reason: C99 type compilation failed.	author:hh

{

        long int sec  = (long int)(seconds);

        long int nsec = (long int)((seconds - sec) * 1000000000);

 

        if (!sleep_with_abs_time) {  /* relative time sleep */

                struct timespec tv[2] = {

                        {

                                .tv_sec  = sec,

                                .tv_nsec = nsec,

                        },

 

                        {

                                .tv_sec  = 0,

                                .tv_nsec = 0,

                        }

                };

 

                int req = 0;

                int rem = 1;

                while (-1 == clock_nanosleep(CLOCK_MONOTONIC, 0, &tv[req], &tv[rem])) {

                        if (EINTR == errno) {

                                req ^= rem; rem ^= req; req ^= rem;

                                continue;

                        } else {

                                perror("clock_nanosleep");

                                return;

                        }

                }

 

        } else {  /* absolute time sleep */

                struct timespec req;

                struct timespec rem;

 

                if (clock_gettime(CLOCK_MONOTONIC, &req) != 0) {

                        perror("clock_gettime");

                        return;

                }

 

                req.tv_sec  += sec;

                req.tv_nsec += nsec;

 

                while (-1 == clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &req, &rem)) {

                        if (EINTR == errno) {

                                continue;

                        } else {

                                perror("clock_nanosleep");

                                return;

                        }

                }

        }

}

 

void nano_sleep(long long nano)

{

        long int sec  = (long int)(nano / 1000000000);

        long int nsec = (long int)(nano % 1000000000);

 

        struct timespec tv[2] = {

                {

                        .tv_sec  = sec,

                        .tv_nsec = nsec,

                },

 

                {

                        .tv_sec  = 0,

                        .tv_nsec = 0,

                }

        };

 

        int req = 0;

        int rem = 1;

        while (-1 == clock_nanosleep(CLOCK_MONOTONIC, 0, &tv[req], &tv[rem])) {

                if (EINTR == errno) {

                        req ^= rem; rem ^= req; req ^= rem;

                        continue;

                } else {

                        perror("clock_nanosleep");

                        return;

                }

        }

 

 

}
struct timespec tm_start;
struct timespec tm_ended;
//Introduction of global variables to calculate thread runtime author:hh
struct thread_data*  thread_param;
//Introduce global variables to pass in the number of lock_counts for a single thread
void *thread_routine(void *arg)
{
    struct thread_data* parg = (struct thread_data*) arg;
    int cpu_id = parg->cpu_id;
    long long counter=0;
    // bind to the core test to add the following code
    // tony: do not bind
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(cpu_id, &set);
 
    if (sched_setaffinity(gettid(), sizeof(cpu_set_t), &set)) {
        perror("sched_setaffinity");
        return NULL;
    }
    
 
    /* The pthread_barrier_wait() function shall synchronize participating threads
     * at the barrier referenced by barrier.
     * The calling thread shall block until
     * the required number of threads have called pthread_barrier_wait() specifying the barrier.
     */
    pthread_barrier_wait(&global_barrier);
 
    // while (likely(thread_stop == 0)) {
    while(1)
	{
		__sync_fetch_and_add(&lock_counter, 1);	
		parg->count++;
	}	 
	/*
    int i = 0;
    while(i<1000){
     
        __sync_fetch_and_add(&lock_counter, 1);
		i++;
	}*/
    /*
        for(long long i = 0; i < parg->lock_interval; i++)
        {
                NOP_DELAY_72
        }
        counter++;
    }
 
    parg->lock_times = counter;
    */
    return NULL;
}



void sig_handler(int signo)
{
        if (signo == SIGINT)
        {			
				int i=0;
				for(i=0;i<nr_parallel_threads;i++){
					printf("\nThe thread %d,count:%d\n",i,thread_param[i].count);
				}//同不绑核测试
                printf("lock_counter=%ld\n", lock_counter);
			    clock_gettime(CLOCK_MONOTONIC, &tm_ended);  // Get the monotonic time at the end of the thread	author：hh
				printf("starttime:sec:%lld,nsec:%lld\n",tm_start.tv_sec,tm_start.tv_nsec);     
				printf("endtime:sec:%lld,nsec:%lld\n",tm_ended.tv_sec,tm_ended.tv_nsec);    
			    printf("running time:%lld\n",(tm_ended.tv_sec-tm_start.tv_sec)*1000000000+tm_ended.tv_nsec-tm_start.tv_nsec);    
                exit(0);
        }
}
//mapping table data structure is used to store the correspondence between threads and core.	author：hh
typedef struct index{
	int cpu_no;
	int thread_no;
}Index;

//mapping function returns the cpu number bound to the thread	author：hh
int mapping(int proc_num,int thread_num,int t_no){
	printf("nprocs:%d\n",proc_num);
	Index* index=(Index*)malloc(thread_num*sizeof(Index));
	int tnum=0,cnum=0;
	while(tnum<thread_num){
		index[tnum].thread_no=tnum;
		index[tnum].cpu_no=cnum;
		tnum++;
		cnum++;
		if(cnum>=proc_num){
			cnum=0;		
		}
	} 
	index[t_no].cpu_no;      
}

 
int main(int argc, char *argv[])
{

 
        if (argc < 4)
        {
                //      0     1                2            3
               printf("%s $execute_duration $threads $lock_delay\n", argv[0]);
               exit(EXIT_FAILURE);
        }
 
        int exe_duration = atoi(argv[1]);
        nr_parallel_threads = atoi(argv[2]);
        int lock_delay=atoi(argv[3]);

	    if (signal(SIGINT, sig_handler) == SIG_ERR)
        {
                printf("failed to install signal handler\n");
                exit(1);
        }
 
        pthread_barrier_init(&global_barrier, NULL, nr_parallel_threads);
 
        pthread_t *thread_list = (pthread_t *)malloc(sizeof(pthread_t) * nr_parallel_threads);
        if (thread_list == NULL) {
                perror("malloc");
                exit(EXIT_FAILURE);
        }

        thread_param = (struct thread_data*) calloc (nr_parallel_threads, sizeof(struct thread_data));
 
        if (thread_param == NULL) {
                perror("calloc");
                exit(EXIT_FAILURE);
        }
 
        clock_gettime(CLOCK_MONOTONIC, &tm_start);
        int i=0;
        for( i = 0; i < nr_parallel_threads; ++i) {
                thread_param[i].cpu_id = mapping(get_nprocs(),nr_parallel_threads,i);
                thread_param[i].lock_interval=lock_delay;
                pthread_create(&thread_list[i], NULL, thread_routine, thread_param+i);
        }
        //printf("third\n");
        nsleep(exe_duration, 0);  // Initially the second false parameter does not pass, so changed to 0 author:hh
        // notify all threads stop and exit
        thread_stop = 1;
 
        for( i = 0; i < nr_parallel_threads; ++i) {
               pthread_join(thread_list[i], NULL);
               lock_counter += thread_param[i].lock_times;
        }
 
        clock_gettime(CLOCK_MONOTONIC, &tm_ended);
        printf("argv[0]:%s,exe_duration:%d,\tnr_parallel_threads:%d,lock_delay:%d,rate:%.0f,\t\t", argv[0], exe_duration,
                nr_parallel_threads, lock_delay,
                1.0 * lock_counter / exe_duration);
        for( i = 0; i < nr_parallel_threads; ++i) {
                if (i != nr_parallel_threads - 1)
                {
                        printf("true %lld, ", thread_param[i].lock_times); //Add a logo to distinguish 	author:hh
                }
                else
                {
                        printf("false %lld \n", thread_param[i].lock_times); 
                }
        }
 
        return 0;
}

	
