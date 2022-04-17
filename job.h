#ifndef HEADER_GUARD_JOB
#define HEADER_GUARD_JOB

typedef enum jobStatus
{
	READY,
	RUNNING,
	WAITING,
	FINISHED
}JOB_STATUS;

typedef struct job
{
	int job_id;
	char* job_name;
	JOB_STATUS status;
	char* job_command;
	char** args;
}JOB;

JOB* createNewJob(char* jobName);
void parseJobCommand(JOB* job);
void freeUpJob(JOB* job);
#endif