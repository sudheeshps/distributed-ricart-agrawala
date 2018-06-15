#pragma once
#ifndef __MULTICAST_H__
#define __MULTICAST_H__

typedef struct sockaddr_in sockaddress;

int initialize();
void uninitialize();
int setup_socket();
sockaddress* create_sockaddr(char* ip_group, int portnum);
void teardown_socket(int fd,sockaddress* paddr);
int multicast_message(int fd, sockaddress* psockaddr, char* buffer, int bufferlen);
int setup_listener(int listener_socket, char* ip_group, int portnum, sockaddress* paddr, struct ip_mreq* pmreq);
#endif//__MULTICAST_H__