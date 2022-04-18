#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>

#include"job_controller.h"


void showCommandList()
{
	printf("\t\t\tWelcome to scheduler !\n");
	printf("\t---------------------------------------------------------------------------\n");
	printf("\t\tThis is a simulation of a scheduler which can execute given jobs in parallel\n");
	printf("\t\tGiven an input number of cores, that number of jobs can run in parallel\n");
	printf("\t\tUses a linked list based queue which is used for holding jobs if the scheduler is busy\n");
	printf("\t\tThe scheduler automatically picks up jobs from the queue and starts executing them.\n");
	printf("\t\tOnce a job is finished it is removed from the running jobs and a new job is selected for execution\n");
	printf("\t\tThe scheduler is run on a different thread.\n");
	printf("\t\tThe access to shared data is maitained by mutex locks\n");
	printf("\t\tThe running jobs will print stdout and stderr to seprate files jobid.out and jobid.err in same location as program.\n");
	printf("\t------------------ List of Commands --------------------\n");
	printf("\t <program name> <number of cores> :\t\t Start the program with specified number of cores\n");
	printf("\t------------------ Inside scheduler prompt ------------------- \n");
	printf("\t submit <job path> <arguments to the job> :\t\t Add the job to scheduler. If scheduler is free it will start the job.\n");
	printf("\t showjobs :\t\t Will list all the jobs in the scheduler. If job has been completed it wont be listed.Only running or waiting jobs.\n");
	printf("\t help :\t\t Prints info about scheduler and usage\n");
	printf("\t exit :\t\t Will stop scheduler. All running and waiting jobs will be completed before exiting. Prompt will not be shown further.\n");

}

int parseInput(char* input)
{
	char* inputStringCopy = malloc(strlen(input) + 1);
	strcpy(inputStringCopy, input);

	// get the command passed 
	char* command = strtok(input, " ");
	char* newline = strchr(command, '\n');
	if (newline)
		*newline = 0;
	if (!strcmp(command, "submit"))
	{
		// get rest of input which is job info
		char* jobInfo = strchr(inputStringCopy, ' ');
		if (jobInfo == NULL)
		{
			printf("No job passed with submit command!\n");
			free(inputStringCopy);
			return 0;
		}
		addToJobQueue(createNewJob(jobInfo));
	}
	else if (!strcmp(command, "showjobs"))
	{
		showJobs();
	}
	else if (!strcmp(command, "help"))
	{
		showCommandList();
	}
	else if (!strcmp(command, "exit"))
	{
		free(inputStringCopy);
		stopScheduler = 1;
		return 1;
	}
	else
	{
		printf("wrong command !\n");
	}
	free(inputStringCopy);
	return 0;
}
