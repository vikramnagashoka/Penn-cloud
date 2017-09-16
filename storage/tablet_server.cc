#include "tablet_server.h"

int testing = 0;
int verbose = 0;
// global config for server to keep track of cluster status
ServerConfig config;

void parseServerConfig(const char *filename, int nn, ServerConfig &config);

int main(int argc, char *argv[]) {
    signal(SIGPIPE, SIG_IGN);
    
    // parse command line arguments
    if (argc < 3) {
        fprintf(stderr, "*** Author: Selina Liu (liu15)\n");
        exit(1);
    }
    int c;
    
    // processing optional arguments
    while ((c = getopt(argc, argv, "trv")) != -1) {
        switch (c) {
        case 'r':
            config.restored = 1; break;
        case 'v':
            // turn on debug mode
            verbose = 1; break;
        case 't':
            testing = 1; break;
        default:
            cout << "error" << endl;
        }
    }

    if (argv[optind] == NULL) throwMyError("No file and server index specified");
    if (argv[optind+1] == NULL) throwMyError("No server index specified");

    const char *filename = argv[optind];
    int nn = atoi(argv[optind+1]);
    parseServerConfig(filename, nn, config);
   
    // if restoring from snapshots
    if (config.restored) { config.myTablet->restore(); } 
   
    int port = config.myServer.port;
    long address = config.myServer.address;
    
    // create tcp socket to accept connections from clients and primary
    int status;
    int optval;
    config.tcpfd = socket(PF_INET, SOCK_STREAM, 0);
    if (config.tcpfd < 0) throwSysError("Failed to obtain socket");
    status = setsockopt(config.tcpfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    if (status < 0) logVerbose("Failed setting reusable address");
    struct sockaddr_in servaddr1;
    servaddr1.sin_family = AF_INET;
    servaddr1.sin_addr.s_addr = address;
    servaddr1.sin_port = port;
    status = ::bind(config.tcpfd, (struct sockaddr*) &servaddr1, sizeof(servaddr1));
    if (status < 0) throwSysError("Socket did not bind");
    logVerbose("Bound to TCP socket with address %s", config.myServer.literal.c_str());
    status = listen(config.tcpfd, 10);

    // create udp socket to receive msgs from master and primary
    config.udpfd = socket(PF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in servaddr2;
    servaddr2.sin_family = AF_INET;
    servaddr2.sin_port = port;
    servaddr2.sin_addr.s_addr = address;
    status = ::bind(config.udpfd, (struct sockaddr*) &servaddr2,sizeof(servaddr2));
    if (status < 0) throwSysError("Socket did not bind");
    logVerbose("Bound to UDP socket with address %s", config.myServer.literal.c_str());

    // to listen to udp messages from master and primary
    pthread_t udp_tid;
    pthread_create(&udp_tid, NULL, &foofunc, (void*) &config.udpfd);

    // primary to send "snap" udp msgs to replicas regularly
    if (config.isPrimary()) {
        pthread_t tid;
        pthread_create(&tid, NULL, &snapshotFunc, NULL);
    }
    
    spd::set_level(spd::level::info);

    if (verbose) config.print();

    // main loop to accept new TCP connections
    while (true) {
        struct sockaddr_in clientaddr;
        socklen_t addrlen = sizeof(clientaddr);
        int connfd = accept(config.tcpfd, (struct sockaddr*) &clientaddr, &addrlen);
        if (connfd < 0) logVerbose("Failed to accept TCP connection");

        pthread_t tid;
        pthread_create(&tid, NULL, &connectionFunc, (void*) &connfd);
    }
    close(config.tcpfd);
    close(config.udpfd);
    return 0;
}


// nn is the last command-line argument
void parseServerConfig(const char *filename, int nn, ServerConfig &config) {
    // open file
    ifstream ifs(filename);
    Json::Reader reader;
    Json::Value root;
    reader.parse(ifs, root);
  
    // master
    string masString = root["master"].asString();
    int pos = masString.find(",");
    string pub = masString.substr(0,pos);
    string pri = masString.substr(pos+1);
    config.publicMaster = Server(pub);
    config.privateMaster = Server(pri);

    // all server nodes
    Json::Value serverArr = root["servers"];
    for (int i = 0; i < serverArr.size(); i++) {
        string raw = serverArr[i].asString();
        Server currServer(raw);
        config.servers.push_back(currServer);
    }

    Json::Value clusterObj = root["clusters"];
    Json::ValueIterator it;

    for (it = clusterObj.begin(); it != clusterObj.end(); it++) {
        string rangeString = it.key().asString();
        Range range(rangeString);
        config.clusters.insert(make_pair(range, vector<Server>()));
        vector<Server> &currCluster = config.clusters[range];
        Json::Value list = *(it);
        for (int i = 0; i < list.size(); i++) {
            int curr = list[i].asInt();
            currCluster.push_back(config.servers[curr]);
            if (curr == nn) config.myRange = range;
        }
    }
    // making new table
    config.nn = nn;
    config.myServer = config.servers[nn];
    config.myCluster = config.clusters[config.myRange];
    config.myPrimary = config.myCluster[0]; //todo: change
    config.myTablet = new Tablet(config.myRange, config.myServer);
    ifs.close();
}
