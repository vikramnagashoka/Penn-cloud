#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <string.h>
#include <algorithm>
#include <arpa/inet.h>

#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include "master_thread.cc"
#include "master_admin.h"

int sockfd1;
int sockfd2;
int sockfd3;
MasterConfig mconfig;

int verbose = 0;

void parseMasterConfig(const char *filename, MasterConfig &config);
void handleMessage(sockaddr_in &src, string msg);
void handleQuery(sockaddr_in &src, string msg);
void assignPrimaries();

int main(int argc, char *argv[]) {
    signal(SIGPIPE, SIG_IGN);
    int c;
    // processing optional arguments
    while ((c = getopt(argc, argv, "v")) != -1) {
        switch (c) {
        case 'v':
            verbose = 1; // turn on debug mode 
            break;
        default:
            throwMyError("illegal arguments");
        }
    }

    if (argv[optind] == NULL) throwMyError("No config file specified");
    const char *filename = argv[optind];

    // experiment
    parseMasterConfig(filename, mconfig);
    mconfig.print();

    // start udp connection
    int status;
    sockfd1 = socket(PF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = mconfig.publicMaster.port;
    servaddr.sin_addr.s_addr = mconfig.publicMaster.address;
    status = ::bind(sockfd1, (struct sockaddr*) &servaddr, sizeof(servaddr));
    if (status < 0) throwSysError("did not bind");
    logVerbose("Bound to UDP socket (fd = %d) with address %s", sockfd1, 
            mconfig.publicMaster.literal.c_str());

    sockfd2 = socket(PF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in servaddr2;
    servaddr2.sin_family = AF_INET;
    servaddr2.sin_port = mconfig.privateMaster.port;
    servaddr2.sin_addr.s_addr = mconfig.privateMaster.address;
    status = ::bind(sockfd2, (struct sockaddr*) &servaddr2, sizeof(servaddr2));
    if (status < 0) throwSysError("did not bind");
    logVerbose("Bound to UDP socket (fd = %d) with address %s", sockfd2, 
            mconfig.privateMaster.literal.c_str());
    
    // for each server in mconfig, set up TCP thread for heart beat 
    for (int i = 0; i < mconfig.servers.size(); i++) {
        pthread_t tid;
        ServerInfo *info = &mconfig.serversInfo[i];
        pthread_create(&tid, NULL, &serverFunc, (void*)info);
    }
    // set tcp listening port
    sockfd3 = socket(PF_INET, SOCK_STREAM, 0);
    if (sockfd3 < 0) throwSysError("Failed to obtain socket");
    struct sockaddr_in servaddr3;
    servaddr3.sin_family = AF_INET;
    servaddr3.sin_port = mconfig.publicMaster.port;
    servaddr3.sin_addr.s_addr = mconfig.publicMaster.address;
    status = ::bind(sockfd3, (struct sockaddr*) &servaddr3, sizeof(servaddr3));
    if (status < 0) throwSysError("Socket did not bind");
    logVerbose("Bound to TCP socket (fd = %d) with address %s", sockfd3, 
            mconfig.publicMaster.literal.c_str());
    status = listen(sockfd3, 10);
    if (status < 0) throwSysError("Socket failed to listen");
    pthread_t tid;
    pthread_create(&tid, NULL, *acceptFunc, (void*)&status);

    sleep(3);
    mconfig.print();

    char buffer[100];
    bzero(buffer, sizeof(buffer));
    
    // Main receiving loop 
    while (true) {
        struct sockaddr_in src;
        socklen_t srcSize = sizeof(src);
        bzero(&src, sizeof(src));

        int rlen = recvfrom(sockfd1, buffer, sizeof(buffer)-1, 0,
                            (struct sockaddr*) &src, &srcSize);
        buffer[rlen] = 0;
        string msg(buffer);
        handleMessage(src, msg);
        bzero(buffer, sizeof(buffer));
    }
}

void handleMessage(sockaddr_in &src, string msg) {
    rtrim(msg);
    logVerbose("Received msg: %s", msg.c_str());
    if (msg.substr(0,6) == "query ") handleQuery(src, msg);
}


void handleQuery(sockaddr_in &src, string msg) {
    int pos = msg.find(" ");
    Server server;
    Key key(msg.substr(pos+1));
    map<Range,vector<ServerInfo*>>::iterator it;
    map<Range,vector<ServerInfo*>> &clus = mconfig.clusters;
    for (it = clus.begin(); it != clus.end(); it++) {
        if (it->first.inRange(key)) {
            int ind = rand() % it->second.size();
            server = it->second[ind]->server;
            break;
        }
    }
    const char* output = server.literal.c_str();
    int status = sendto(sockfd1, output, strlen(output), 0,
                    (struct sockaddr*) &src, sizeof(src));
    if (status < 0) cout << "ERROR" << endl;
}

void parseMasterConfig(const char *filename, MasterConfig &config) {
    ifstream ifs(filename);
    Json::Reader reader;
    Json::Value root;
    reader.parse(ifs, root);
 
    config.filename = string(filename);
    string masString = root["master"].asString();
    int pos = masString.find(",");
    string pub = masString.substr(0,pos);
    string pri = masString.substr(pos+1);
    config.publicMaster = Server(pub);
    config.privateMaster = Server(pri);
    
    Json::Value serverArr = root["servers"];
    for (int i = 0; i < serverArr.size(); i++) {
        string raw = serverArr[i].asString();
        Server currServer(raw);
        config.servers.push_back(currServer);
        ServerInfo si(i, currServer);
        config.serversInfo.push_back(si);
    }

    Json::Value clusterObj = root["clusters"];
    Json::ValueIterator it;

    for (it = clusterObj.begin(); it != clusterObj.end(); it++) {
        string rangeString = it.key().asString();
        Range range(rangeString);
        config.clusters.insert(make_pair(range, vector<ServerInfo*>()));
        vector<ServerInfo*> &currCluster = config.clusters[range];
        Json::Value list = *(it);
        for (int i = 0; i < list.size(); i++) {
            int curr = list[i].asInt();
            if (i == 0) config.serversInfo[curr].isPrimary = true;
            config.serversInfo[curr].range = range;
            currCluster.push_back(&config.serversInfo[curr]);
        }
    }
    ifs.close();
}

