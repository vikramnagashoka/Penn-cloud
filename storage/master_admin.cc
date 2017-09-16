#include "master_admin.h"

// for handling commands for admin console

void handleAdmin(int connfd, string command);
void handleQuit(int connfd);
void handleKeyVal(int connfd, string command);

void parseCommand(int connfd, string commad);
void *adminFunc(void *params);

void *acceptFunc(void *params) {
    while (true) {
        struct sockaddr_in caddr;
        socklen_t addrlen = sizeof(caddr);
        int connfd = accept(sockfd3, (struct sockaddr*) &caddr, &addrlen);
        if (connfd < 0) logVerbose("Failed to accept TCP connection");
        pthread_t tid;
        pthread_create(&tid, NULL, &adminFunc, (void*) &connfd);
    }
    return NULL;
}

void *adminFunc(void *params) {
    int connfd = *((int*) params);
    pthread_detach(pthread_self()); 
    
    // read command
    char rbuffer[100];
    char cbuffer[100];
    bzero(rbuffer, sizeof(rbuffer)); bzero(cbuffer, sizeof(cbuffer));
    string command;

    // main loop to parse commands
    while (read(connfd, rbuffer, sizeof(rbuffer)) > 0) {
        char *found = strstr(rbuffer, CRLF);
        if (found) {
            int len = found - rbuffer;
            strncat(cbuffer, rbuffer, len);
            command = string(cbuffer);
            logVerbose("[%d] FE: %s", connfd, command.c_str());
            parseCommand(connfd, command);
            bzero(cbuffer, sizeof(cbuffer));
            strcpy(cbuffer, found+2);
        } else {
            strncat(cbuffer, rbuffer, strlen(rbuffer));
        }
        bzero(rbuffer, sizeof(rbuffer));
    }
    return NULL;
}

void parseCommand(int connfd, string command) {
    rtrim(command);
    if (command == "admin") handleAdmin(connfd, command);
    if (command == "quit") handleQuit(connfd);
}

void handleAdmin(int connfd, string command) {
    ostringstream archiveStream;
    boost::archive::text_oarchive archive(archiveStream);
    archive << mconfig; 
    string archiveString = archiveStream.str() + "<EOB>";
    int size = archiveString.size();
    const unsigned char *data = reinterpret_cast<const unsigned char*>(archiveString.c_str());
    cout << data << endl;
    int numBytes = write(connfd, data, size);
    if (numBytes < 0) logVerbose("Failed writing config data");
    else if (numBytes == size) logVerbose("Written config to frontend");
    else logVerbose("Number of bytes written different");
}

void handleQuit(int connfd) {
    sendMsg(connfd, "+OK Server is quitting");
    close(connfd);
    int *retval;
    pthread_exit(retval);
    return;
}
