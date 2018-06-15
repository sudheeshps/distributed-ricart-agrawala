#include "ricart-agrawal-algorithm.h"
#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <stdlib.h>
#include <process.h>
#include <Windows.h>

void run_master(int process_count);
void run_peer(int process_count);
int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		printf("Usage Ricart-Agrawala-Algorithm </monitor {process count}> or </peer {process count}>\n");
		return -1;
	}
	// No number validation done for simplicity
	if (0 == strcmp("/monitor", argv[1]))
	{
		//Runs in the monitor terminal mode...displays messages
		run_master(atol(argv[2]));
		return 0;
	}
	//Runs as a peer process. 
	//It multicats requests and listens to multicasts
	run_peer(atol(argv[2]));
	return 0;
}
void run_master(int process_count)
{
	setup(1,process_count);
	printf("Press a key to exit...\n\n");
	printf("---------------------------\n");
	printf("This is a monitor process which listens to multicast messages\n");
	printf("This process is not involved in algorithm execution\n");
	printf("********Debug Logs*********\n");
	printf("---------------------------\n");
	_getch();
	teardown();
}

void run_peer(int process_count)
{
	setup(0,process_count);
	//Starts the algorithm simulation.
	//It randomly sends the mutex requests
	start_algorithm();
	printf("Press a key to exit...\n\n");
	_getch();
	teardown();
}

