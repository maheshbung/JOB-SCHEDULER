#ifndef HEADER_GUARD_JOB_CONTROLLER
#define HEADER_GUARD_JOB_CONTROLLER

#include "job.h"
#include <pthread.h>
#include <unistd.h>

typedef struct jobQueueNode
{
	JOB* job;
	struct jobQueueNode* next;
}JQ_NODE;

typedef struct jobQueue
{
	JQ_NODE* head;
	JQ_NODE* tail;
}JQ;


JQ* jq;


pthread_mutex_t jobQueueMutex;


int isJobQueueEmpty();
void addToJobQueue(JOB* newJob);
JOB* getNextJobInQueue();



typedef struct runningJobsStruct
{
	int maxJobs;
	unsigned int runningJobsIndex;
	JOB** runningJobs;
	pid_t* runningJobsPid;  //array of pid
}RUNNING_JOBS;


RUNNING_JOBS* runningJobs;


pthread_mutex_t runningJobsMutex;


int stopScheduler;

void submitJob(char* jobName);
int isSchedulerFree();
void startJob(JOB* job, int index);



void* scheduler(void*);


void showJobs();

// Job controller funcs
void initializeJobController(int maxJobs);
void freeJobController();

#endif
