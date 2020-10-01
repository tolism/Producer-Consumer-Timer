/*
 * FILE: prod-cons.c
 * THMMY, 8th semester, Real Time Embedded Systems: 2nd assignment
 * Implementation of producer - consumer problem using timer
 * Author:
 *   Moustaklis Apostolos, 9127, amoustakl@auth.gr
 * Compile command with :
 *   make prod-con
 * Run command example:
 *   ./prod-con
 * It will create a queue with QUEUESIZE size and nOfProducers / nOfConsumers
 */

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>


// Defines for the queue and the prod/cons
#define QUEUESIZE 5
#define nOfProducers 1
#define nOfConsumers 2
#define nOfFunctions 3
#define nOfArguments 16
#define nOfExec 10
#define PI 3.14159265



#define secondsToRun 10


// Thread functions decleration
void * producer(void * args);
void * consumer(void * args);



// Functions decleration
void * circleArea(void * args);
void * circleCirCumf(void * args);
void * expo(void * args);
void * sinF(void * args);
void * cosF(void * args);

void * ( * functions[nOfFunctions])(void * x) = {
//  circleArea,
//  circleCirCumf,
  expo,
  sinF,
  cosF

};

int argu[16] = {  0,  5,  10,  15,  20,  25,  30, 35,  40,  45,  60,  75,  90,  100,  120,  180};


typedef struct {
  void * ( * work)(void * );
  void * arg;
}
workFunction;

struct timeval arrTime[QUEUESIZE];



// Global flags/counters/times
int prodFinished = 0;
int terminateConFlag = 0;
struct timeval timeStamp;
double tempTime = 0;
int  jobsLostCounter=0;


// The queue
typedef struct {
  workFunction buf[QUEUESIZE];
  long head, tail;
  int full, empty;
  pthread_mutex_t * mut;
  pthread_cond_t * notFull, * notEmpty;
}
queue;

//Consumer Argument Struct
typedef struct{
  queue *Q;
  pthread_mutex_t *helperMut;
} tArg;


//The timer struct
typedef struct{
    int period; //Period of the timer
    int tasksToExecute; //The number of tasks timer needs to execute
    int startDelay; //A possible delay for the timer to start
    void * (*startFnc) (void *); //Start function of Timer
    void * (*stopFnc)  (void *); //Stop function of Timer
    void * (*timerFnc) (void *); //Function the Timer will execute
    void * (*errorFnc) (void *); //Error function
    void *userData;
    queue *Q;
    pthread_t tid;
} Timer;


typedef struct{
  int *buffer;
  int index;
} metrixArray;

//Metrix files to calculate for time measurements
FILE *f1;
FILE *f2;
FILE *f3;
FILE *f4;


//Functions Decleration
queue * queueInit(void);
void queueDelete(queue * q);
void queueAdd(queue * q, workFunction in );
void queueDel(queue * q, workFunction * out);
Timer *timerInit(int period, int tasksToExecute, int startDelay , queue *queue, void *(*producer)(void *arg),void *(*errorFnc)(), void * (*timerFnc) (void *););
void timerStop(Timer *T);
void start(Timer *T);
void startat(Timer *T, int year, int month, int day, int hour, int minute, int second);
void *error();
metrixArray * metrixArrayInit(int n);
void metrixArrayAdd(FILE *file_to_write , int value);


int main() {

//Available timer's period in mseconds
int period[3] = {1000, 100, 10};
int mode = 0;
printf("Timer Execution Options:\n");
printf("1 - 1 sec period\n");
printf("2 - 0.1 sec period\n");
printf("3 - 0.01 sec period\n");
printf("4 - All of the above\n");
printf("Select Mode: ");
scanf("%d", &mode);
if (mode!=1 && mode!=2 && mode!=3 && mode!=4) {
    printf("No such mode \n");// // Work function implementations

    exit(0);
}

int jobsToExecute = 0;
switch (mode)
{
  case 1:
  jobsToExecute = secondsToRun * (int)1e3 / period[0];

  break;
  case 2:
  jobsToExecute = secondsToRun * (int)1e3 / period[1];

  break;
  case 3:
  jobsToExecute = secondsToRun * (int)1e3 / period[2];

  break;
  case 4:
  jobsToExecute = secondsToRun * (int)1e3 / period[0] +  secondsToRun * (int)1e3 / period[1] +  secondsToRun * (int)1e3 / period[2];

  break;
}

  srand(time(NULL));
  queue * fifo;
  pthread_t *pro, *con;

f1 = fopen("TimeInQueue.txt", "w");
f2 = fopen("DriftTime.txt", "w");
f3 = fopen("JobExecTime.txt", "w");
f4 = fopen("ProdWaitTime.txt", "w");


//Fifo Initialization
  fifo = queueInit();
  if (fifo == NULL) {
    fprintf(stderr, "main: Queue Init failed.\n");
    exit(1);
  }

  pro = (pthread_t *) malloc( nOfProducers * sizeof(pthread_t) );
  con = (pthread_t *) malloc( nOfConsumers * sizeof(pthread_t) );


  tArg *proArgs, *conArgs;
  proArgs = (tArg *) malloc( nOfProducers * sizeof(tArg) );
  conArgs = (tArg *) malloc( nOfConsumers * sizeof(tArg) );

  Timer *T;

  if(pro == NULL || con == NULL || proArgs == NULL || conArgs == NULL ){
    fprintf(stderr, "Error at memory initialization.\n");
    exit(1);
  }

//Creating up the consumers / producers threads
  for (int i = 0; i < nOfConsumers; i++) {
    conArgs[i].Q = fifo;
    conArgs[i].helperMut = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    // elegxos gia memory leak
    pthread_mutex_init(conArgs[i].helperMut, NULL);
    pthread_create( &con[i], NULL, consumer, (void *)(conArgs + i));
  }
    int randF;
    randF = rand() % nOfFunctions;
  switch(mode){
    case 1:
    T = (Timer *)malloc(sizeof(Timer));
    T[0] = *timerInit(period[0],jobsToExecute , 0, fifo , producer, error , functions[randF] );
    start(T);
    break;
    case 2:
    T = (Timer *)malloc(sizeof(Timer));
    T[0] = *timerInit(period[1],jobsToExecute , 0, fifo , producer,error , functions[randF]);
    start(T);
    break;
    case 3:
    T = (Timer *)malloc(sizeof(Timer));
    T[0] = *timerInit(period[2],jobsToExecute , 0, fifo , producer,error, functions[randF]);
    start(T);
    break;
    case 4:
    T = (Timer *)malloc(3 * sizeof(Timer));
    // elegxos gia memory leak

    T[0] = *timerInit(period[0],secondsToRun * (int)1e3 / period[0] , 0, fifo , producer,error,functions[randF]);
    T[1] = *timerInit(period[1],secondsToRun * (int)1e3 / period[1] , 0, fifo , producer,error,functions[randF]);
    T[2] = *timerInit(period[2],secondsToRun * (int)1e3 / period[2] , 0, fifo , producer,error,functions[randF]);
    start((T+0));
    start((T+1));
    start((T+2));

    break;
  }


//Waiting for the threads to join
  for (int i = 0; i < nOfProducers; i++) {
    pthread_join(pro[i], NULL);
  }

  for (int i = 0; i < nOfConsumers; i++) {
    pthread_join(con[i], NULL);
  }

  switch(mode){
    case 1:
    timerStop(T);
    break;
    case 2:
    timerStop(T);
    break;
    case 3:
    timerStop(T);
    break;
    case 4:
    timerStop(T);
    break;
  }


    fclose(f1);
    fclose(f2);
    fclose(f3);
    fclose(f4);



  queueDelete(fifo);


  return 0;
}

//Producer thread function
void * producer(void * q) {

  queue * fifo;
  int i ,tid ;
  Timer *T = (Timer *)q;
  fifo = T->Q ;
  int  randAr;
  double totalDrift = 0;
  struct timeval  timeValue  , prodJobStart , prodJobEnd ;
  double previousInsert , nextInsert ;

  sleep(T->startDelay);
  gettimeofday(&timeValue, NULL);
  previousInsert = 1e6* timeValue.tv_sec + timeValue.tv_usec;
  nextInsert = previousInsert;


  for (i = 0; i < T->tasksToExecute; i++) {
    pthread_mutex_lock(fifo -> mut);
    gettimeofday(&prodJobStart, NULL);

    while (fifo -> full) {
      printf("The queue is full \n");
      //Error function execution
      T->errorFnc;
      pthread_cond_wait(fifo -> notFull, fifo -> mut);
    }
    gettimeofday(&timeValue, NULL);
    previousInsert = 1e6*timeValue.tv_sec + timeValue.tv_usec;

    //Workfunction object to add at the queue
    workFunction wf;

    //Randomly choose an argument
    randAr = rand() % nOfArguments;
    wf.arg  = & argu[randAr];
    wf.work = T->timerFnc;



    //Getting the arrival time at the queue
    gettimeofday(&(arrTime[(fifo->tail)]),NULL);
    gettimeofday(&prodJobEnd, NULL);

    queueAdd(fifo, wf);

    //Calculate the time taken for a producer to push a job to the queue
    int prodJobADD = (prodJobEnd.tv_sec-prodJobStart.tv_sec)*(int)1e6 + prodJobEnd.tv_usec-prodJobStart.tv_usec;
    metrixArrayAdd(f4, prodJobADD);
    //printf("Producer push waiting time : %d  \n " , prodJobADD);
    pthread_mutex_unlock(fifo -> mut);
    pthread_cond_signal(fifo -> notEmpty);

    //Calculate the driftTime
    double driftTime = previousInsert - nextInsert;
    metrixArrayAdd(f2,driftTime);
    //printf("Drift time : %d \n " , (int)driftTime);
    double sleepTime = T->period - driftTime/(int)1e3;
    if(sleepTime > 0){
      usleep(sleepTime*(int)1e3);
      //printf("Drift time : %lf \n" , sleepTime);
    }
    else{

    //printf("NO FUCKING SLEEP \n");
    }
    //Update time value

    gettimeofday(&timeValue, NULL);
    nextInsert = 1e6*timeValue.tv_sec + timeValue.tv_usec;
  }


  prodFinished++;
  //Termination condition for the consumers
  if (prodFinished == nOfProducers) {

    terminateConFlag = 1;
    //Broadcast for the locked consumers
    pthread_cond_broadcast(fifo -> notEmpty);
  }

  return (NULL);
}

// Consumer thread function
void * consumer(void * q) {

  queue * fifo;
  int i, d ;


  int waitingTime ;
  struct timeval  JobExecStart, JobExecEnd;

  tArg *conArg;
  conArg = (tArg *)q ;
  fifo = conArg->Q;


  while (1) {


    pthread_mutex_lock(fifo -> mut);

    while (fifo -> empty == 1 && terminateConFlag != 1) {

      //printf("The queue is empty \n");
      pthread_cond_wait(fifo -> notEmpty, fifo -> mut);
    }
    //Termination flag for the consumers
    if (terminateConFlag == 1 && fifo -> empty == 1) {
      pthread_mutex_unlock(fifo -> mut);
      pthread_cond_broadcast(fifo -> notEmpty);
      break;
    }

    //Workfunction object to remove from the queue
    workFunction wf;
    struct timeval leaveTime;
    //Getting the leave time from the queueegettimeofday(&JobExecStart,NULL);
    gettimeofday(&leaveTime,NULL);
    //Calculating the waiting time at the queue
    waitingTime= (leaveTime.tv_sec -(arrTime[fifo->head]).tv_sec) *1e6 + (leaveTime.tv_usec-(arrTime[fifo->head]).tv_usec) ;
   // printf("The waiting time is : %d  \n " , waitingTime);
    metrixArrayAdd(f1,waitingTime);
    queueDel(fifo, &wf);
    tempTime += waitingTime;


    pthread_mutex_unlock(fifo -> mut);
    pthread_cond_signal(fifo -> notFull);
    //Executing the work function outside the mutexes
    gettimeofday(&JobExecStart,NULL);
    wf.work(wf.arg);
    gettimeofday(&JobExecEnd,NULL);

    pthread_mutex_lock(conArg->helperMut);
    int JobDur = (JobExecEnd.tv_sec-JobExecStart.tv_sec)*(int)1e6 + JobExecEnd.tv_usec-JobExecStart.tv_usec;
  //  printf("Execution time is  : %d  \n " , JobDur);
    metrixArrayAdd(f3,JobDur);
    pthread_mutex_unlock(conArg->helperMut);


  }
//  pthread_cond_signal (fifo->notEmpty);
  return (NULL);
}

//Queue function implementations
queue * queueInit(void) {
  queue * q;

  q = (queue * ) malloc(sizeof(queue));
  if (q == NULL) return (NULL);

  q -> empty = 1;
  q -> full = 0;
  q -> head = 0;
  q -> tail = 0;
  q -> mut = (pthread_mutex_t * ) malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(q -> mut, NULL);
  q -> notFull = (pthread_cond_t * ) malloc(sizeof(pthread_cond_t));
  pthread_cond_init(q -> notFull, NULL);
  q -> notEmpty = (pthread_cond_t * ) malloc(sizeof(pthread_cond_t));
  pthread_cond_init(q -> notEmpty, NULL);

  return (q);
}

void queueDelete(queue * q) {
  pthread_mutex_destroy(q -> mut);
  free(q -> mut);
  pthread_cond_destroy(q -> notFull);
  free(q -> notFull);
  pthread_cond_destroy(q -> notEmpty);
  free(q -> notEmpty);
  free(q);
}

void queueAdd(queue * q, workFunction in ) {
  q -> buf[q -> tail] = in ;
  q -> tail++;
  if (q -> tail == QUEUESIZE)
    q -> tail = 0;
  if (q -> tail == q -> head)
    q -> full = 1;
  q -> empty = 0;

  return;
}

void queueDel(queue * q, workFunction * out) {
  * out = q -> buf[q -> head];

  q -> head++;
  if (q -> head == QUEUESIZE)
    q -> head = 0;
  if (q -> head == q -> tail)
    q -> empty = 1;
  q -> full = 0;

  return;
}

// Work function implementations

void * expo(void * args) {
  double x = ( * (int * ) args);
  double result;
  for(int i = 0 ;  i < nOfExec ; i++)
  result = exp(x / 180);
}



void * sinF(void * args) {
  double x = * (int * ) args;
  double ret, val;
  val = PI / 180;
  for(int i = 0; i <nOfExec; i++)
  ret = sin(x * val);
}


void * cosF(void * args) {
  double x = * (int * ) args;
  double ret, val;
  val = PI / 180;
  for(int i = 0; i<nOfExec ; i++)
  ret = cos(x * val);
}



//Timer initialization function
Timer *timerInit(int period, int tasksToExecute, int startDelay , queue *queue, void *(*producer)(void *arg),void *(*errorFnc)(), void * (*timerFnc) (void *)){
    printf("Initializing Timer\n");
    Timer *T = (Timer *) malloc( sizeof(Timer) );
    T->period = period;
    T->tasksToExecute = tasksToExecute;
    T->startDelay = startDelay;
    T->Q = queue;
    T->errorFnc = errorFnc;
    T->timerFnc = timerFnc;
    T->startFnc = producer;
    T->userData = NULL;
    // na valw tid

    return T;
}
//Function to start the timer
void start(Timer *T){
 pthread_create(&T->tid, NULL, T->startFnc, T);

}
//Function to start the function at a specific time
void startat(Timer *T, int year, int month, int day, int hour, int minute, int second){
int delay = 0;

//Get current time using tm struct from <time.h> library

struct tm start;
start.tm_year = year;
start.tm_mon = month;
start.tm_mday = day;
start.tm_hour = hour;
start.tm_min = minute;
start.tm_sec = second;

time_t currentTime = time(NULL);
delay = (int) difftime(currentTime, mktime(&start));

if(delay > 0){
  T->startDelay = delay;
  pthread_create(&T->tid, NULL, T->startFnc, T);
}
else{
  printf("Invalid timestamp \n");
  exit(0);
}
}

//Timer free
void timerStop(Timer *T){
  free( T );
}
//Timmer error function
void *error() {
    jobsLostCounter++;
}

metrixArray * metrixArrayInit(int n){
  metrixArray *metr = (metrixArray *) malloc( sizeof(metrixArray) );
  if( metr==NULL ) {
    printf("Error at memory allocation \n");
  }
  metr->index = 0;
  metr->buffer = (int *)malloc(sizeof(int)*n);
  if(metr->buffer == NULL){
    printf("Error at memory allocation \n");
  }

  return metr;
}

void metrixArrayAdd(FILE *file_to_write , int value){

  fprintf(file_to_write, "%d\n", value );

}
