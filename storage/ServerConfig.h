#include "common.h"
#include "base_structs.h"
#include "tablet.h"

struct ServerConfig {
    int nn;
    int tcpfd;
    int udpfd;
    int restored;
    
    Server publicMaster;
    Server privateMaster;
    vector<Server> servers;
    map<Range, vector<Server>> clusters;
    
    Server myServer;
    Range myRange;
    vector<Server> myCluster;
    Server myPrimary;
    
    Tablet *myTablet;

    bool isPrimary() { return myServer == myPrimary; }
   
    int addToMyCluster(Server target) {
        vector<Server>::iterator it = find(myCluster.begin(), myCluster.end(), target);
        if (it == myCluster.end()) {
            myCluster.push_back(target);
        } 
        return 1;
    }

    int removeFromMyCluster(Server target) {
        vector<Server>::iterator it = find(myCluster.begin(), myCluster.end(), target);
        if (it == myCluster.end()) return -1;
        myCluster.erase(it);
        return 1;
    }

    void print() {
        cout << " ----- TABLET CONFIG ----- " << endl;
        cout << "PUBLIC MASTER: " << publicMaster << endl;
        cout << "THIS RANGE: " << myRange.toString() << endl;
        cout << "ALL SERVERS: " << endl;
        for (int i = 0; i < servers.size(); i++) {
            cout << servers[i] << endl;
        }
        cout << "PEERS IN CLUSTER:" << endl;
        for (int i = 0; i < myCluster.size(); i++) {
            cout << myCluster[i];
            if (myCluster[i] == myPrimary) cout << " (pr)";
            if (myCluster[i] == myServer) cout << " (me)";
            cout << endl;
        }
        cout << "ALL CLUSTERS:" << endl;
        map<Range,vector<Server>>::iterator it;
        for (it = clusters.begin(); it != clusters.end(); it++) {
            cout << it->first << ": ";
            vector<Server> &vec = it->second;
            for (int i = 0; i < vec.size(); i++) {
                cout << vec[i] << " ";
            }
            cout << endl;
        }
    }
};
