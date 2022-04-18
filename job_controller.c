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
		for (int o = 0; job->args[o] != NULL; i++, o++)
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