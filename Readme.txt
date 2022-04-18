About
---------------
This is a simulation of a scheduler which can execute given jobs in parallel
Given an input number of cores, that number of jobs can run in parallel
Uses a linked list based queue which is used for holding jobs if the scheduler is busy
The scheduler automatically picks up jobs from the queue and starts executing them.
Once a job is finished it is removed from the running jobs and a new job is selected for execution
The scheduler is run on a different thread.
The access to shared data is maitained by mutex locks
The running jobs will print stdout and stderr to seprate files jobid.out and jobid.err in same location as program.


to Compile   -  gcc job.c job_controller.c main.c -lpthread -lm -Wall -o scheduler

to execute :-   ./scheduler <number of cores>


Commands in scheduler prompt
-----------------------------
 <program name> <number of cores> : Start the program with specified number of cores

 submit <job path> <arguments to the job> : Add the job to scheduler. If scheduler is free it will start the job.

 showjobs : Will list all the jobs in the scheduler. If job has been completed it wont be listed.Only running or waiting jobs.

 help : Prints info about scheduler and usage

 exit : Will stop scheduler. All running and waiting jobs will be completed before exiting. Prompt will not be shown further.



