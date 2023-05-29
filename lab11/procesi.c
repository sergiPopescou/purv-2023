#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <signal.h>
#include <stdlib.h>
#include <sched.h>
#include <string.h>

const int PERIOD_ONE = 40;
const int PERIOD_TWO = 80;
const int PERIOD_THREE = 120;
const int OFFSET = 1000000;

static sigset_t sigset;
pid_t child1, child2, child3;

// Kad uradimo CTRL-C da ubijemo djecu procese
void kill_handler(int signo, siginfo_t *info, void *context){
  if(signo==SIGINT){
    kill(child1,SIGSTOP);
    kill(child2,SIGSTOP);
    kill(child3,SIGSTOP);
  }
}

// periodicni zadatak na 40 milisekundi
void task1(void)
{
  int i,j;
 
  for (i=0; i<4; i++) {
    for (j=0; j<1000; j++) ;
    printf("1");
    fflush(stdout);
  }
}

// periodicni zadatak na 80 milisekundi
void task2(void)
{
  int i,j;

  for (i=0; i<6; i++) {
    for (j=0; j<10000; j++) ;
    printf("2");
    fflush(stdout);
  }
}

// periodicni zadatak na 120 milisekundi
void task3(void)
{
  int i,j;

  for (i=0; i<6; i++) {
    for (j=0; j<100000; j++) ;
    printf("3");
    fflush(stdout);
  }
}

static void wait_next_activation(void){
    int dummy;
    sigwait(&sigset, &dummy);
}

int start_periodic_timer(uint64_t offs, int period){
    struct itimerspec t;
    struct sigevent sigev;
    timer_t timer;
    const int signal = SIGALRM;
    int res;

    t.it_value.tv_sec = offs / 1000000;
    t.it_value.tv_nsec = (offs % 1000000) * 1000;
    t.it_interval.tv_sec = period / 1000000;
    t.it_interval.tv_nsec = (period % 1000000) * 1000;
	
    sigemptyset(&sigset);
    sigaddset(&sigset, signal);
    sigprocmask(SIG_BLOCK, &sigset, NULL);

    memset(&sigev, 0, sizeof(struct sigevent));
    sigev.sigev_notify = SIGEV_SIGNAL;
    sigev.sigev_signo = signal;
	
    res = timer_create(CLOCK_MONOTONIC, &sigev, &timer);
    if (res < 0) {
        perror("Timer Create");
        exit(-1);
    }
    return timer_settime(timer, 0, &t, NULL);
}

int main(int argc, char *argv[]){
  child1 = fork();
  if (child1 < 0) {
        perror("Fork child 1 failure !\n");
        return -1;
  }
  if (child1 == 0) {
    int res;
    int pmin = sched_get_priority_min(SCHED_FIFO);
    struct sched_param param; 
    param.sched_priority = pmin + 50; 
    sched_setscheduler(0, SCHED_FIFO, &param);
    res = start_periodic_timer(OFFSET, PERIOD_ONE);
    if(res < 0){
      perror("Child 1 timer failure !\n");
      return -1;
    }
    while(1){
      wait_next_activation();
      task1();
    }
    return 50;
  }

  child2 = fork();
  if (child2 < 0) {
        perror("Fork child 2 failure !\n");
        return -1;
  }
  if (child2 == 0) {
    int res;
    int pmin = sched_get_priority_min(SCHED_FIFO);
    struct sched_param param; 
    param.sched_priority = pmin + 30; 
    sched_setscheduler(0, SCHED_FIFO, &param);
    res = start_periodic_timer(OFFSET, PERIOD_TWO);
    if(res < 0){
      perror("Child 2 timer failure !\n");
      return -1;
    }
    while(1){
      wait_next_activation();
      task2();
    }
    return 30;
  }

  child3 = fork();
  if (child3 < 0) {
        perror("Fork child 3 failure !\n");
        return -1;
  }
  if (child3 == 0) {
    int res;
    int pmin = sched_get_priority_min(SCHED_FIFO);
    struct sched_param param; 
    param.sched_priority = pmin + 10; 
    sched_setscheduler(0, SCHED_FIFO, &param);
    res = start_periodic_timer(OFFSET, PERIOD_THREE);
    if(res < 0){
      perror("Child 3 timer failure !\n");
      return -1;
    }
    while(1){
      wait_next_activation();
      task3();
    }
    return 10;
  }

  struct sigaction act;
  memset(&act,0,sizeof(act));
  act.sa_sigaction=kill_handler;
  act.sa_flags=SA_SIGINFO;
  sigaction(SIGINT,&act,NULL);

  wait();
  wait();
  wait();

  return 0;
}

