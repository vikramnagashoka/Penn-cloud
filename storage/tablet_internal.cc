#include "tablet_internal.h"

void *snapshotFunc(void *params) {
    pthread_detach(pthread_self());
    assert(config.isPrimary());
    
    while (true) {
        sleep(20);
        vector<Server> &peers = config.myCluster;
        config.myTablet->snapshot();
        for (int i = 0; i < peers.size(); i++) {
            if (peers[i] == config.myServer) continue;
            sendMsg_udp(peers[i], "snap");
        }
        sleep(20);
    }
    return NULL;
}

bool sourceIsMaster(sockaddr_in &source) {
    bool out = (source.sin_port == config.privateMaster.port) &&
                (source.sin_addr.s_addr == config.privateMaster.address);
    return out;
}

// elect <server>
void handleElect(sockaddr_in &src, string command) {
    int pos = command.find(SPACE);
    string serverString = command.substr(pos+1);
    Server newPrimary(serverString);
    config.myPrimary = newPrimary;
    sendReply_udp(src, "+ACK elect");
    if (config.isPrimary()) {
        pthread_t tid;
        pthread_create(&tid, NULL, &snapshotFunc,NULL);
    }
}

// crashed <server>
void handleCrashed(sockaddr_in &src, string command) {
    int pos = command.find(SPACE);
    string serverString = command.substr(pos+1);
    Server crashed(serverString);
    int res = config.removeFromMyCluster(crashed);
    if (res > 0) sendReply_udp(src, "+ACK crashed");
    else sendReply_udp(src, "-NOO crashed did not exist");
}

// restart <server>
void handleRestarted(sockaddr_in &src, string command) {
    int pos = command.find(SPACE);
    string serverString = command.substr(pos+1);
    Server restarted(serverString);
    config.addToMyCluster(restarted);
    sendReply_udp(src, "+ACK restart");
}

void parseMasterCommand(sockaddr_in &src, string command) {
    rtrim(command);
    string sub1 = command.substr(0,10);
    string sub2 = command.substr(0,6);
    string sub3 = command.substr(0,8);
    if (sub2 == "elect ") handleElect(src, command);
    else if (sub3 == "crashed ") handleCrashed(src, command);
    else if (sub1 == "restarted ") handleRestarted(src, command);
}

// receiving UDP messages from master
void *foofunc(void *params) {
    int sockfd = *(int*) params;
    char buffer[100];
    bzero(buffer, sizeof(buffer));
    
    while (true) {
        struct sockaddr_in src;
        socklen_t srcSize = sizeof(src);
        bzero(&src, sizeof(src));
        int rlen = recvfrom(sockfd, buffer, sizeof(buffer)-1, 0,
                            (struct sockaddr*) &src, &srcSize);
        buffer[rlen] = 0;
        string msg(buffer);
        
        if (sourceIsMaster(src)) {
            logVerbose("[U] M: %s", buffer);
            parseMasterCommand(src, msg);
            continue;
        } else {
            if (msg == "snap") {
                logVerbose("[U] P: %s", buffer);
                config.myTablet->snapshot();
            }
        }
    }
}
