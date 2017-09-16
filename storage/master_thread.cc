#include "master_common.h"

void handleServerCrash(ServerInfo &crashed);
int restartCrashedServer(ServerInfo &crashed);
void notifyServerRestart(ServerInfo &restarted);

// thread function to ping all servers in the cluster
void *serverFunc(void *params) {
    pthread_detach(pthread_self());
    ServerInfo *p = (ServerInfo*) params;
    ServerInfo &info = *p;
    Server &server = info.server;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) logVerbose("Failed obtaining socket for TCP connection to tablet server");
    // update config 
    info.sockfd = sockfd;

    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = server.address;
    saddr.sin_port = server.port;

    int status = connect(sockfd, (struct sockaddr*) &saddr, sizeof(saddr));
    if (status < 0) logVerbose("Cannot connect to tablet server");

    sendMsg(sockfd, "master");
    expectToRead(sockfd, "+OK master", 3);
    int res; 
    while (true) {
        status = write(sockfd, "ping\r\n", 6);
        if (status < 0) logVerbose("[%d] ERR could not ping data", sockfd);
        res = expectToRead(sockfd, "pong", 5);
        if (res == -1) {
            logVerbose("[%d] read different data", sockfd);
        } 
        else if (res == 0) {
            logVerbose("[%d] lost server", sockfd);
            break;
        } 
        sleep(10);
    }
    close(sockfd);
    handleServerCrash(info); // inform other servers and elect new primary
    sleep(28);
    restartCrashedServer(info);
    notifyServerRestart(info);
    return NULL;
}

int restartCrashedServer(ServerInfo &crashed) {
    pid_t pid;
    int res, count, err;

    //const char *macpath = "./tabletmac";
    const char *vmpath = "./tabletvm";
    string ind = to_string(crashed.index); 
    char *cmd[6];
    //cmd[0] = "tabletmac"; // tabletvm
    cmd[0] = "tabletvm"; // tabletvm
    cmd[1] = "-r";
    cmd[2] = "-v";
    cmd[3] = (char*) mconfig.filename.c_str();
    cmd[4] = (char*) ind.c_str();
    cmd[5] = (char*) 0;

    int pipefds[2];
    if (pipe(pipefds)) {
        perror("pipe");
        return EX_OSERR;
    }
    if (fcntl(pipefds[1], F_SETFD, fcntl(pipefds[1], F_GETFD) | FD_CLOEXEC)) {
        perror("fcntl");
        return EX_OSERR;
    }
    switch ((pid = fork())) {
        case -1:
            perror("fork failed");
            break;
        case 0: // in child
            close(pipefds[0]);
            // res = execv(macpath, cmd);
            res = execv(vmpath, cmd);
            write(pipefds[1], &errno, sizeof(int));
            logVerbose("child: execv failed");
            _exit(0);
            break;
        default:
            close(pipefds[1]);
            while ((count = read(pipefds[0], &err, sizeof(errno))) == -1) {
                if (errno != EAGAIN && errno != EINTR) break;
            }
            if (count) {
                logVerbose("my child's execvp: %s", strerror(err));
                return -1;
            }
            close(pipefds[0]);
            logVerbose("sucessfully restarted process");
            break;
    }
    // child successfully created
    sleep(15);
    crashed.alive = true;
    pthread_t tid;
    pthread_create(&tid, NULL, &serverFunc, (void*)&crashed);
    return 1;
}

// notify peers that server has been restarted
void notifyServerRestart(ServerInfo &restarted) {
    Range &range = restarted.range;
    vector<ServerInfo*> &peers = mconfig.clusters[range];
     
    string restartMsg = "restarted " + restarted.server.literal;
    for (int i = 0; i < peers.size(); i++) {
        ServerInfo *peer = peers[i];
        if (peer == &restarted) {
            string prim = mconfig.getPrimaryForRange(range).literal;
            string electMsg = "elect " + prim;
            sendMsg_udp(peer->server, electMsg.c_str());
            continue;
        }
        if (peer->isHealthy()) {
            sendMsg_udp(peer->server, restartMsg.c_str());
        }
    }
}

// send crash (and elect primary) messages to nodes in same range 
void handleServerCrash(ServerInfo &crashed) {
    crashed.alive = false;
    crashed.sockfd = -1;
    Range &range = crashed.range;
    vector<ServerInfo*> &peers = mconfig.clusters[range];
    stringstream ess;
    stringstream css;
    Server newPrimary; 
    bool primaryDied = crashed.isPrimary;
    if (primaryDied) {
        crashed.isPrimary = false;
        for (int i = 0; i < peers.size(); i++) {
            ServerInfo *peer = peers[i];
            if (peer->isHealthyBackup()) {
                newPrimary = peer->server;
                peer->isPrimary = true;
                break;
            }
        }
        ess << "elect " << newPrimary; 
    } 
    css << "crashed " << crashed.server;

    string electMsg = ess.str();
    string crashedMsg = css.str();

    for (int i = 0; i < peers.size(); i++) {
        ServerInfo *peer = peers[i];
        if (peer->isHealthy()) {
            sendMsg_udp(peer->server, crashedMsg.c_str());
            if (primaryDied) {
                sendMsg_udp(peer->server, electMsg.c_str());
            }
        }
    }
}

