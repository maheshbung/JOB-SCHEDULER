
#include<string.h>
#include<stdio.h>
#include<stdlib.h>

#include"job.h"

JOB* createNewJob(char* jobInfo)
{
	static int jobIdCounter = 1;
	JOB* newJob = malloc(sizeof(JOB));
	if (newJob == NULL)
	{
		printf("Failed to allocate memory for new JOB");
		return NULL;
	}
	newJob->job_id = jobIdCounter++;
	newJob->status = WAITING;
	newJob->job_name = malloc(strlen(jobInfo) + 1);
	if (newJob->job_name == NULL)
	{
		printf("Failed to allocate memory for job_name");
		return NULL;
	}
	newJob->job_name[0] = '\0';
	strcpy(newJob->job_name, jobInfo);
	parseJobCommand(newJob);
	printf("Created new job successfully with job id : [%d]!\n", newJob->job_id);
	return newJob;
}

void parseJobCommand(JOB* job)
{
	char* temp_command = job->job_name;
	job->job_command = strdup(strtok(temp_command, " "));
	job->args = malloc(15 * sizeof(char*));
	if (job->args == NULL)
	{
		printf("Failed to allocate memory for job_name");
		return;
	}
	int i = 0;
	if (job->job_command != NULL)
	{
		char* temp = strtok(NULL, " ");
		while (temp != NULL)
		{
			job->args[i] = temp;
			temp = strtok(NULL, " ");
			i++;
		}
	}
}

void freeUpJob(JOB* job)
{
	free(job->job_name);
	free(job->job_command);
	free(job->args);
	free(job);
}
