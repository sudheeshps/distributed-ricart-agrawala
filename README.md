*This is an attempt to simulate the working of Ricart-Agrawala-Algorithm for mutual exclusion in distributed applications*

THIS IS A WINDOWS BASED SAMPLE AND TESTED ONLY IN WINDOWS (my laptop is Win10, but it does not matter, it can run in any Windows version)

* About the socket APIs used and mode of communication:
  * It uses POSIX compatible socket APIs.
  * Uses WSAStartup() for socket library intialization which is a Win32 API and is mandatory for using sockets in Windows
  * Uses WSACleanup() for socket library uninitialization
  * Uses Windows kernel event to wait for replies (CreateEvent,WaitForSingleObject and SetEvent)
  * Uses Sleep() to simulate delay
  * It uses sockets to communicate and multicasts to send requests/replies

* How to run algorithm sample:
  * Run "run-algorithm.bat" to see the algorithm in action
  * It starts 4 processes in separate consoles
  * One process is a monitor process which listens to the requests and replies.It acts a debug console to see what messages are exchanged between processes
  * The other 3 are peer processes which runs the algorithm
  * Each instance runs its own child thread and main thread waits on a key hit to quit

* How to run an instance of the sample application:
  * Ricart-Agrawala-Algorithm /monitor 3
  * Ricart-Agrawala-Algorithm /peer 3
  * The 1st argument is /monitor makes process to run as debug console/monitor
  * The 1st argument is /peer makes a peer process which broadcasts messages for requests and replies
  * The 2nd argument is to specify what are the total number of peer processes in the system (example: here 3 means 3 peer processes)

* About how the algorithm usage is simulated:
  * The sample program randomly raises the request for critical sections
  * When critical section is granted it sleeps 5 seconds to simulate processing delay and randomness
  * See the following piece of code which randomizes the request for entering to critical section
```
void run_algorithm_simulator(void* param)
{
    int randno;
    srand(GetTickCount());
    while (1)
    {
        randno = rand() * 121;
        if ((randno % 2) == 0)
        {
            enter_critical_section();
        }
        Sleep(10 * 1000);
    }
}
```
* About the source code structure:
  * Ricart-Agrawala-Algorithm.sln - This is Visual Studio 2013 solution
  * main.c - This is the entry point file which has main()
  * multicast.h - This file contains declarations for multicast APIs
  * multicast.c - This file contains implementation for multicast APIs
  * ricart-agrawal-algorithm.h - This file contains the function declarations for setting up the algorithm
  * ricart-agrawal-algorithm.c - This file contains the implementation for the algorithm
