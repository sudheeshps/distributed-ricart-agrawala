#pragma once
#ifndef __STRUCTS_H__
#define __STRUCTS_H__
#include <time.h>
typedef enum _command
{
	REQUEST = 0,
	REPLY = 1	
}command;

/*broadcast message content*/
typedef struct _message
{
	command cmd;
	int total_process_count;
	int pid; /* process identifier*/
	int time; /* lamport timestamp*/
}message;

/* critical section states */
typedef enum _cs_state
{
	NONE = 0,
	WANTED = 1,
	HELD = 2,
	RELEASED = 3
}cs_state;
#endif
