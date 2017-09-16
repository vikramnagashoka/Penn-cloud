// #include "tablet_common.h"
#include "base_structs.h"

class RangeCluster {
    Range range;
    vector<Server> servers;
    
    bool operator == (const RangeCluster &a) const {
        return range == a.range;
    }

    public:
        RangeCluster(Range kr) {
            range = kr;
        }
        Range& getRange() {
            return range;
        }
        bool inRange(Key &k) {
            return range.inRange(k);
        }
        bool hasServer(Server s) {
            vector<Server>::iterator it = find(servers.begin(), servers.end(), s);
            return it != servers.end();
        }
        void addServer(Server s) {
            if (hasServer(s)) return;
            servers.push_back(s);
        }
        Server getServer(int ind) {
            assert (ind < servers.size());
            return servers[ind]; 
        }
        
        int size() {
            return servers.size();
        }

        void removeServer(Server s) {
            if (!hasServer(s)) return;
            vector<Server>::iterator it = find(servers.begin(), servers.end(), s);
            servers.erase(it);
        }

        friend ostream& operator << (ostream &os, const RangeCluster &r) {
            os << "RANGE " << r.range.toString() << endl;
            for (int i = 0; i < r.servers.size(); i++) {
                os << r.servers[i];
                os << endl;
            }
            return os;
        }
        
        void broadcast(int sockfd, string msg) {
            struct sockaddr_in addr; bzero(&addr, sizeof(addr));
            addr.sin_family = AF_INET;
            long address;
            int port;
            for (int i = 0; i < servers.size(); i++) {
                address = servers[i].address;
                port = servers[i].port;
                addr.sin_addr.s_addr = address;
                addr.sin_port = port;
                int status = sendto(sockfd, msg.c_str(), msg.size(), 0, 
                                (struct sockaddr*) &addr, sizeof(addr));
                if (status < 0) {
                    //todo
                }
            }
        }
};
