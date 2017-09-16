#ifndef __tablet_common_h
#define __tablet_common_h


#define MAXBLOBSIZE 20000000
#define SPACE " "

#include "common.h"
#include "base_structs.h"
#include "tablet.h"
#include "ServerConfig.h"

extern ServerConfig config;
void *backupfunc(void *params);
void sendReply(int connfd, const char* msg); // server sends reply to client
void sendReply_udp(sockaddr_in &src, const char* msg); // send reply to master
void sendMsg_udp(Server &server, const char* msg); // primary send msg to replicas
#endif
