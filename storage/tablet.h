#ifndef __tablet_h
#define __tablet_h

#include "common.h"
#include "base_structs.h"
using boost::shared_mutex;
using boost::shared_lock;

struct Mutex {
    boost::shared_ptr<shared_mutex> ptr;
    Mutex() : ptr(new shared_mutex()) {}
};

class Tablet {
    private:
        Range range;
        Server server;
        string snapshotDir;
        string snapshotPrefix;
        map<Key,Blob> tab;
        boost::mutex snapMutex;
        map<Key,Mutex> mutexes;
        std::shared_ptr<spd::logger> logger;
        string logfile;
        int snapCount; // counter for snapshots taken
        vector<string> hashes;
        /*
        friend class boost::serialization::access;
        template<class Archive>
            void serialize(Archive &ar, const unsigned int version) {
                ar & tab;
            }
        */
        bool seen(string hash);
        int getLatestSnapshotVersion();
        void replay(int snapshotVersion);
        void put(Key key, Blob blob);
        void erase(Key key);

    public:
        Tablet() {}
        Tablet(Range r, Server s);
        bool inRange(Key key);
        Range& getRange();
        bool exists(Key key);
        Blob& operator[] (Key key);
        Blob get(Key key);
        void put(string hash, Key key, Blob blob);
        void erase(string hash, Key key);
        void print();
        void snapshot();
        void restore();
        string getKeys();
};


#endif
