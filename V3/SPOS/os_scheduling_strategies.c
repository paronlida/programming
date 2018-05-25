#include "os_scheduling_strategies.h"
#include "defines.h"

#include <stdlib.h>

SchedulingInformation SchedulingInfo;

/*!
 *  Reset the scheduling information for a specific strategy
 *  This is only relevant for RoundRobin and InactiveAging
 *  and is done when the strategy is changed through os_setSchedulingStrategy
 *
 * \param strategy  The strategy to reset information for
 */
void os_resetSchedulingInformation(SchedulingStrategy strategy) {
	
	if(strategy == OS_SS_ROUND_ROBIN){
		SchedulingInfo.Zeitschiebe = os_getProcessSlot(os_getCurrentProc())->priority;
	}
	else if(strategy == OS_SS_INACTIVE_AGING){
		for(int i = 0; i < MAX_NUMBER_OF_PROCESSES; i++){
			SchedulingInfo.age[i] = 0;
		}
	}
	
}

/*!
 *  Reset the scheduling information for a specific process slot
 *  This is necessary when a new process is started to clear out any
 *  leftover data from a process that previously occupied that slot
 *
 *  \param id  The process slot to erase state for
 */
void os_resetProcessSchedulingInformation(ProcessID id) {
    SchedulingInfo.age[id] = 0;
}

/*!
 *  This function implements the even strategy. Every process gets the same
 *  amount of processing time and is rescheduled after each scheduler call
 *  if there are other processes running other than the idle process.
 *  The idle process is executed if no other process is ready for execution
 *
 *  \param processes An array holding the processes to choose the next process from.
 *  \param current The id of the current process.
 *  \return The next process to be executed determined on the basis of the even strategy.
 */
ProcessID os_Scheduler_Even(Process const processes[], ProcessID current) 
{
	uint8_t i;
	uint8_t counter= 0;
	if (current == MAX_NUMBER_OF_PROCESSES-1)
	{
		i = 1;
	} 
	else
	{
		i = current + 1;
	}
	
	do 
	{
		if (processes[i].state == OS_PS_READY)
		{
			return i;
		}
		
		if (i == MAX_NUMBER_OF_PROCESSES-1)
		{
			i = 1;
		} 
		else
		{
			i = i+1;
		}
		counter++;
	} while (counter < 6);
	
	return 0;
}

/*!
 *  This function implements the random strategy. The next process is chosen based on
 *  the result of a pseudo random number generator.
 *
 *  \param processes An array holding the processes to choose the next process from.
 *  \param current The id of the current process.
 *  \return The next process to be executed determined on the basis of the random strategy.
 */
ProcessID os_Scheduler_Random(Process const processes[], ProcessID current) 
{
	uint8_t i = 0;
	uint8_t numOfReadyProc = os_getNumberOfReadyProcs();
	uint8_t procIndex[numOfReadyProc];
	
	for (uint8_t j = 1; j < MAX_NUMBER_OF_PROCESSES; j++)
	{
		if (processes[j].state == OS_PS_READY)
		{
			procIndex[i] = j;
			i++;
		}
	}
	
	if (i == 0)
	{
		return 0;
	}
	else
	{
		uint8_t a = rand() % numOfReadyProc;
		return procIndex[a];
	}
}

/*!
 *  This function implements the round-robin strategy. In this strategy, process priorities
 *  are considered when choosing the next process. A process stays active as long its time slice
 *  does not reach zero. This time slice is initialized with the priority of each specific process
 *  and decremented each time this function is called. If the time slice reaches zero, the even
 *  strategy is used to determine the next process to run.
 *
 *  \param processes An array holding the processes to choose the next process from.
 *  \param current The id of the current process.
 *  \return The next process to be executed determined on the basis of the round robin strategy.
 */
ProcessID os_Scheduler_RoundRobin(Process const processes[], ProcessID current) {

		uint8_t i;
		uint8_t counter= 0;
		
		if(SchedulingInfo.Zeitschiebe > 0)
		{
			SchedulingInfo.Zeitschiebe --;
			return current;
		}
		
		else
		{
		if (current == MAX_NUMBER_OF_PROCESSES-1)
		{
			i = 1;
		}
		else
		{	
			i = current + 1;
		}
		
		do
		{
			if (processes[i].state == OS_PS_READY)
			{	
				SchedulingInfo.Zeitschiebe = processes[i].priority;
				return i;
			}
			
			if (i == MAX_NUMBER_OF_PROCESSES-1)
			{
				i = 1;
			}
			else
			{
				i = i+1;
			}
			counter++;
		} while (counter < 6);
		}

		return 0;
	}



/*!
 *  This function realizes the inactive-aging strategy. In this strategy a process specific integer ("the age") is used to determine
 *  which process will be chosen. At first, the age of every waiting process is increased by its priority. After that the oldest
 *  process is chosen. If the oldest process is not distinct, the one with the highest priority is chosen. If this is not distinct
 *  as well, the one with the lower ProcessID is chosen. Before actually returning the ProcessID, the age of the process who
 *  is to be returned is reset to its priority.
 *
 *  \param processes An array holding the processes to choose the next process from.
 *  \param current The id of the current process.
 *  \return The next process to be executed, determined based on the inactive-aging strategy.
 */
ProcessID os_Scheduler_InactiveAging(Process const processes[], ProcessID current) {
    // This is a presence task
	uint8_t Max = 0;
	if(SchedulingInfo.Zeitschiebe > 0)
	{
		SchedulingInfo.Zeitschiebe --;
		for(int i = 1; i < 8 ; i++)
		{
			if(current != i){
				SchedulingInfo.age[i] += processes[i].priority;
			}
		}
		return current;
	}
	
	else
	{
		SchedulingInfo.age[current] = processes[current].priority;
		for(int i = 1 ; i < 8 ; i++){
			if(processes[i].state == OS_PS_READY && SchedulingInfo.age[i] > SchedulingInfo.age[Max]){
				Max = i;
			}
			else if(processes[i].state == OS_PS_READY && SchedulingInfo.age[i] == SchedulingInfo.age[Max] && processes[i].priority > processes[Max].priority){
				Max = i;
			}
			else if(processes[i].state == OS_PS_READY && SchedulingInfo.age[i] == SchedulingInfo.age[Max] && processes[i].priority > processes[Max].priority && i < Max){
				Max = i;
			}
		}
		
		return Max;
	}
}

/*!
 *  This function realizes the run-to-completion strategy.
 *  As long as the process that has run before is still ready, it is returned again.
 *  If  it is not ready, the even strategy is used to determine the process to be returned
 *
 *  \param processes An array holding the processes to choose the next process from.
 *  \param current The id of the current process.
 *  \return The next process to be executed, determined based on the run-to-completion strategy.
 */
ProcessID os_Scheduler_RunToCompletion(Process const processes[], ProcessID current) {
    // This is a presence task
	uint8_t counter;
	uint8_t i = 1;
	if(processes[current].state == OS_PS_RUNNING)
	{
		return current;
	}
	else{
		do
		{
			if (processes[i].state == OS_PS_READY)
			{
				return i;
			}
			
			if (i == MAX_NUMBER_OF_PROCESSES-1)
			{
				i = 1;
			}
			else
			{
				i = i+1;
			}
			counter++;
		} while (counter < 6);
	}
	
	return 0;
	
	}
    

