/** @file libscheduler.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libscheduler.h"
#include "../libpriqueue/libpriqueue.h"


/**
  Stores information making up a job to be scheduled including any statistics.

  You may need to define some global variables or a struct to store your job
	queue elements.
*/

//this structure will be the data to put in the queue
typedef struct _job_t
{
	int job_number;
	int arrival_time;
	int priority;
	int coreNum;
	int start_time;
	int running_time;
	int remaining_time;
	int last_start_time;

} job_t;

float avg_waiting_time;
float avg_response_time;
float avg_turnaround_time;
int totalJobs;

scheme_t currScheme;
int numCores;
int *coresArr;

priqueue_t Queue;

int compareArrival(const void * a, const void * b)
{
	const job_t *p = a, *q = b;
	return(p->arrival_time - q->arrival_time);
}
int compareBurst(const void * a, const void * b)
{
	const job_t *p = a, *q = b;
	int sol = p->remaining_time - q->remaining_time;
	//if a tie, compare arrival times
	if(sol == 0)
	{
		sol = p->arrival_time - q->arrival_time;
	}

	return(sol);
}
int comparePriority(const void * a, const void * b)
{
	const job_t *p = a, *q = b;
	int sol = p->priority - q->priority;
	//if a tie, compare arrival times
	if(sol == 0)
	{
		sol = p->arrival_time - q->arrival_time;
	}
	return(sol);
}


/**
  Initalizes the scheduler.

  Assumptions:
    - You may assume this will be the first scheduler function called.
    - You may assume this function will be called once once.
    - You may assume that cores is a positive, non-zero number.
    - You may assume that scheme is a valid scheduling scheme.

  @param cores the number of cores that is available by the scheduler.
	These cores will be known as core(id=0), core(id=1), ..., core(id=cores-1).
  @param scheme  the scheduling scheme that should be used. This value will be
	one of the six enum values of scheme_t
*/
void scheduler_start_up(int cores, scheme_t scheme)
{
	currScheme  = scheme;
	numCores    = cores;
	coresArr    = malloc(cores * sizeof(int));
	int i;
	for(i = 0 ; i < numCores ; i++)
	{
		coresArr[i]    = -1;
	}

	if(currScheme == PRI)
		priqueue_init(&Queue, comparePriority);
	else if(currScheme == PPRI)
		priqueue_init(&Queue, comparePriority);
	else if(currScheme == SJF)
		priqueue_init(&Queue, compareBurst);
	else if(currScheme == PSJF)
		priqueue_init(&Queue, compareBurst);
	else
		priqueue_init(&Queue, compareArrival);

	totalJobs           = 0;
	avg_waiting_time    = 0.0;
	avg_response_time   = 0.0;
	avg_turnaround_time = 0.0;
}


/**
  Called when a new job arrives.

  If multiple cores are idle, the job should be assigned to the core with the
  lowest id.
  If the job arriving should be scheduled to run during the next
  time cycle, return the zero-based index of the core the job should be
  scheduled on. If another job is already running on the core specified,
  this will preempt the currently running job.
  Assumption:
    - You may assume that every job wil have a unique arrival time.

  @param job_number a globally unique identification number of the job arriving.
  @param time the current time of the simulator.
  @param running_time the total number of time units this job will run before
	it will be finished.
  @param priority the priority of the job. (The lower the value, the higher the
	priority.)
  @return index of core job should be scheduled on
  @return -1 if no scheduling changes should be made.

 */
int scheduler_new_job(int job_number, int time, int running_time, int priority)
{
	job_t* newJob           = malloc(sizeof(job_t));
	newJob->job_number      = job_number;
	newJob->arrival_time    = time;
	newJob->running_time    = running_time;
	newJob->remaining_time  = running_time;
	newJob->priority        = priority;
	//-1 for idle
	newJob->coreNum         = -1;
	newJob->start_time      = -1;
	newJob->last_start_time = -1;

	totalJobs++;

	//if it's FCFS, the data field that should be compared within the queue
	//is going to be the time (or arrival_time)
	if(currScheme == FCFS || currScheme == PRI || currScheme == SJF)
	{
		int i,j;
		int size = priqueue_size(&Queue);
		//loop through the cores and see if there is space
		for(i = 0 ; i < numCores ; i++)
		{
			int coreInUse = 0;
			for(j = 0 ; j < size ; j++)
			{
				job_t *temp = priqueue_at(&Queue, j);
				if(temp->coreNum == i)
				{
					coreInUse = 1;
					j = size;
				}
			}
			//core not being used, put new job on it
			if(coreInUse == 0)
			{
				newJob->coreNum         = i;
				newJob->start_time      = time;
				newJob->last_start_time = time;
				priqueue_offer(&Queue, newJob);
				return i;
			}
		}
		//no availabe cores, put the node in the queue
		priqueue_offer(&Queue, newJob);
		return -1;
	}
	else if(currScheme == PPRI)
	{
		int currHighestPri = -1;
		int indexOfHigh    = -1;
		int arrival        = 0;
		int i,j;
		int size           = priqueue_size(&Queue);
		//loop through the cores and see if there is space
		for(i = 0 ; i < numCores ; i++)
		{
			int coreInUse = 0;
			for(j = 0 ; j < size ; j++)
			{
				job_t *temp = priqueue_at(&Queue, j);
				if(temp->coreNum == i)
				{
					if(temp->priority > currHighestPri)
					{
						currHighestPri = temp->priority;
						indexOfHigh = j;
					}
					coreInUse = 1;
					j = size;
				}
			}
			//core not being used, put new job on it
			if(coreInUse == 0)
			{
				newJob->coreNum         = i;
				newJob->start_time      = time;
				newJob->last_start_time = time;
				priqueue_offer(&Queue, newJob);
				return i;
			}
		}
		//all the cores are occupied
		//we stored the highest priority

		job_t *temp = priqueue_at(&Queue, indexOfHigh);
		if(temp->priority > newJob->priority)
		{
			temp                    = priqueue_remove_at(&Queue, indexOfHigh);
			int progressTime        = time - temp->last_start_time;
			temp->remaining_time    = temp->remaining_time - progressTime;
			int coreIndex           = temp->coreNum;
			temp->coreNum           = -1;
			if(temp->start_time == time)
			{
				temp->start_time      = -1;
			}
			temp->last_start_time   = -1;
			priqueue_offer(&Queue, temp);

			newJob->coreNum         = coreIndex;
			newJob->start_time      = time;
			newJob->last_start_time = time;
			priqueue_offer(&Queue, newJob);
			return coreIndex;
		}
		// no cores available and the running jobs have higher priority
		priqueue_offer(&Queue, newJob);
		return -1;
	}
	else if(currScheme == PSJF)
	{
		int currLongest    = -1;
		int indexOfLong    = -1;
		int arrival        = 0;
		int i,j;
		int size           = priqueue_size(&Queue);
		//loop through the cores and see if there is space
		for(i = 0 ; i < numCores ; i++)
		{
			int coreInUse = 0;
			for(j = 0 ; j < size ; j++)
			{
				job_t *temp = priqueue_at(&Queue, j);
				if(temp->coreNum == i)
				{
					if(temp->remaining_time >= currLongest)
					{
						//need to check the arrival times
						if(temp->remaining_time == currLongest)
						{
							if(temp->arrival_time > arrival)
							{
								arrival = temp->arrival_time;
								currLongest = temp->remaining_time;
								indexOfLong = j;
							}
						}
						else
						{
							arrival = temp->arrival_time;
							currLongest = temp->remaining_time;
							indexOfLong = j;
						}
					}
					coreInUse = 1;
					j = size;
				}
			}
			//core not being used, put new job on it
			if(coreInUse == 0)
			{
				newJob->coreNum         = i;
				newJob->start_time      = time;
				newJob->last_start_time = time;
				priqueue_offer(&Queue, newJob);
				return i;
			}
		}
		//all the cores are occupied
		//we stored the highest priority

		job_t *temp = priqueue_at(&Queue, indexOfLong);
		if(temp->remaining_time > newJob->remaining_time)
		{
			printf("temp->remaining_time %d\n", temp->remaining_time);
			printf("newJob->remaining_time %d\n", newJob->remaining_time);
			temp                    = priqueue_remove_at(&Queue, indexOfLong);
			int progressTime        = time - temp->last_start_time;
			temp->remaining_time    = temp->remaining_time - progressTime;
			int coreIndex           = temp->coreNum;
			temp->coreNum           = -1;
			if(temp->start_time == time)
			{
				temp->start_time      = -1;
			}
			temp->last_start_time   = -1;
			priqueue_offer(&Queue, temp);

			newJob->coreNum         = coreIndex;
			newJob->start_time      = time;
			newJob->last_start_time = time;
			priqueue_offer(&Queue, newJob);
			return coreIndex;
		}
		// no cores available and the running jobs have higher priority
		priqueue_offer(&Queue, newJob);
		return -1;

	}
	else if(currScheme == RR)
	{

	}

	return -1;
}


/**
  Called when a job has completed execution.

  The core_id, job_number and time parameters are provided for convenience.
	You may be able to calculate the values with your own data structure.
  If any job should be scheduled to run on the core free'd up by the
  finished job, return the job_number of the job that should be scheduled to
  run on core core_id.

  @param core_id the zero-based index of the core where the job was located.
  @param job_number a globally unique identification number of the job.
  @param time the current time of the simulator.
  @return job_number of the job that should be scheduled to run on core core_id
  @return -1 if core should remain idle.
 */
int scheduler_job_finished(int core_id, int job_number, int time)
{
	//find the job on the core
	job_t *temp;
	int i;
	int size = priqueue_size(&Queue);
	for( i = 0 ; i < size ; i++)
	{
		temp = priqueue_at(&Queue, i);
		if(temp->coreNum == core_id)
		{
			break;
		}
	}
	temp = priqueue_remove_at(&Queue, i);

	//temp points the job that just finished, get some stats
	avg_waiting_time    += temp->start_time - temp->arrival_time;
	avg_response_time   += time - temp->running_time - temp->arrival_time;
	avg_turnaround_time += time - temp->arrival_time;

	//job finished, free the assets
	free(temp);

	//search the queue for non running jobs, put the highest 'priority' on a core
	size = priqueue_size(&Queue);
	for(i = 0 ; i < size ; i++)
	{
		temp = priqueue_at(&Queue, i);
		//highest priority val to put on
		if(temp->coreNum < 0)
		{
			if(temp->start_time < 0)
			{
				temp->start_time = time;
			}
			temp->coreNum = core_id;
			temp->last_start_time = time;
			return temp->job_number;
		}
	}
	//else there's no idle jobs
	return -1;
}


/**
  When the scheme is set to RR, called when the quantum timer has expired
  on a core.

  If any job should be scheduled to run on the core free'd up by
  the quantum expiration, return the job_number of the job that should be
  scheduled to run on core core_id.

  @param core_id the zero-based index of the core where the quantum has expired.
  @param time the current time of the simulator.
  @return job_number of the job that should be scheduled on core cord_id
  @return -1 if core should remain idle
 */
int scheduler_quantum_expired(int core_id, int time)
{
	return -1;
}


/**
  Returns the average waiting time of all jobs scheduled by your scheduler.

  Assumptions:
    - This function will only be called after all scheduling is complete (all
		jobs that have arrived will have finished and no new jobs will arrive).
  @return the average waiting time of all jobs scheduled.
 */
float scheduler_average_waiting_time()
{
	return avg_waiting_time / totalJobs;
}


/**
  Returns the average turnaround time of all jobs scheduled by your scheduler.

  Assumptions:
    - This function will only be called after all scheduling is complete
		(all jobs that have arrived will have finished and no new jobs will arrive).
  @return the average turnaround time of all jobs scheduled.
 */
float scheduler_average_turnaround_time()
{
	return avg_turnaround_time / totalJobs;
}


/**
  Returns the average response time of all jobs scheduled by your scheduler.

  Assumptions:
    - This function will only be called after all scheduling is complete
		(all jobs that have arrived will have finished and no new jobs will arrive).
  @return the average response time of all jobs scheduled.
 */
float scheduler_average_response_time()
{
	return avg_response_time / totalJobs;
}


/**
  Free any memory associated with your scheduler.

  Assumption:
    - This function will be the last function called in your library.
*/
void scheduler_clean_up()
{
	priqueue_destroy(&Queue);
	free(coresArr);
}


/**
  This function may print out any debugging information you choose. This
  function will be called by the simulator after every call the simulator
  makes to your scheduler.
  In our provided output, we have implemented this function to list the jobs in
	the order they are to be scheduled. Furthermore, we have also listed the
	current state of the job (either running on a given core or idle).
	For example, if we have a non-preemptive algorithm and job(id=4) has began
	running, job(id=2) arrives with a higher priority, and job(id=1) arrives with
	a lower priority, the output in our sample output will be:

    2(-1) 4(0) 1(-1)

  This function is not required and will not be graded. You may leave it
  blank if you do not find it useful.
 */
void scheduler_show_queue()
{
	int size = priqueue_size(&Queue);
	int i;
	for(i = 0 ; i < size ; i++)
	{
		job_t *temp = priqueue_at(&Queue, i);
		printf("%d(%d) ", temp->job_number, temp->priority);
	}
}
