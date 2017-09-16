#include "MasterConfig.h"

void MasterConfig::print() {
    cout << " ----- MASTER CONFIG ----- " << endl;
    cout << "PUBLIC MASTER: " << publicMaster << endl;
    cout << "PRIVATE MASTER: " << privateMaster << endl;
    cout << "SERVERS INFO: " << endl;
    for (int i = 0; i < serversInfo.size(); i++) {
        ServerInfo &si = serversInfo[i];
        cout << si;
    }
}

Server MasterConfig::getPrimaryForRange(Range &range) {
    vector<ServerInfo*> &peers = clusters[range];
    for (int i = 0; i < peers.size(); i++) {
        ServerInfo *peer = peers[i];
        if (peer->isPrimary) {
            return peer->server;
        }
    }
    return Server();
}

MasterConfig MasterConfig::fetchFromMaster(string literal) {
    Server m(literal);
    struct sockaddr_in paddr;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) logVerbose("Failed obtaining socket for outbound TCP connection");
    paddr.sin_family = AF_INET;
    paddr.sin_addr.s_addr = m.address;
    paddr.sin_port = m.port;
    int status = connect(sockfd, (struct sockaddr*) &paddr, sizeof(paddr));
    if (status < 0) logVerbose("Tablet cannot connect to backup");
    status = write(sockfd, "admin\r\n", 7);
    if (status < 0) logVerbose("Failed writing");
    char rbuffer[10];
    stringstream ss;
    while (read(sockfd, rbuffer, sizeof(rbuffer)) > 0) {
        char *found = strstr(rbuffer, "<EOB>");
        if (found) {
            *found = 0;
            ss << rbuffer;
            break;
        } else { 
            ss << rbuffer; 
        }
        bzero(rbuffer, sizeof(rbuffer));
    }
    MasterConfig output;
    boost::archive::text_iarchive ia(ss);
    ia >> output;
    output.print();
    status = write(sockfd, "quit\r\n", 6);
    if (status < 0) logVerbose("Failed writing");
    close(sockfd);
    return output;
}


