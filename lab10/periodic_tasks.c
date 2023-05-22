#include <sys/time.h>
#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

const int PERIOD_ONE = 500;
const int PERIOD_TWO = 1800;
static int LCM = 0;
static int GCM = 0;
int counter = 0;

static sigset_t sigset;

static void wait_next_activation(void){
    int dummy;
    sigwait(&sigset, &dummy);
    counter++;
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

void task1(void) {
  int i,j;
  for (i=0; i<4; i++) {
    for (j=0; j<1000; j++) ;
    printf("1");
    fflush(stdout);
  }
}

void task2(void) {
  int i,j;
  for (i=0; i<6; i++) {
    for (j=0; j<10000; j++) ;
    printf("2");
    fflush(stdout);
  }
}

void calc_lcm(int a, int b){
  int n1 = a, n2 = b;
  while(n1 != n2){
    if(n1 < n2)
      n1 += a;
    else 
      n2 += b;
  }
  LCM = n1;
}

void calc_gcm(int a, int b){
  int n1 = a, n2 = b;
  while(n1 != n2){
    if(n1 > n2)
      n1 -= n2;
    else
      n2 -= n1;
  }
  GCM = n1;
}

int main(int argc, char *argv[]) {
    int res;

    calc_lcm(PERIOD_ONE, PERIOD_TWO);
    calc_gcm(PERIOD_ONE, PERIOD_TWO);

    res = start_periodic_timer(1000000, GCM*1000);
    if (res < 0) {
        perror("Start Periodic Timer");

        return -1;
    }

    while(1) {
        wait_next_activation();
        if(counter * GCM == LCM){
          counter = 0;
          task1();
          task2();
        }
        else if((counter * GCM) % PERIOD_ONE == 0)
          task1();
        else if((counter * GCM) % PERIOD_TWO == 0)
          task2();
    }
    return 0;
}