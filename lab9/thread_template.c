// Kompajlirati sa 'gcc thread_template.c -lpthread -lrt -Wall'
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h> // potrebno za mlockall()
#include <unistd.h> // potrebno za sysconf(int name);
#include <malloc.h>
#include <sys/time.h> // potrebno za getrusage
#include <sys/resource.h> // potrebno za getrusage
#include <pthread.h>
#include <limits.h>

#define PRE_ALLOCATION_SIZE (10*1024*1024) /* 100MB pagefault free buffer */
#define MY_STACK_SIZE       (100*1024)      /* 100 kB dodatak za stek */

struct params{
	int sleep_sec, priority, stack_size, print_on, value;
};

static pthread_mutex_t mtx;
static pthread_mutexattr_t mtx_attr;
static int VALUE = 0;
   
static void setprio(int prio, int sched){
	struct sched_param param;
	param.sched_priority = prio;
	if(sched_setscheduler(0, sched, &param) < 0)
		perror("sched_setscheduler");
}

static void *resource_thread_fn(void *args){
	struct params* prms = (struct params*)args;
   	struct timespec ts;
   	ts.tv_sec = prms->sleep_sec;
   	ts.tv_nsec = 0;

   	setprio(prms->priority, SCHED_RR);

	if(prms->priority == sched_get_priority_max(SCHED_RR))
		clock_nanosleep(CLOCK_REALTIME, 0, &ts, NULL);
   
   	if(prms->print_on)
   		printf("I am an RT-thread with a stack that does not generate page-faults during use, stacksize=%i\n", prms->stack_size);
   
   	pthread_mutex_lock(&mtx);
		VALUE += prms->value;
		printf("RT-Thread priority %d incerasing shared value to %d.\n", prms->priority, VALUE);
		if(prms->priority == sched_get_priority_min(SCHED_RR)){
			struct timespec start, end;
			long elapsed_time;
			clock_gettime(CLOCK_MONOTONIC, &start); // Get start time
			while (1) {
				clock_gettime(CLOCK_MONOTONIC, &end); 
				elapsed_time = end.tv_sec - start.tv_sec;
				if (elapsed_time >= 5) {
					break;
				}
			}
		}
	pthread_mutex_unlock(&mtx);
   
   	return NULL;
}

static void *non_resource_thread_fn(void *args){
	struct params* prms = (struct params*)args;
   	struct timespec ts;
   	ts.tv_sec = prms->sleep_sec;
   	ts.tv_nsec = 0;
   
   	setprio(prms->priority, SCHED_RR);

	clock_nanosleep(CLOCK_REALTIME, 0, &ts, NULL);
   
   	if(prms->print_on)
   		printf("I am an RT-thread with a stack that does not generate page-faults during use, stacksize=%i\n", prms->stack_size);
   
   	printf("RT-Thread priority %d, not accessing shared value.\n", prms->priority);
   
   	return NULL;
}
   
static void error(int at) {
   	fprintf(stderr, "Some error occured at %d", at);
   	exit(1);
}
   
static pthread_t start_rt_thread(struct params* param){
   	pthread_t thread;
   	pthread_attr_t attr;
   	/* inicijalizacija programske niti */
   	if (pthread_attr_init(&attr))
   		error(1);
   	/* inicijalizacija memorije potrebne za stek */
   	if (pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN + param->stack_size))
   		error(2);
   	/* kreiranje programske niti */
	if(param->value != 0)
   		pthread_create(&thread, &attr, resource_thread_fn, (void*)param);
	else
		pthread_create(&thread, &attr, non_resource_thread_fn, (void*)param);
   	return thread;
}
   
static void configure_malloc_behavior(void){
   	/* konfiguracija allociranja memorije */
   	if (mlockall(MCL_CURRENT | MCL_FUTURE))
   		perror("mlockall failed:");
   
   	/* sbrk nema nazad */
   	mallopt(M_TRIM_THRESHOLD, -1);
   
   	/* iskljuci mmap */
   	mallopt(M_MMAP_MAX, 0);
}
   
static void reserve_process_memory(int size){
	   // rezervisanje memorije, guranje svega u RAM
   	int i;
   	char *buffer;
   
   	buffer = malloc(size);
   
   	for (i = 0; i < size; i += sysconf(_SC_PAGESIZE)) {
   		buffer[i] = 0;
   	}
   	free(buffer);
}
   
int main(int argc, char *argv[]){
	struct params p1 = {
		.stack_size = MY_STACK_SIZE,
		.value = 10,
		.print_on = 0,
		.priority = sched_get_priority_max(SCHED_RR),
		.sleep_sec = 1
	};
	struct params p2 = {
		.stack_size = MY_STACK_SIZE,
		.value = 0,
		.print_on = 0,
		.priority = sched_get_priority_max(SCHED_RR)/2,
		.sleep_sec = 2
	};
	struct params p3 = {
		.stack_size = MY_STACK_SIZE,
		.value = 2,
		.print_on = 0,
		.priority = sched_get_priority_min(SCHED_RR),
		.sleep_sec = 0
	};

   	configure_malloc_behavior();
   	reserve_process_memory(PRE_ALLOCATION_SIZE);

	pthread_mutexattr_init(&mtx_attr);
	pthread_mutexattr_setprotocol(&mtx_attr, PTHREAD_PRIO_NONE);
	pthread_mutex_init(&mtx,&mtx_attr);

   	start_rt_thread(&p1);
	start_rt_thread(&p2);
	start_rt_thread(&p3);

   	getchar();
   
	pthread_mutexattr_destroy(&mtx_attr);
	pthread_mutex_destroy(&mtx);
   	return 0;
}
   
