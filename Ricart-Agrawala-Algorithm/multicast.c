#include <ws2tcpip.h>
#include <winsock.h>
#include "multicast.h"
//Intializes the winsock library.
//This is required, otherwise socket APIs will fail
int initialize()
{
    WORD wVersionRequested;
    WSADATA wsaData = { 0 };

    /* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
    wVersionRequested = MAKEWORD(2, 2);

    if (0 != WSAStartup(wVersionRequested, &wsaData))
    {
        return -1;
    }
    return 0;
}

//Uninitializes the socket library
void uninitialize()
{
    WSACleanup();
}

int setup_socket()
{
    int fd = 0;

    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        return -1;
    }

    return fd;
}

sockaddress* create_sockaddr(char* ip_group, int portnum)
{
    sockaddress* paddr = malloc(sizeof(sockaddress));
    /* set up destination address */
    memset(paddr, 0, sizeof(struct sockaddr_in));
    paddr->sin_family = AF_INET;
    paddr->sin_addr.s_addr = inet_addr(ip_group);
    paddr->sin_port = htons(portnum);
    return paddr;
}

void teardown_socket(int fd, sockaddress* paddr)
{
    free(paddr);
    closesocket(fd);
}

int multicast_message(int fd, sockaddress* psockaddr, char* buffer, int bufferlen)
{
    if (sendto(fd, buffer, bufferlen, 0, (struct sockaddr *)psockaddr,
        sizeof(sockaddress)) < 0)
    {
        return -1;
    }
    return 0;
}

//Listens to the multicast requests
int setup_listener(int listener_socket, char* ip_group, int portnum,
                   sockaddress* paddr, struct ip_mreq* pmreq)
{
    u_int yes = 1;

    if (setsockopt(listener_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&yes, sizeof(yes)) < 0)
    {
        return -1;
    }
    memset(paddr, 0, sizeof(struct sockaddr));
    paddr->sin_family = AF_INET;
    paddr->sin_addr.s_addr = htonl(INADDR_ANY);
    paddr->sin_port = htons(portnum);

    /* bind to receive address */
    if (bind(listener_socket, (struct sockaddr *) paddr, sizeof(sockaddress)) < 0)
    {
        return -1;
    }

    /* use setsockopt() to request that the kernel join a multicast group */
    pmreq->imr_multiaddr.s_addr = inet_addr(ip_group);
    pmreq->imr_interface.s_addr = htonl(INADDR_ANY);
    if (setsockopt(listener_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)pmreq, sizeof(struct ip_mreq)) < 0)
    {
        return -1;
    }
    return 0;
}