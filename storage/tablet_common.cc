#include "tablet_common.h"


// UTILITY FUNCTIONS
void sendReply(int connfd, const char* msg) {
    int len = strlen(msg) + 2;
    char buffer[len];
    strcpy(buffer, msg);
    strcat(buffer, "\r\n");
    int status = write(connfd, buffer, len);
    if (status < 0) logVerbose("[%d] S%d failed sending: %s", connfd, config.nn, msg);
    else logVerbose("[%d] S%d: %s", connfd, config.nn, msg);
}

// send UDP reply to master
void sendReply_udp(sockaddr_in &src, const char* msg) {
    int status = sendto(config.udpfd, msg, strlen(msg), 0,
                    (struct sockaddr*) &src, sizeof(src));
    if (status < 0) logVerbose("[U] S%d failed sending: %s", config.nn, msg);
    else logVerbose("[U] S%d: %s", config.nn, msg);
}

// primary send to replicas
void sendMsg_udp(Server &server, const char* msg) {
    struct sockaddr_in addr; bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = server.address;
    addr.sin_port = server.port;
    int status = sendto(config.udpfd, msg, strlen(msg), 0, 
                    (struct sockaddr*) &addr, sizeof(addr));
    if (status < 0) logVerbose("Error sending UDP msg: %s", msg);
}
