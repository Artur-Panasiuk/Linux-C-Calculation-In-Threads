#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>

long double globalResult = 1;
pthread_mutex_t lock;

struct args{
  long unsigned int start;
  long unsigned int end;
};

long double wallisInRange(long unsigned int start, long unsigned int end){
  long double result = 1;
  if(start < 1) return result;

  for(long unsigned int n = start; n <= end; n++){
    result = result * ((2*n) * (2*n)) / (((2*n) - 1) * ((2*n) + 1));
  }
  
  return result;
}

void* test(void *input){
  long unsigned int a = ((struct args*)input)->start;
  long unsigned int b = ((struct args*)input)->end;

  long double *wallis = malloc(sizeof(long double));
  *wallis = wallisInRange(a,b);

  pthread_mutex_lock(&lock);
  globalResult *= wallisInRange(a,b);
  pthread_mutex_unlock(&lock);

  return (void*)wallis;
}

int main(int argc, char* argv[]){

  if(argc < 3 || argc > 5) return 1;

  int n = atoi(argv[1]);
  int w = atoi(argv[2]);

  if(n <= 1 || n >= 1000000000) return 1;
  if(w <= 1 || w >= 100) return 1;

  struct args **argArr = malloc(w * sizeof(struct args*));
  pthread_t *tids = malloc(w * sizeof(pthread_t));
  void **exits = malloc(w * sizeof(void*));
  
  clock_t wt1, wt2, wot1, wot2;
  double time1, time2;
  
  wt1 = clock();

  if(n%w == 0){
    long unsigned int siz, jump = n/w;

    long unsigned int first = 1;
    long unsigned int last = jump;

    for(int i = 0; i < w; i++){
      argArr[i] = (struct args *)malloc(sizeof(struct args));
      argArr[i]->start  = first;
      argArr[i]->end = last;

      siz = last-first + 1;

      first += jump;
      last += jump;

      pthread_create(&tids[i], NULL, test, (void *)argArr[i]);
      printf("Thread #%lu size=%lu first=%lu\n", tids[i], siz, argArr[i]->start);
    }

    for(int j = 0; j < w; j++){
      pthread_join(tids[j], &exits[j]);
      printf("Thread #%lu prod=%Lf\n", tids[j], *(long double*)exits[j]);
    }
    //////////////////// ELSE ///////////////////////
  }else{
    long unsigned int siz, diff = n%w;
    long unsigned int jump = (n - diff)/w;

    long unsigned int first = 1;
    long unsigned int last = jump;

    for(int i = 0; i < w-1; i++){
      argArr[i] = (struct args *)malloc(sizeof(struct args));
      argArr[i]->start  = first;
      argArr[i]->end = last;

      siz = last-first + 1;

      first += jump;
      last += jump;

      pthread_create(&tids[i], NULL, test, (void *)argArr[i]);
      printf("Thread #%lu size=%lu first=%lu\n", tids[i], siz, argArr[i]->start);
    }
    argArr[w-1] = (struct args *)malloc(sizeof(struct args));
    argArr[w-1]->start  = first;
    argArr[w-1]->end = n;
    siz = argArr[w-1]->end - argArr[w-1]->start + 1;
    pthread_create(&tids[w-1], NULL, test, (void *)argArr[w-1]);
    printf("Thread #%lu size=%lu first=%lu\n", tids[w-1], siz, argArr[w-1]->start);

    for(int j = 0; j < w; j++){
      pthread_join(tids[j], &exits[j]);
      printf("Thread #%lu prod=%Lf\n", tids[j], *(long double*)exits[j]);
    }
  }
  
  wt2 = clock();

  pthread_mutex_destroy(&lock);

  free(argArr);
  free(tids);
  free(exits);
  
  wot1 = clock();
  
  long double woThreadsWallis = wallisInRange(1,n);
  
  wot2 = clock();
  
  time1 = ((double)(wt2-wt1)) / CLOCKS_PER_SEC;
  time2 = ((double)(wot2-wot1)) / CLOCKS_PER_SEC;
  
  printf("w/Threads: PI=%Lf time=%fs\n", (globalResult*2), time1);
  printf("wo/Threads: PI=%Lf time=%fs\n", (woThreadsWallis*2), time2);

  return 0;
}
