#include "tablet_thread.h"

void *connectionFunc(void *params);
void parseFirstCommand(int connfd, string command);
void masterThreadFunc(int connfd);
void clientThreadFunc(int connfd);
void parseClientCommand(string command, ClientSession &ses);

void masterThreadFunc(int connfd) {
    sendReply(connfd, "+OK master");
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
            // logVerbose("[%d] M: %s", connfd, command.c_str());
            if (command == "ping") sendReply(connfd, "pong");
            bzero(cbuffer, sizeof(cbuffer));
            strcpy(cbuffer, found+2);
        } else {
            strncat(cbuffer, rbuffer, strlen(rbuffer));
        }
        bzero(rbuffer, sizeof(rbuffer));
    }
}


void parseFirstCommand(int connfd, string command) {
    rtrim(command);
    if (command == "yo") clientThreadFunc(connfd);
    else if (command == "master") masterThreadFunc(connfd);
    else {
        sendReply(connfd, "-ERR invalid first command; server quitting");
        close(connfd);
        int *retval;
        pthread_exit(retval);
        return;
    }
}

// first thread function to start
void *connectionFunc(void *params) {
    logVerbose("in connectionFunc");
    int connfd = *((int*) params);
    pthread_detach(pthread_self());

    // read command
    char rbuffer[100];
    char cbuffer[100];
    bzero(rbuffer, sizeof(rbuffer)); bzero(cbuffer, sizeof(cbuffer));
    string command;

    // main loop to parse commands
    while (read(connfd, rbuffer, sizeof(rbuffer)) > 0) {
        logVerbose("rbuffer: %s", rbuffer);
        char *found = strstr(rbuffer, CRLF);
        if (found) {
            int len = found - rbuffer;
            strncat(cbuffer, rbuffer, len);
            command = string(cbuffer);
            logVerbose("[%d] FIRST: %s", connfd, command.c_str());
            break;
        } else {
            strncat(cbuffer, rbuffer, strlen(rbuffer));
        }
        bzero(rbuffer, sizeof(rbuffer));
    }

    parseFirstCommand(connfd, command);
    return NULL;
}


void clientThreadFunc(int connfd) {
    ClientSession *session = new ClientSession(connfd, pthread_self());
    ClientSession &ses = *session;
   
    sendReply(connfd, "+OK hi");

    // read command from client
    char rbuffer[100];
    char cbuffer[100];
    bzero(rbuffer, sizeof(rbuffer)); bzero(cbuffer, sizeof(cbuffer));
    string command;

    // main loop to parse commands
    while (read(connfd, rbuffer, sizeof(rbuffer)) > 0) {
        logVerbose("rbuffer: %s", rbuffer);
        char *found = strstr(rbuffer, CRLF);
        if (found) {
            int len = found - rbuffer;
            strncat(cbuffer, rbuffer, len);
            command = string(cbuffer);
            logVerbose("[%d] C: %s", connfd, command.c_str());
            parseClientCommand(command, ses);
            bzero(cbuffer, sizeof(cbuffer));
            strcpy(cbuffer, found+2);
        } else {
            strncat(cbuffer, rbuffer, strlen(rbuffer));
        }
        bzero(rbuffer, sizeof(rbuffer));
    }
}

void parseClientCommand(string command, ClientSession &ses) {
    int connfd = ses.getConnfd();
    rtrim(command);
    string sub1 = command.substr(0,4);
    string sub2 = command.substr(0,5);
    string sub3 = command.substr(0,7);

    // parsing command
    if (command == "") { return;
    } else if (command == "noop") { sendReply(connfd, "+OK you are good");
    } else if (command == "yo") { sendReply(connfd, "+OK");
    } else if (command == "kval") { ses.handleKval();
    } else if (command == "quit") { ses.handleQuit();
    } else if (command == "rset") { ses.handleRset();
    } else if (sub1 == "get ") { ses.handleGet(command);
    } else if (sub1 == "put ") { ses.handlePut(command);
    } else if (sub1 == "del ") { ses.handleDel(command);
    } else if (sub2 == "cput ") { ses.handleCput(command);
    } else if (sub3 == "oldval ") { ses.handleOldVal(command);
    } else if (sub3 == "newval ") { ses.handleNewVal(command);
    } else if (sub2 == "forw ") { ses.handleForward(command);
    } else if (sub2 == "writ ") { ses.handleWrite(command);
    } else if (sub2 == "eras ") { ses.handleErase(command);
    } else if (command == "get" || command == "put" || 
                command == "del" || command == "cput"|| 
                command == "val") {
        sendReply(connfd, "Argument missing");
    } else sendReply(connfd, "-ERR Invalid command"); 
    return;
}
