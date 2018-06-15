#include "multicast.h"
#include "structs.h"
#include "ricart-agrawal-algorithm.h"
#include <stdbool.h>
#include <stdio.h>
#include <process.h>
#include <windows.h>

#define MULTI_CAST_IP_GROUP "225.0.0.37"
#define PORT 20000

const char* REQUEST_MSG = "REQUEST";
const char* REPLY_MSG = "REPLY";

int g_is_monitorprocess = 0;
int g_socket = 0;
int g_total_process_count = 0;
int g_my_pid = 0;
int g_mytime = 0;
int g_reply_count = 0;
bool g_defer_reply = false;
cs_state g_critical_section_state = NONE;

sockaddress* g_psock_addr = NULL;
HANDLE g_reply_sync_event = NULL;

void run_algorithm_simulator(void* param);

void receiver_thread(void* param);
void on_message_received(message msg);
void on_request_enter_critical_section(message msg);
void on_reply_enter_critical_section(message msg);
bool is_defer_reply(message msg);
void update_clock(message msg);

void enter_critical_section();
bool is_all_postive_replies_received();
void leave_critical_section();

int send_message(command cmd);

const char* get_command_name(command cmd);
void print_message(message msg);

int setup(int monitior_process,int process_count)
{
    g_is_monitorprocess = monitior_process;
    g_total_process_count = process_count;
    if (initialize() < 0)
    {
        return -1;
    }
    if ((g_socket = setup_socket()) < 1)
    {
        return -1;
    }
    g_psock_addr = create_sockaddr(MULTI_CAST_IP_GROUP, PORT);
    g_my_pid = GetCurrentProcessId();
    printf("my process id:%d\n", g_my_pid);
    g_reply_sync_event = CreateEvent(0, FALSE, FALSE, NULL);
    _beginthread(receiver_thread, 0, 0);
    return 0;
}

void teardown()
{
    teardown_socket(g_socket, g_psock_addr);
    uninitialize();
}

void start_algorithm()
{
    _beginthread(run_algorithm_simulator, 0, 0);
}

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

void receiver_thread(void* param)
{
    int listener_socket, addrlen;
    sockaddress addr = { 0 };
    struct ip_mreq mreq = { 0 };
    message msg = { 0 };
    listener_socket = setup_socket();
    if (listener_socket < 0)
    {
        printf("Receiver thread: failed to setup listener socket...exiting listener\n");
        return;
    }
    setup_listener(listener_socket, MULTI_CAST_IP_GROUP, PORT, &addr, &mreq);
    while (1)
    {
        addrlen = sizeof(sockaddress);
        if (recvfrom(listener_socket, (char*)&msg, sizeof(msg), 0,
            (struct sockaddr *) &addr, &addrlen) > 0)
        {
            on_message_received(msg);
        }
    }
    _endthread();
}

void on_message_received(message msg)
{
    if (g_my_pid == msg.pid)
    {
        /*Skip my own message*/
        return;
    }

    //If monitor process, simply displays the messages
    if (g_is_monitorprocess)
    {
        print_message(msg);
        return;
    }
    switch (msg.cmd)
    {
    case REQUEST:
        on_request_enter_critical_section(msg);
        break;
    case REPLY:
        on_reply_enter_critical_section(msg);
        break;
    }
}

void on_request_enter_critical_section(message msg)
{
    printf("Enter critical section message received from pid:%d\n", msg.pid);
    update_clock(msg);
    g_defer_reply = is_defer_reply(msg);
    if (g_defer_reply)
    {
        printf("Defering reply...\n");
        return;
    }
    printf("Sending reply...\n");
    send_message(REPLY);
}

bool is_defer_reply(message msg)
{
    //Already in critical section
    if (g_critical_section_state == HELD) return true;
    //If either NONE or RELEASED. Reply can be send.
    if (g_critical_section_state != WANTED) return false;
    //If my time is causally before incoming time, reply has to be deffered
    if (g_mytime < msg.time) return true;
    //In case of same time, use node number or process identifier to break the tie
    if ((g_mytime == msg.time) && (g_my_pid < msg.pid)) return true;
    return false;
}

//Update my Lamport clock
void update_clock(message msg)
{
    g_mytime = (g_mytime < msg.time) ? msg.time + 1 : g_mytime;
}

void on_reply_enter_critical_section(message msg)
{
    //I am already in requested state
    if (g_critical_section_state == WANTED)
    {
        ++g_reply_count;
    }
    //Whether I got all replies...then signal wait
    if (is_all_postive_replies_received())
    {
        printf("All replies received(%d)...signalling event...\n", g_reply_count);
        SetEvent(g_reply_sync_event);
        g_reply_count = 0;
    }
    //For each message received, update my clock with that in message based on the rule
    update_clock(msg);
}

int wait_for_replies()
{
    const int TIME_OUT = 5 * 1000;
    if (WAIT_TIMEOUT == WaitForSingleObject(g_reply_sync_event, TIME_OUT))
    {
        printf("Oops went in to deadlock...timedout\n");
        return -1;
    }
    return 0;
}

void enter_critical_section()
{
    printf("Entering critical section...\n");
    g_critical_section_state = WANTED;
    
    printf("Raising request for critical section...\n");
    g_mytime += 1;
    send_message(REQUEST);
    
    printf("Waiting for replies...\n");
    if (-1 == wait_for_replies())
    {
        printf("Critical section not granted...\n");
        leave_critical_section();
    }
    
    printf("Critical section granted...setting HELD...sleeping 5 sec\n");
    g_critical_section_state = HELD;
    
    Sleep(5 * 1000);
    leave_critical_section();
}

void leave_critical_section()
{
    printf("Leaving critical section...setting RELEASED...\n");
    g_critical_section_state = RELEASED;
    g_mytime += 1;
    if (g_defer_reply)
    {
        printf("Reply pending...sending reply...\n");
        send_message(REPLY);
    }
    g_reply_count = 0;
    g_defer_reply = false;
}

bool is_all_postive_replies_received()
{
    if (g_reply_count == (g_total_process_count - 1))
    {
        return true;
    }
    return false;
}

int send_message(command cmd)
{
    message msg = { 0 };
    msg.cmd = cmd;
    msg.pid = g_my_pid;
    msg.time = g_mytime;
    return multicast_message(g_socket, g_psock_addr, (char*)&msg, sizeof(msg));
}

void print_message(message msg)
{
    printf("Debug Log: command=%s process-id=%d time-stamp=%d\n", 
           get_command_name(msg.cmd), msg.pid, msg.time);
}

const char* get_command_name(command cmd)
{
    switch (cmd)
    {
    case REQUEST: return REQUEST_MSG;
    case REPLY: return REPLY_MSG;
    }
    return "";
}