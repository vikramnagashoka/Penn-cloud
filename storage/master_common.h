#ifndef __master_common_h
#define __master_common_h

#include "common.h"
#include "MasterConfig.h"

extern int sockfd1;
extern int sockfd2;
extern int sockfd3;

extern MasterConfig mconfig;

// tcp
inline void sendMsg(int connfd, const char* msg) {
    int len = strlen(msg) + 2;
    char buffer[len];
    strcpy(buffer, msg);
    strcat(buffer, "\r\n");
    int status = write(connfd, buffer, len);
    if (status < 0) logVerbose("[%d] Failed writing: %s", connfd, buffer);
    else logVerbose("[%d] M: %s", connfd, msg);
}

inline void sendMsg_udp(Server &server, const char* msg) {
    struct sockaddr_in addr; bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = server.address;
    addr.sin_port = server.port;
    char buf[strlen(msg)];
    sprintf(buf, "%s", msg);
    int status = sendto(sockfd2, buf, strlen(buf), 0, 
                    (struct sockaddr*) &addr, sizeof(addr));
    if (status < 0) logVerbose("Error sending UDP msg: %s", buf);
}

#endif
