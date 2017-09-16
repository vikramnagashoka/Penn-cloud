#include "tablet.h"

bool Tablet::seen(string hash) {
    vector<string>::iterator it = find(hashes.begin(), hashes.end(), hash);
    return it != hashes.end();
}


Tablet::Tablet(Range r, Server s) {
    range = r;
    server = s;
    logfile = "logs/log_" + range.toString() + "_" + server.literal + ".txt";
    logger = spd::basic_logger_mt(range.toString(), logfile);
    logger->flush_on(spd::level::info);
    logger->set_pattern("[ %H:%M:%S.%e] [%t] %v");
    logger->info("Table {} created", range.toString());
    snapshotDir = "snapshots/";
    snapshotPrefix = "tablet_" + range.toString() + "_" + 
            server.literal + "_";
}

bool Tablet::inRange(Key key) {
    return range.inRange(key);
}

Range& Tablet::getRange() {
    return range;
}

bool Tablet::exists(Key key) {
    map<Key,Blob>::iterator it = tab.find(key);
    return it != tab.end();
}

Blob& Tablet::operator[] (Key key) {
    assert (exists(key));
    return tab[key];
}

string Tablet::getKeys() {
    unique_lock<boost::mutex> scoped_lock(snapMutex);
    map<Key,Blob>::iterator it;
    stringstream ss;
    for (it = tab.begin(); it != tab.end(); it++) {
        ss << it->first << "\n"; 
    }
    return ss.str();
}

Blob Tablet::get(Key key) {
    assert (exists(key));
    shared_mutex &mu = *mutexes[key].ptr;
    shared_lock<shared_mutex> lock(mu);
    return tab[key];
}

void Tablet::put(Key key, Blob blob) {
    unique_lock<boost::mutex> scoped_lock(snapMutex);
    shared_mutex &mu = *mutexes[key].ptr;
    boost::upgrade_lock<shared_mutex> upgrade(mu);
    boost::upgrade_to_unique_lock<shared_mutex> uniqueLock(upgrade);
    
    tab[key] = blob; 
    logVerbose("Put %s to tablet", key.toString().c_str());
    logger->info("PUT {},{}", key, blob);
}

void Tablet::put(string hash, Key key, Blob blob) {
    if (seen(hash)) {
        logVerbose("PUT already fulfilled");
        return;
    }
    unique_lock<boost::mutex> scoped_lock(snapMutex);
    shared_mutex &mu = *mutexes[key].ptr;
    boost::upgrade_lock<shared_mutex> upgrade(mu);
    boost::upgrade_to_unique_lock<shared_mutex> uniqueLock(upgrade);
    
    tab[key] = blob; 
    logVerbose("Put %s to tablet", key.toString().c_str());
    hashes.push_back(hash); 
    logger->info("PUT {},{}", key, blob);
}

void Tablet::erase(Key key) {
    if (!exists(key)) return;
    unique_lock<boost::mutex> scoped_lock(snapMutex);
    shared_mutex &mu = *mutexes[key].ptr;
    boost::upgrade_lock<shared_mutex> upgrade(mu);
    boost::upgrade_to_unique_lock<shared_mutex> uniqueLock(upgrade);
    
    tab.erase(key);
    logger->info("DEL {}", key);
}

void Tablet::erase(string hash, Key key) {
    if (!exists(key)) return;
    if (seen(hash)) {
        logVerbose("DEL already fulfilled");
        return;
    }
    unique_lock<boost::mutex> scoped_lock(snapMutex);
    shared_mutex &mu = *mutexes[key].ptr;
    boost::upgrade_lock<shared_mutex> upgrade(mu);
    boost::upgrade_to_unique_lock<shared_mutex> uniqueLock(upgrade);
    
    tab.erase(key);
    hashes.push_back(hash); 
    logger->info("DEL {}", key);
}

// internal debugging
void Tablet::print() {
    map<Key,Blob>::iterator it;
    cout << " --- Tablet ---" << endl;
    for (it = tab.begin(); it != tab.end(); it++) {
        cout << "KEY <" << it->first << ">";
        Blob &blob = it->second;
        int dataBytes = sizeof(blob[0]) * blob.size();
        int totalBytes = sizeof(Blob) + dataBytes;
        double dataKb = dataBytes / 1024;
        double dataMb = dataKb / 1024;
        cout << " (SIZE " << dataMb << "MB, " << dataKb 
            << "KB, " << dataBytes << "B)    ";
        cout << blob << endl;
    }
}

int Tablet::getLatestSnapshotVersion() {
    DIR *dir;
    struct dirent *entry;
    char *p;
    char *pos1, *pos2, *curr, *keep;
    bool seen = false;
    if ((dir = opendir("./snapshots")) != NULL) {
        while ((entry = readdir(dir)) != NULL) {
            curr = entry->d_name;
            p = strstr(curr, snapshotPrefix.c_str()); 
            if (seen && p == NULL) break;
            if (p != NULL) {
                if (!seen) seen = true;
                keep = curr;
            }
        }
    }
    pos1 = strrchr(keep, '_');
    pos2 = strrchr(keep,'.');
    char buffer[10];
    memcpy(buffer, pos1+1, pos2-pos1-1);
    int out = atoi(buffer);
    // cout << pos1 << " | " << pos2 << " | " << out << endl;
    return out;
}

void Tablet::snapshot() {
    unique_lock<boost::mutex> scoped_lock(snapMutex);
    logVerbose("Tablet checkpointing");
    string filename = snapshotDir + snapshotPrefix + to_string(snapCount) + ".out";
    ofstream ofs(filename);
    boost::archive::text_oarchive oa(ofs);
    oa << tab;
    // oa << *this;
    // clear log file
    ofstream fileToClear;
    fileToClear.open(logfile, ofstream::out | ofstream::trunc);
    fileToClear.close();
    logger->info("SNAPSHOT {}", snapCount);

    // clear the 3rd last snapshot
    if (snapCount > 2) {
        string toClear = snapshotDir + snapshotPrefix + to_string(snapCount-3) + ".out";
        if (remove(toClear.c_str()) != 0) {
            logVerbose("Error deleting old snapshot");
        } else {
            logVerbose("Snapshot %d deleted", snapCount-3);
        }
    }
    snapCount++;
}

// for a newly created object
// filename = name of snapshot
// todo
void Tablet::restore() {
    assert (tab.empty());
    int version = getLatestSnapshotVersion(); 
    string filename = snapshotDir + snapshotPrefix + to_string(version) + ".out";
    logVerbose("restoring from %s", filename.c_str());
    ifstream ifs(filename);
    boost::archive::text_iarchive ia(ifs);
    // ia >> tb;
    ia >> tab;
    replay(version);
    print();
    string currfile;
    for (int i = 0; i < 4; i++) {
        int num = version-i;
        currfile = snapshotDir + snapshotPrefix + to_string(num) + ".out";
        if (remove(currfile.c_str()) != 0) {
            logVerbose("Error deleting %s", currfile.c_str());
        } else {
            logVerbose("Deleted %s", currfile.c_str());
        }
    }
}

//todo: move to private
void Tablet::replay(int snapshotVersion) {
    string filename = "logs/log_" + range.toString() + ".txt";
    ifstream ifs(filename);
    string snapEntry = "SNAPSHOT " + to_string(snapshotVersion);
    string line;
    long pput, pdel;
    long pos1, pos2;
    int found = 0;
    while (getline(ifs, line)) {
        if (line.find(snapEntry) != line.npos) {
            cout << "found snapshot entry in log!" << endl;
            found = 1;
            continue;
        }
        if (found == 0) continue;
        pdel = line.find("DEL ");
        pput = line.find("PUT ");
        if (pdel != line.npos) {
            pos1 = line.find(COMMA,pdel+4);
            string row = line.substr(pdel+4,pos1-pdel-4);
            string col = line.substr(pos1+1);
            Key key(row,col);
            erase(key); 
            print();
        } else if (pput != line.npos) {
            pos1 = line.find(COMMA,pput+4);
            pos2 = line.find(COMMA, pos1+1);
            string row = line.substr(pput+4,pos1-pput-4);
            string col = line.substr(pos1+1,pos2-pos1-1);
            string data = line.substr(pos2+1);
            Key key(row,col);
            Blob blob(data.begin(), data.end());
            put(key, blob);
        }

    }
}
