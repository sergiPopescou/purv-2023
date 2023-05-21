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
   
   #define PRE_ALLOCATION_SIZE (10*1024*1024) /* 10MB pagefault free buffer */
   #define MY_STACK_SIZE       (100*1024)      /* 100 kB dodatak za stek */
   #define ISPIS 0
   static pthread_mutex_t mtx;
   static pthread_mutexattr_t mtx_attr;
   static int shared_value = 0;
   

   struct argumenti_niti{
      int sleep;
      int priority;
      int extra_stack_size;
      int print_info;
      int resource_change;
   };


   static void setprio(int prio, int sched) {

   	struct sched_param param;
   	// podesavanje prioriteta i schedulera
   	// vise o tome kasnije
   	param.sched_priority = prio;
   	if (sched_setscheduler(0, sched, &param) < 0)
   		perror("sched_setscheduler");
   }


   
   void show_new_pagefault_count(const char* logtext, const char* allowed_maj, const char* allowed_min) {

	   // ispis pagefaultova!
   	static int last_majflt = 0, last_minflt = 0;
   	struct rusage usage;
   
   	getrusage(RUSAGE_SELF, &usage);
   
   	printf("%-30.30s: Pagefaults, Major:%ld (Allowed %s), " \
   	       "Minor:%ld (Allowed %s)\n", logtext,
   	       usage.ru_majflt - last_majflt, allowed_maj,
   	       usage.ru_minflt - last_minflt, allowed_min);
   	
   	last_majflt = usage.ru_majflt; 
   	last_minflt = usage.ru_minflt;
   }
   

   static void prove_thread_stack_use_is_safe(int stacksize, int do_log) {

	   //gurni stek u RAM
   	volatile char buffer[stacksize];
   	int i;
   
   	for (i = 0; i < stacksize; i += sysconf(_SC_PAGESIZE)) {
   		/* "Touch" za cijeli stek od programske niti */
   		buffer[i] = i;
   	}
	 if(do_log)
		show_new_pagefault_count("Caused by using thread stack", "0", "0");
   }
   
  
   
   static void error(int at) {

   	/* Ispisi gresku i izadji */
   	fprintf(stderr, "Some error occured at %d", at);
   	exit(1);
   }
   
   static void configure_malloc_behavior(void) {

   	/* konfiguracija allociranja memorije */
   	if (mlockall(MCL_CURRENT | MCL_FUTURE))
   		perror("mlockall failed:");
   
   	/* sbrk nema nazad */
   	mallopt(M_TRIM_THRESHOLD, -1);
   
   	/* iskljuci mmap */
   	mallopt(M_MMAP_MAX, 0);
   }
   
   static void reserve_process_memory(int size) {
	   // rezervisanje memorije, guranje svega u RAM
   	int i;
   	char *buffer;
   
   	buffer = malloc(size);
   
   	for (i = 0; i < size; i += sysconf(_SC_PAGESIZE)) {
   		buffer[i] = 0;
   	}
   	free(buffer);
   }


   static void* resource_thread_fn(void* arg){

      struct argumenti_niti arg_niti = *(struct argumenti_niti *)arg;

      setprio(arg_niti.priority, SCHED_RR);

      if(arg_niti.print_info){
        printf("I am an RT-thread with a stack that does not generate " \
             "page-faults during use, stacksize=%i\n", MY_STACK_SIZE); 
        show_new_pagefault_count("Caused by creating thread", ">=0", ">=0");
        prove_thread_stack_use_is_safe(MY_STACK_SIZE, 1);
   
      }

      struct timespec ts;
      ts.tv_sec = arg_niti.sleep;
      ts.tv_nsec = 0;

      if(arg_niti.priority == 3)
         clock_nanosleep(CLOCK_REALTIME, 0, &ts, NULL);

      pthread_mutex_lock(&mtx);

         printf("Nit prioriteta %d\n", arg_niti.priority);
         printf("\tPristupanje zajednickom resursu...\n");
         shared_value += arg_niti.resource_change;
         // treba da se zadrzi u kriticnooj sekciji dovoljno dugo da se probude ostale niti
         sleep(5);

      pthread_mutex_unlock(&mtx);   
      

      free(arg);
      return NULL;

   }


   static void* non_res_thread_fn(void* arg){

      

      struct argumenti_niti arg_niti = *(struct argumenti_niti *)arg;

      setprio(arg_niti.priority, SCHED_RR);
   
      if(arg_niti.print_info){
         printf("I am an RT-thread with a stack that does not generate " \
             "page-faults during use, stacksize=%i\n", MY_STACK_SIZE);
         
         show_new_pagefault_count("Caused by creating thread", ">=0", ">=0");
         prove_thread_stack_use_is_safe(MY_STACK_SIZE, 1);
      }

      struct timespec ts;
      ts.tv_sec = arg_niti.sleep;
      ts.tv_nsec = 0;

      clock_nanosleep(CLOCK_REALTIME, 0, &ts, NULL);

      printf("Nit prioriteta %d\n", arg_niti.priority);
      
      free(arg);
      return NULL;

   }



   static pthread_t start_rt_thread(int sleep, int priority, int extra_stack_size, int print_info, int resource_change) {

      pthread_t thread;
      pthread_attr_t attr;
   
      /* inicijalizacija programske niti */
      if (pthread_attr_init(&attr))
         error(1);

      /* inicijalizacija memorije potrebne za stek */
      if (pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN + MY_STACK_SIZE))
         error(2);



      /* pripremanje argumenata koji ce biti proslijedjeni u nit */
      struct argumenti_niti *arg_niti = malloc(sizeof(struct argumenti_niti));

      arg_niti->sleep = sleep;
      arg_niti->priority = priority;
      arg_niti->extra_stack_size = extra_stack_size;
      arg_niti->print_info = print_info;
      arg_niti->resource_change = resource_change;

      /* kreiranje programske niti */
      if(resource_change == 0){
         pthread_create(&thread, &attr, non_res_thread_fn, (void*)arg_niti);   
      } else {
         pthread_create(&thread, &attr, resource_thread_fn, (void*)arg_niti);
      }

      return thread;

   }
   
   int main(int argc, char *argv[]) {

      /**** boilerplate kod  *****/

      if(ISPIS){
         show_new_pagefault_count("Initial count", ">=0", ">=0");
      }
      
      /* konfiguracija dinamicke memorije, zakljucavanja i podesavanjae mmap i sbrk */
   	configure_malloc_behavior();
   
      if(ISPIS){
        show_new_pagefault_count("mlockall() generated", ">=0", ">=0"); 
      }
   	
      /* rezervisanje memorije  u RAM*/
   	reserve_process_memory(PRE_ALLOCATION_SIZE);
   
      if(ISPIS){
        show_new_pagefault_count("malloc() and touch generated", ">=0", ">=0"); 
      }

   	if(ISPIS){
         printf("Ponovna alokacija memorije nece izazvati nikakve pagefaultove ");
         reserve_process_memory(PRE_ALLOCATION_SIZE);
         show_new_pagefault_count("2nd malloc() and use generated", "0", "0");
      }
   	
      if(ISPIS){
        printf("\n\nLook at the output of ps -leyf, and see that the RSS is now about %d [MB]\n", PRE_ALLOCATION_SIZE / (1024 * 1024));
      }
   	
      /**** zavrsetak boilerplate koda  *****/



      /**** real-time kod  *****/


      /* inicijalizacija mutex-a */
      pthread_mutexattr_init(&mtx_attr);
      pthread_mutexattr_setprotocol(&mtx_attr, PTHREAD_PRIO_PROTECT);
      pthread_mutex_init(&mtx, &mtx_attr);


      printf("Press <ENTER> to exit\n");
     

      int print_info = 0;
      if(ISPIS){
         print_info = 1;
      }
      //start_rt_thread(int sleep, int priority, int extra_stack_size, int print_info, int resource_change);
   	start_rt_thread(2, 3, 0, print_info, 3);   // nit najviseg prioriteta
      start_rt_thread(0, 1, 0, print_info, 1);   // nit najnizeg prioriteta
      start_rt_thread(3, 2, 0, print_info, 0);  // nit srednjeg prioriteta


   	getchar();
      printf("Vrijednost resursa je: %d\n", shared_value);

      pthread_mutexattr_destroy(&mtx_attr);
      pthread_mutex_destroy(&mtx);
   
   	return 0;
   }
   