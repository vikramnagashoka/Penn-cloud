#include "common_r.h"

class MasterConfig {
    public:
        string filename;
        Server publicMaster;
        Server privateMaster;
        vector<Server> servers;
        vector<ServerInfo> serversInfo;
        map<Range, vector<ServerInfo*>> clusters;
        
        friend class boost::serialization::access;
        template<class Archive>
            void serialize(Archive &ar, const unsigned int version) {
                ar & publicMaster;
                ar & servers;
                ar & serversInfo;
                ar & clusters;
            }

        void print();
        static MasterConfig fetchFromMaster(string literal);
};
<EOB>