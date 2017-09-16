#include "ClientSession.h"

ClientSession::ClientSession(int fd, pthread_t threadId): connfd(fd), tid(threadId) {
    status = 1;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    // create a unique hash based on client session
    size_t seed;
    boost::hash_combine(seed, (int)tv.tv_sec);
    boost::hash_combine(seed, (long)tv.tv_usec);
    boost::hash_combine(seed, tid);
    boost::hash_combine(seed, connfd);
    db.hashSeed = seed;
}

void ClientSession::handleKval() {
    string out = tab->getKeys() + "<EOB>";
    sendReply(connfd, out.c_str());
}

// format: get sel,mel
void ClientSession::handleGet(string command) {
    if (status != 1) {
        sendReply(connfd, "-ERR wrong sequence of command");
        return;
    }
    int pos1 = command.find(SPACE);
    int pos2 = command.find(SPACE, pos1+1);
    string keyString = command.substr(pos1+1);
    Key key(keyString);
    if (db.populateKey(connfd, key) < 0) return;
    if (tab->exists(db.currKey)) {
        Blob b = tab->get(db.currKey);
        b.push_back('<');
        b.push_back('E');
        b.push_back('O');
        b.push_back('B');
        b.push_back('>');
        int status = write(connfd, &b[0], b.size()); 
        if (status < 0) logVerbose("Failed writing blob back to client");
    }
    else sendReply(connfd, "-ERR key does not exist");
}

// format: put sel,mel 5 2
void ClientSession::handlePut(string command) {
    if (status != 1) {
        sendReply(connfd, "-ERR wrong sequence of command");
        return;
    }
    int pos1 = command.find(SPACE);
    int pos2 = command.find(SPACE, pos1+1);
    int pos3 = command.find(SPACE, pos2+1);
    string keyString = command.substr(pos1+1, pos2-pos1-1);
    Key key(keyString);
    int numBytes = stoi(command.substr(pos2+1,pos3-pos2-1));
    int counterId = stoi(command.substr(pos3+1));
    if (db.populateAll(connfd, key, numBytes) < 0) return;
    db.setInvocationId(counterId);
    Blob blob = db.getBlob();
    stringstream oss;
    stringstream ess;
    oss << "+OK PUT " << db.currKey << " successful";
    ess << "-ERR PUT " << db.currKey << " failed";
    string okReply = oss.str();
    string errReply = ess.str();

    if (config.isPrimary()) {
        logVerbose("This primary is notifying other replicas");
        tab->put(db.getHash(), db.currKey, blob);
        tab->print();
        int result = notifyReplicas(true);
        if (result > 0) {
            sendReply(connfd, okReply.c_str());
            db.reset();
            logVerbose("[%d] Reset data buffer", connfd);
        } else {
            sendReply(connfd, "-ERR put incomplete"); 
        }
    } 
    else {
        logVerbose("This replica is forwarding to primary");
        ForwardContext *context = new ForwardContext(true, &db);
        if (forwardFunc(context) > 0) { sendReply(connfd, okReply.c_str()); } 
        else { sendReply(connfd, errReply.c_str()); }
    }
}

// format: del sel,mel <id>
void ClientSession::handleDel(string command) {
    if (status != 1) {
        sendReply(connfd, "-ERR wrong sequence of command");
        return;
    }
    int pos1 = command.find(SPACE);
    int pos2 = command.find(SPACE, pos1+1);
    string keyString = command.substr(pos1+1, pos2-pos1-1);
    Key key(keyString);
    int counterId = stoi(command.substr(pos2+1));

    if (db.populateKey(connfd, key) < 0) return;
    db.setInvocationId(counterId);

    if (!tab->exists(db.currKey)) {
        sendReply(connfd, "-ERR no value to delete");
        return;
    }
    stringstream ss;
    stringstream ess;
    ss << "+OK DEL " << db.currKey << " successful";
    ess << "+ERR DEL " << db.currKey << " failed";
    string okReply = ss.str();
    string errReply = ess.str();
    int res; 
    if (config.isPrimary()) {
        tab->erase(db.getHash(), db.currKey);
        res = notifyReplicas(false);
    } else {
        ForwardContext *context = new ForwardContext(false, &db);
        res = forwardFunc(context);
    }
    if (res > 0) { sendReply(connfd, okReply.c_str()); } 
    else { sendReply(connfd, errReply.c_str()); }
    db.reset();
    logVerbose("[%d] Reset data buffer", connfd);
}

void ClientSession::handleQuit() {
    sendReply(connfd, "+OK Server is quitting");
    close(connfd);
    int *retval;
    pthread_exit(retval);
    delete this;
    return;
}

void ClientSession::handleRset() {
    status = 1;
    db.reset();
    logVerbose("[%d] Reset session", connfd);
    sendReply(connfd, "+OK Session is reset");
}

// cput sel,mel 5
void ClientSession::handleCput(string command) {
    if (status != 1) {
        sendReply(connfd, "-ERR wrong sequence of command");
        return;
    }
    int pos1 = command.find(SPACE);
    int pos2 = command.find(SPACE, pos1+1);
    string keyString = command.substr(pos1+1, pos2-pos1-1);
    Key key(keyString);
    int counterId = stoi(command.substr(pos2+1));
    
    if (db.populateKey(connfd, key) < 0) return;
    db.setInvocationId(counterId);
    if (tab->exists(db.currKey)) { 
        status = 2; 
        sendReply(connfd, "+OK old value then new value");
    } 
    else {
        sendReply(connfd, "-ERR old value does not exist, CPUT aborted");
        status = 1;
    }
}

// oldval 10
void ClientSession::handleOldVal(string command) {
    if (status != 2) {
        sendReply(connfd, "-ERR wrong sequence of command");
        return;
    }
    
    int pos1 = command.find(SPACE);
    int numBytes = stoi(command.substr(pos1+1));

    db.populateBuffer(connfd, numBytes);
    Blob blob = db.getBlob();
    
    if (blob == (*tab)[db.currKey]) {
        sendReply(connfd, "+OK old value verified, new value next");
        status = 3;
    } else {
        sendReply(connfd, "-ERR old value is wrong, CPUT aborted");
        status = 1;
    }
}

// newval 10
void ClientSession::handleNewVal(string command) {
    if (status != 3) {
        sendReply(connfd, "-ERR wrong sequence of command");
        return;
    }
    int pos1 = command.find(SPACE);
    int numBytes = stoi(command.substr(pos1+1));
    
    db.populateBuffer(connfd, numBytes);
    Blob blob = db.getBlob();
    assert (tab->exists(db.currKey));
    
    // Prepare the reply message
    stringstream ss;
    ss << "+OK CPUT " << db.currKey << " successful";
    string okReply = ss.str();

    if (config.isPrimary()) { // this is the primary
        logVerbose("This primary is notifying other replicas");

        tab->put(db.getHash(), db.currKey, blob);
        tab->print();

        int result = notifyReplicas(true);
        
        if (result > 0) {
            sendReply(connfd, okReply.c_str());
            db.reset();
            logVerbose("[%d] Reset data buffer", connfd);
        } else {
            sendReply(connfd, "-ERR CPUT did not go through");
        }
    } else { // this replica forwarding CPUT request to primary
        ForwardContext *context = new ForwardContext(true, &db);
        if (forwardFunc(context) > 0) {
            sendReply(connfd, okReply.c_str());
        }
    }
    status = 1;
}

int ClientSession::notifyReplicas(bool toWrite) {
    vector<Server> &peers = config.myCluster;
    vector<Server> copySet = config.myCluster; // make a copy of peers
    vector<Server>::iterator it = find(copySet.begin(), copySet.end(), config.myServer);
    assert(config.isPrimary());
    copySet.erase(it); // remove primary from copy set of peers
    pthread_t tids[peers.size()]; 
    int primaryIndex; 

    // write to all other tablets
    for (int i = 0; i < peers.size(); i++) {
        Server currPeer = peers[i];
        if (currPeer == config.myServer) { primaryIndex = i; continue; }
        NotifyContext *context = new NotifyContext(toWrite, &db, &copySet, currPeer);
        pthread_create(&tids[i], NULL, &notifyFunc, (void*) context);
    }

    for (int i = 0; i < peers.size(); i++) {
        if (i == primaryIndex) continue;
        pthread_join(tids[i], NULL);
    }

    if (copySet.size() == 0) {
        logVerbose("Got ACKs from all replicas");
        return 1;
    }
    logVerbose("Some replicas did not ACK");
    return -1;
}

// forw put row,col <size> <hash>
// forw del row,col <hash>
// Primary handling forward from a replica
void ClientSession::handleForward(string command) {
    assert (config.isPrimary());
    int pos1 = command.find(SPACE);
    int pos2 = command.find(SPACE, pos1+1);
    string op = command.substr(pos1+1, pos2-pos1-1);
    string origCommand = command.substr(pos1+1);
    int result = 0;
    bool toWrite;
    if (op == "put") {
        int pos1 = origCommand.find(SPACE);
        int pos2 = origCommand.find(SPACE, pos1+1);
        int pos3 = origCommand.find(SPACE, pos2+1);
        string keyString = origCommand.substr(pos1+1, pos2-pos1-1);
        Key key(keyString);
        int numBytes = stoi(origCommand.substr(pos2+1,pos3-pos2-1));
        string hash = origCommand.substr(pos3+1);
        // int counterId = stoi(origCommand.substr(pos3+1));
        if (db.populateAll(connfd, key, numBytes) < 0) return;
        // db.setInvocationId(counterId);
        // put to local tablet first 
        tab->put(hash, db.currKey, db.getBlob());
        toWrite = true;
    } else if (op == "del") {
        int pos1 = origCommand.find(SPACE);
        int pos2 = origCommand.find(SPACE, pos1+1);
        string keyString = origCommand.substr(pos1+1, pos2-pos1-1);
        Key key(keyString);
        string hash = origCommand.substr(pos2+1);
        // int counterId = stoi(origCommand.substr(pos2+1));
        if (db.populateKey(connfd, key) < 0) return;
        // db.setInvocationId(counterId);
        tab->erase(hash, db.currKey);
        toWrite = false;
    }
    result = notifyReplicas(toWrite);
    if (result > 0) sendReply(connfd, "+OK finalized");
}

// writ sel,mel <size> <id> 
void ClientSession::handleWrite(string command) {
    int pos1 = command.find(SPACE);
    int pos2 = command.find(SPACE, pos1+1);
    int pos3 = command.find(SPACE, pos2+1);
    string keyString = command.substr(pos1+1, pos2-pos1-1);
    Key key(keyString);
    int numBytes = stoi(command.substr(pos2+1,pos3-pos2-1));
    string hash = command.substr(pos3+1);
    if (db.populateAll(connfd, key, numBytes) < 0) return;
    Blob b = db.getBlob();
    tab->put(hash, db.currKey, b);
    tab->print();
    db.reset();
    sendReply(connfd, "+OK written");
}

// erase sel,mel <id>
void ClientSession::handleErase(string command) {
    int pos1 = command.find(SPACE);
    int pos2 = command.find(SPACE, pos1+1);
    string keyString = command.substr(pos1+1, pos2-pos1-1);
    Key key(keyString);
    string hash = command.substr(pos2+1);
    // int counterId = stoi(command.substr(pos2+1));
    int res = db.populateKey(connfd, key);
    assert (res > 0);
    tab->erase(hash, db.currKey);
    tab->print();
    sendReply(connfd, "+OK erased");
}

// THREAD FUNCTIONS //
// for primary to notify other replicas
void *notifyFunc(void *params) {
    logVerbose("in notifyFunc");
    
    NotifyContext &context = *((NotifyContext*) params);
    DataBuffer &dbuffer = *context.dataBuffer;
    
    struct sockaddr_in paddr;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) logVerbose("Failed obtaining socket for outbound TCP connection");
    paddr.sin_family = AF_INET;
    paddr.sin_addr.s_addr = context.peer.address;
    paddr.sin_port = context.peer.port;
    int status = connect(sockfd, (struct sockaddr*) &paddr, sizeof(paddr));
    if (status < 0) logVerbose("Tablet cannot connect to backup");

    stringstream ss;
    if (context.toWrite) {
        ss << "writ " << dbuffer.currKey << SPACE << dbuffer.currBlobSize;
        ss << SPACE << dbuffer.getHash() << "\r\n";
    } else {
        ss << "eras " << dbuffer.currKey << SPACE << dbuffer.getHash() << "\r\n";
    }
    string command = ss.str();
    status = write(sockfd, "yo\r\n", 4);
    if (status < 0) {
        logVerbose("Failed writing to peer server");
        return NULL;
    }
    expectToRead(sockfd, "+OK hi", 2);
    status = write(sockfd, command.c_str(), command.size()); 
  
    // op = write
    if (context.toWrite) {
        if (expectToRead(sockfd, "+OK shoot", 5) > 0) {
            status = write(sockfd, dbuffer.blobBuffer, dbuffer.currBlobSize);
            if (status < 0) {
                logVerbose("Failed writing to peer server");
                return NULL;
            }
            if (expectToRead(sockfd, "+OK written", 15) > 0) context.crossOff(); 
        }
    } 
    // op = delete
    else { // waiting for replica to confirm deletion
        if (expectToRead(sockfd, "+OK erased", 5) > 0) context.crossOff(); 
    }
    close(sockfd);
    return NULL;
}

// forward
int forwardFunc(void *params) {
    Server &primary = config.myPrimary; 

    ForwardContext &context = *((ForwardContext*) params);
    DataBuffer &db = *context.dataBuffer;

    int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) logVerbose("Failed creating a socket");
    
    struct sockaddr_in paddr;
    paddr.sin_family = AF_INET;
    paddr.sin_addr.s_addr = primary.address;
    paddr.sin_port = primary.port;

    struct sockaddr_in ownaddr;
    ownaddr.sin_family = AF_INET;
    ownaddr.sin_addr.s_addr = config.myServer.address;
    int portno = ntohs(config.myServer.port) + 3000;
    ownaddr.sin_port = htons(portno);
    int status = ::bind(sockfd, (struct sockaddr*) &ownaddr, sizeof(ownaddr));
    if (status < 0) logVerbose("Cannot bind socket to port");
    status = connect(sockfd, (struct sockaddr*) &paddr, sizeof(paddr));
    if (status < 0) logVerbose("Cannot connect to primary");

    stringstream ss;
    if (context.toWrite) {
        ss << "forw put " << db.currKey << SPACE << db.currBlobSize;
        ss << SPACE << db.getHash() << "\r\n";
    } else ss << "forw del " << db.currKey << SPACE << db.getHash() << "\r\n";
    
    string command = ss.str();
    status = write(sockfd, "yo\r\n", 4);
    if (status < 0)  { logVerbose("Failed initiating session with primary server"); return -1; }
    expectToRead(sockfd, "+OK hi", 2);
    status = write(sockfd, command.c_str(), command.size()); 
    if (status < 0)  { logVerbose("Failed writing to primary server"); return -1; }
    else logVerbose("Written to primary: %s", command.c_str());
   
    if (context.toWrite) {
        // wait for primary to respond with +OK 
        if (expectToRead(sockfd, "+OK shoot", 5) > 0) {
            status = write(sockfd, db.blobBuffer, db.currBlobSize);
            if (status < 0) {
                logVerbose("Failed writing blob to primary server");
                return -1;
            }
        } else { logVerbose("Read something else"); }
    }

    // wait for primary to confirm write or delete
    if (expectToRead(sockfd, "+OK finalized", 15) > 0) {
        close(sockfd);
        return 1;
    } else {
        close(sockfd);
        return -1; 
    }
}
