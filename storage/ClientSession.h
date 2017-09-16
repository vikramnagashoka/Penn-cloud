#include "tablet_common.h"

// read data by set number of bytes to buffer
inline void readDataToBuffer(int connfd, unsigned char *buffer, int numBytes) {
    bzero(buffer, sizeof(buffer));
    unsigned char *ptr = buffer;
    int n = numBytes;
    while (numBytes > 0) {
        int num = recv(connfd, ptr, sizeof(buffer), 0);
        if (num < 0) {
            logVerbose("Error receiving data to buffer");
        }
        numBytes -= num;
        ptr += num;
    }
}

struct DataBuffer {
    size_t hashSeed;
    int invoId;
    Key currKey;
    int currBlobSize;
    unsigned char blobBuffer[MAXBLOBSIZE];
    string currHash;

    void reset() {
        currKey = {"",""};
        int currBlobSize = 0;
        bzero(blobBuffer, sizeof(blobBuffer));
    }

    void setInvocationId(int id) {
        invoId = id;
        size_t out = hashSeed;
        boost::hash_combine(out, id);
        char str[256];
        snprintf(str, sizeof str, "%zu", out);
        currHash = string(str);
    }

    string getHash() {
        return currHash;
    }
    
    // command = <action> sel,mel
    int populateKey(int connfd, Key key) {
        if (config.myRange.inRange(key)) {
            currKey = key;
            logVerbose("Queried key set successfully");
            return 1;
        } else {
            sendReply(connfd, "-ERR key out of range");
            return -1;
        }
    }

    // command = <action> 5
    void populateBuffer(int connfd, int size) {
        currBlobSize = size;
        readDataToBuffer(connfd, blobBuffer, currBlobSize); 
    }

    // command = <action> sel,mel 5
    int populateAll(int connfd, Key key, int size) {
        if (!config.myRange.inRange(key)) {
            sendReply(connfd, "-ERR key out of range");
            return -1;
        }
        currKey = key;
        logVerbose("Queried key set successfully");
        currBlobSize = size;
        sendReply(connfd, "+OK shoot");
        readDataToBuffer(connfd, blobBuffer, currBlobSize);
        logVerbose("populated data buffer: %s", blobBuffer);
        return 1;
    }

    Blob getBlob() {
        return Blob(blobBuffer, blobBuffer + currBlobSize);
    }
};

// Context for primary server to notify other replicas
struct NotifyContext {
    bool toWrite; // true for write, false for del
    DataBuffer *dataBuffer;
    vector<Server> *peersSet;
    Server peer;
    int total;
    
    NotifyContext(bool write, DataBuffer *buffer, vector<Server> *set, Server p) {
        toWrite = write;
        dataBuffer = buffer;
        peer = p;
        peersSet = set;
        total = set->size();
    }

    void crossOff() {
        vector<Server> &set = *peersSet;
        vector<Server>::iterator it = find(set.begin(), set.end(), peer);
        if (it != set.end()) {
            set.erase(it);
            logVerbose("Crossed off server %s (%lu/%d)", 
                    peer.literal.c_str(), set.size(), total);
        } else {
            logVerbose("(Error) Peer server not in set");
        }
    }
};

// Context for replica servers to forward request to primary server
struct ForwardContext {
    bool toWrite;
    DataBuffer *dataBuffer;
    ForwardContext(int write, DataBuffer *db): 
        toWrite(write), dataBuffer(db) {}
};

// If primary needs to notify writes to replicas, it starts a thread with this function
void *notifyFunc(void *params);

// a blocking function called by replicas that waits for primary server to respond
int forwardFunc(void *params);

class ClientSession { 
    private: 
        pthread_t tid; 
        int status = 1;
        int connfd; 
        DataBuffer db;
        Tablet *tab = config.myTablet;
        int notifyReplicas(bool write);
    public: 
        ClientSession(int fd, pthread_t threadId);
        int getConnfd() { return connfd; }
        void handleGet(string command);
        void handlePut(string command);
        void handleDel(string command);
        void handleCput(string command);
        void handleOldVal(string command);
        void handleNewVal(string command);
        void handleForward(string command);
        void handleWrite(string command);
        void handleErase(string command);
        void handleKval();
        void handleQuit();
        void handleRset();
};


