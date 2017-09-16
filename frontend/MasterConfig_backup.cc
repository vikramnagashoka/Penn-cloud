#include "MasterConfig.h"

void MasterConfig::print() {
    cout << "SERVERS INFO: " << endl;
    for (int i = 0; i < serversInfo.size(); i++) {
        ServerInfo &si = serversInfo[i];
        cout << si;
    }
}

MasterConfig MasterConfig::fetchFromMaster(string literal) {
    Server m(literal);
    int verbose = 1;
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
  
    unsigned char rbuffer[256];
    string ass;
    int pos;
    while (read(sockfd, rbuffer, sizeof(rbuffer)) > 0) {
        ass.append(reinterpret_cast<const char*>(rbuffer));
        pos = ass.find("<EOB>");
        if (pos != string::npos) break;
    }
    ass = ass.substr(0,pos);
    stringstream ss;
    ss.str(ass);
    cout << ss.str() << endl;
    MasterConfig output;
    boost::archive::text_iarchive ia(ss);
    ia >> output;
    output.print();
    status = write(sockfd, "quit\r\n", 6);
    if (status < 0) logVerbose("Failed writing");
    close(sockfd);
    return output;
}
