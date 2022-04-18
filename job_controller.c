#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <math.h>

#include "job_controller.h"

//job queue helper functions
int isJobQueueEmpty()
{
	return (jq->head == NULL);
}

//adds new job to queue
void addToJobQueue(JOB* newJob)
{
	pthread_mutex_lock(&jobQueueMutex);

	JQ_NODE* newNode = malloc(sizeof(JQ_NODE));
	newNode->job = newJob;
	newNode->next = NULL;
	if (jq->head == NULL)
	{
		jq->head = newNode;
		jq->tail = newNode;
		pthread_mutex_unlock(&jobQueueMutex);
		return;
	}
	jq->tail->next = newNode;
	jq->tail = newNode;
	pthread_mutex_unlock(&jobQueueMutex);

}

//returns the next job in queue
JOB* getNextJobInQueue()
{
	pthread_mutex_lock(&jobQueueMutex);
	if (jq->head == NULL)
	{
		pthread_mutex_unlock(&jobQueueMutex);
		return NULL;
	}
	JOB* retNode = jq->head->job;
	JQ_NODE* oldHeadNode = jq->head;
	jq->head = oldHeadNode->next;
	free(oldHeadNode);

	pthread_mutex_unlock(&jobQueueMutex);

	return retNode;
}


//jobs in execution
int isSchedulerFree()
{
	pthread_mutex_lock(&runningJobsMutex);

	int ret = ffs(runningJobs->runningJobsIndex);

	pthread_mutex_unlock(&runningJobsMutex);
	return ret;

}

//start job
void startJob(JOB* job, int index)
{
	int pid = fork();
	if (pid < 0)
	{
		printf("Failed to fork ");
	}
	else if (pid == 0)
	{
		//child process
		char* arr[64];
		arr[0] = job->job_command;
		int i = 1;
		int o = 0;
		for (o; job->args[o] != NULL; i++, o++)
		{
			arr[i] = job->args[o];
		}
		arr[i] = NULL;
		char jobIdString[100];
		sprintf(jobIdString, "%d", job->job_id);

		//std output for job
		char* stdOutFilePath = malloc(25);
		strcpy(stdOutFilePath, jobIdString);
		strcat(stdOutFilePath, ".out");

		//std error for job
		char* stdErrFilePath = malloc(25);
		strcpy(stdErrFilePath, jobIdString);
		strcat(stdErrFilePath, ".err");

		int fd1 = open(stdOutFilePath, O_CREAT | O_WRONLY);
		int fd2 = open(stdErrFilePath, O_CREAT | O_WRONLY);

		// redirect stdout and stderr
		dup2(fd1, 1);
		dup2(fd2, 2);

		//execute command on file
		execvp(job->job_command, arr);
	}
	else
	{
		pthread_mutex_lock(&runningJobsMutex);

		runningJobs->runningJobs[index] = job;

		runningJobs->runningJobsIndex = runningJobs->runningJobsIndex & (~(1 << index));

		runningJobs->runningJobsPid[index] = pid;
		job->status = RUNNING;
		pthread_mutex_unlock(&runningJobsMutex);
	}
}
//the main scheduler function which periodically checks the running jobs and removes finished jobs
void* scheduler(void* arg)
{
	// check if any of the running jobs have finished
	// if finished the clear up the core to add new job
	while (1)
	{
		int stop = stopScheduler && isJobQueueEmpty();

		pthread_mutex_lock(&runningJobsMutex);

		//printf("checking index : [%d]\n", runningJobs->runningJobsIndex);
		int i = 0;
		for (i; i < runningJobs->maxJobs; i++)
		{
			int status;
			int current_pid = runningJobs->runningJobsPid[i];
			//printf("current_pid = [%d]\n",current_pid);
			if (current_pid <= 0)
				continue;
			pid_t return_pid = waitpid(current_pid, &status, WNOHANG); /* WNOHANG def'd in wait.h */
			if (return_pid == -1) {
				printf("Unable to know status of pid %d\n", current_pid);
			}
			else if (return_pid == 0) {
				if (stop)
					i--;

				continue;
			}
			else if (return_pid == current_pid) {
				runningJobs->runningJobsIndex |= (1 << i);
				runningJobs->runningJobsPid[i] = 0;
				freeUpJob(runningJobs->runningJobs[i]);
				runningJobs->runningJobs[i] = NULL;
			}
			if (runningJobs->runningJobs[i] == NULL)
			{
				runningJobs->runningJobsIndex |= (1 << i);
			}
		}
		pthread_mutex_unlock(&runningJobsMutex);

		if (stop)
			break;

		int freeIndex = 0;
		while (!isJobQueueEmpty() && (freeIndex = isSchedulerFree()) != 0)
		{
			//check for jobs from queue
			JOB* job = getNextJobInQueue();
			//printf("Starting new job with job id : [%d]\n\n", job->job_id);

			startJob(job, freeIndex - 1);
		}
		sleep(1);
	}
	return NULL;
}

void submitJob(char* jobName)
{
	JOB* newJob = createNewJob(jobName);
	addToJobQueue(newJob);
}

void showJobs()
{

	printf("JOBID\tJOB NAME\t\t\tSTATUS\n");
	printf("--------------------------------------------------------------------------------------------------\n");
	pthread_mutex_lock(&runningJobsMutex);
	int i = 0;
	for (i; i < runningJobs->maxJobs; i++)
	{
		if (runningJobs->runningJobs[i] != NULL)
		{
			printf("%d\t%s\t\t\t%s\n", runningJobs->runningJobs[i]->job_id, runningJobs->runningJobs[i]->job_name, runningJobs->runningJobs[i]->status == WAITING ? "WAITING" : "RUNNING");
		}
	}
	pthread_mutex_unlock(&runningJobsMutex);

	pthread_mutex_lock(&jobQueueMutex);
	JQ_NODE* tempNode = jq->head;
	for (; tempNode != NULL; tempNode = tempNode->next)
	{
		printf("%d\t%s\t\t\t%s\n", tempNode->job->job_id, tempNode->job->job_name, tempNode->job->status == WAITING ? "WAITING" : "RUNNING");
	}
	pthread_mutex_unlock(&jobQueueMutex);
}

// main function to initialize entire job controller
void initializeJobController(int maxJobs)
{

	// initialize the runningJobs 
	pthread_mutex_init(&runningJobsMutex, NULL);

	runningJobs = malloc(sizeof(RUNNING_JOBS));
	runningJobs->maxJobs = maxJobs;
	runningJobs->runningJobs = malloc(sizeof(JOB*) * maxJobs);
	runningJobs->runningJobsPid = malloc(sizeof(pid_t) * maxJobs);
	memset(runningJobs->runningJobsPid, 0, sizeof(pid_t) * maxJobs);

	runningJobs->runningJobsIndex = (int)pow(2, maxJobs) - 1;

	//initialize the jobQueue
	pthread_mutex_init(&jobQueueMutex, NULL);
	jq = malloc(sizeof(JQ));
	jq->head = NULL;
	jq->tail = NULL;
}
//call at end of scheduling
void freeJobController()
{
	int i = 0;
	for (i; i < runningJobs->maxJobs; i++)
	{
		if (runningJobs->runningJobs[i] != NULL)
			freeUpJob(runningJobs->runningJobs[i]);
	}
	free(runningJobs->runningJobsPid);
	free(runningJobs->runningJobs);
	free(runningJobs);
	free(jq);
}

