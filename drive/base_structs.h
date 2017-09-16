#ifndef __base_structs_h
#define __base_structs_h

#include "common_r.h"

// Key
struct Key { 
    string row; 
    string col; 
    string literal;
    bool operator < (const Key &a) const {
        return tie(row, col) < tie(a.row, a.col);
    }
    bool operator == (const Key &a) const {
        return row == a.row && col == a.col;
    }
    bool operator <= (const Key &a) const {
        return *this == a || *this < a;
    }
    Key() {}
    Key(string r, string c) { row = r; col = c; literal = r + "," + c; }
    Key(string s) {
        int pos = s.find(COMMA);
        literal = s;
        row = s.substr(0,pos);
        col = s.substr(pos+1);
    }
    friend ostream& operator << (ostream& os, const Key &k) {
        os << k.literal;
        return os;
    }
    
    string toString() { return literal; }

    private:
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive &ar, const unsigned int version) {
            ar & row;
            ar & col;
        }
};

// Blob
typedef vector<unsigned char> Blob;
inline bool operator == (const Blob &a, const Blob &b) {
    if (a.size() != b.size()) return false;
    for (int i = 0; i < a.size(); i++) {
        if (a[i] != b[i]) return false;
    }
    return true;
}
inline ostream& operator << (ostream& os, const Blob &b) {
    for (int i = 0; i < b.size(); i++) {
        os << b[i];
    }
    return os;
}


// Range
struct Range {
    pair<char,char> range;
    string literal;

    Range() {}
    Range(string input) {
        literal = input;
        int pos = input.find("-");
        range = make_pair(input[0],input[pos+1]);
    }

    string& toString() {
        return literal;
    }
    
    string toString() const {
        return literal;
    }
    
    bool inRange(Key &k) {
        return range.first <= k.row[0] && k.row[0] <= range.second;
    }
    
    bool inRange(Key &k) const {
        return range.first <= k.row[0] && k.row[0] <= range.second;
    }
    bool operator < (const Range &k) const {
        return range < k.range;
    }
    bool operator == (const Range &k) const {
        return range == k.range;
    }
    bool operator <= (const Range &k) const {
        return range <= k.range;
    }
    friend ostream& operator << (ostream& os, const Range &s) {
        os << s.literal; return os;
    }

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar & literal;
    }
};


// Server
struct Server {
    string literal;
    uint32_t address; // in network byte order
    uint16_t port; // in network byte order

    bool operator == (const Server &a) const {
        return literal == a.literal;
    }
   
    Server() {}
    // arguments are in network byte order
    Server(uint32_t a, uint16_t p) {
        address = a;
        port = p;
        in_addr wrapper; wrapper.s_addr = a;
        literal = inet_ntoa(wrapper) + string(":") + to_string(ntohs(p));
    }

    Server(string input) {
        literal = input;
        int pos = literal.find(COLON);
        // convert to network byte order
        address = inet_addr(literal.substr(0,pos).c_str());
        port = htons(stoi(literal.substr(pos+1)));
    }

    friend ostream& operator << (ostream& os, const Server &s) {
        os << s.literal; return os;
    }

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar & literal;
    }
};

// for master
struct ServerInfo {
    // data
    int index;
    Server server;
    Range range;
    
    bool alive = true;
    bool isPrimary = false;
    int sockfd = -1;
  
    ServerInfo() {}
    ServerInfo(int i, Server s): index(i), server(s) {}
    bool isHealthyBackup() { return alive && sockfd > 0 && !isPrimary; }
    bool isHealthy() { return alive && sockfd > 0; }

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar & server;
        ar & alive;
        ar & range;
        bool isPrimary;
    }
    friend ostream& operator << (ostream& os, const ServerInfo &si) {
        os << si.server << " | ";
        os << "range = " << si.range << " | ";
        os << "alive = " << si.alive << " | ";
        os << "isPrimary = " << si.isPrimary << " | ";
        os << "sockfd = " << si.sockfd << " | ";
        os << endl;
        return os;
    }
};
#endif
<EOB>#ifndef __base_structs_h
#define __base_structs_h

#include "common_r.h"

// Key
struct Key { 
    string row; 
    string col; 
    string literal;
    bool operator < (const Key &a) const {
        return tie(row, col) < tie(a.row, a.col);
    }
    bool operator == (const Key &a) const {
        return row == a.row && col == a.col;
    }
    bool operator <= (const Key &a) const {
        return *this == a || *this < a;
    }
    Key() {}
    Key(string r, string c) { row = r; col = c; literal = r + "," + c; }
    Key(string s) {
        int pos = s.find(COMMA);
        literal = s;
        row = s.substr(0,pos);
        col = s.substr(pos+1);
    }
    friend ostream& operator << (ostream& os, const Key &k) {
        os << k.literal;
        return os;
    }
    
    string toString() { return literal; }

    private:
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive &ar, const unsigned int version) {
            ar & row;
            ar & col;
        }
};

// Blob
typedef vector<unsigned char> Blob;
inline bool operator == (const Blob &a, const Blob &b) {
    if (a.size() != b.size()) return false;
    for (int i = 0; i < a.size(); i++) {
        if (a[i] != b[i]) return false;
    }
    return true;
}
inline ostream& operator << (ostream& os, const Blob &b) {
    for (int i = 0; i < b.size(); i++) {
        os << b[i];
    }
    return os;
}


// Range
struct Range {
    pair<char,char> range;
    string literal;

    Range() {}
    Range(string input) {
        literal = input;
        int pos = input.find("-");
        range = make_pair(input[0],input[pos+1]);
    }

    string& toString() {
        return literal;
    }
    
    string toString() const {
        return literal;
    }
    
    bool inRange(Key &k) {
        return range.first <= k.row[0] && k.row[0] <= range.second;
    }
    
    bool inRange(Key &k) const {
        return range.first <= k.row[0] && k.row[0] <= range.second;
    }
    bool operator < (const Range &k) const {
        return range < k.range;
    }
    bool operator == (const Range &k) const {
        return range == k.range;
    }
    bool operator <= (const Range &k) const {
        return range <= k.range;
    }
    friend ostream& operator << (ostream& os, const Range &s) {
        os << s.literal; return os;
    }

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar & literal;
    }
};


// Server
struct Server {
    string literal;
    uint32_t address; // in network byte order
    uint16_t port; // in network byte order

    bool operator == (const Server &a) const {
        return literal == a.literal;
    }
   
    Server() {}
    // arguments are in network byte order
    Server(uint32_t a, uint16_t p) {
        address = a;
        port = p;
        in_addr wrapper; wrapper.s_addr = a;
        literal = inet_ntoa(wrapper) + string(":") + to_string(ntohs(p));
    }

    Server(string input) {
        literal = input;
        int pos = literal.find(COLON);
        // convert to network byte order
        address = inet_addr(literal.substr(0,pos).c_str());
        port = htons(stoi(literal.substr(pos+1)));
    }

    friend ostream& operator << (ostream& os, const Server &s) {
        os << s.literal; return os;
    }

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar & literal;
    }
};

// for master
struct ServerInfo {
    // data
    int index;
    Server server;
    Range range;
    
    bool alive = true;
    bool isPrimary = false;
    int sockfd = -1;
  
    ServerInfo() {}
    ServerInfo(int i, Server s): index(i), server(s) {}
    bool isHealthyBackup() { return alive && sockfd > 0 && !isPrimary; }
    bool isHealthy() { return alive && sockfd > 0; }

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar & server;
        ar & alive;
        ar & range;
        bool isPrimary;
    }
    friend ostream& operator << (ostream& os, const ServerInfo &si) {
        os << si.server << " | ";
        os << "range = " << si.range << " | ";
        os << "alive = " << si.alive << " | ";
        os << "isPrimary = " << si.isPrimary << " | ";
        os << "sockfd = " << si.sockfd << " | ";
        os << endl;
        return os;
    }
};
#endif
<EOB>#ifndef __base_structs_h
#define __base_structs_h

#include "common_r.h"

// Key
struct Key { 
    string row; 
    string col; 
    string literal;
    bool operator < (const Key &a) const {
        return tie(row, col) < tie(a.row, a.col);
    }
    bool operator == (const Key &a) const {
        return row == a.row && col == a.col;
    }
    bool operator <= (const Key &a) const {
        return *this == a || *this < a;
    }
    Key() {}
    Key(string r, string c) { row = r; col = c; literal = r + "," + c; }
    Key(string s) {
        int pos = s.find(COMMA);
        literal = s;
        row = s.substr(0,pos);
        col = s.substr(pos+1);
    }
    friend ostream& operator << (ostream& os, const Key &k) {
        os << k.literal;
        return os;
    }
    
    string toString() { return literal; }

    private:
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive &ar, const unsigned int version) {
            ar & row;
            ar & col;
        }
};

// Blob
typedef vector<unsigned char> Blob;
inline bool operator == (const Blob &a, const Blob &b) {
    if (a.size() != b.size()) return false;
    for (int i = 0; i < a.size(); i++) {
        if (a[i] != b[i]) return false;
    }
    return true;
}
inline ostream& operator << (ostream& os, const Blob &b) {
    for (int i = 0; i < b.size(); i++) {
        os << b[i];
    }
    return os;
}


// Range
struct Range {
    pair<char,char> range;
    string literal;

    Range() {}
    Range(string input) {
        literal = input;
        int pos = input.find("-");
        range = make_pair(input[0],input[pos+1]);
    }

    string& toString() {
        return literal;
    }
    
    string toString() const {
        return literal;
    }
    
    bool inRange(Key &k) {
        return range.first <= k.row[0] && k.row[0] <= range.second;
    }
    
    bool inRange(Key &k) const {
        return range.first <= k.row[0] && k.row[0] <= range.second;
    }
    bool operator < (const Range &k) const {
        return range < k.range;
    }
    bool operator == (const Range &k) const {
        return range == k.range;
    }
    bool operator <= (const Range &k) const {
        return range <= k.range;
    }
    friend ostream& operator << (ostream& os, const Range &s) {
        os << s.literal; return os;
    }

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar & literal;
    }
};


// Server
struct Server {
    string literal;
    uint32_t address; // in network byte order
    uint16_t port; // in network byte order

    bool operator == (const Server &a) const {
        return literal == a.literal;
    }
   
    Server() {}
    // arguments are in network byte order
    Server(uint32_t a, uint16_t p) {
        address = a;
        port = p;
        in_addr wrapper; wrapper.s_addr = a;
        literal = inet_ntoa(wrapper) + string(":") + to_string(ntohs(p));
    }

    Server(string input) {
        literal = input;
        int pos = literal.find(COLON);
        // convert to network byte order
        address = inet_addr(literal.substr(0,pos).c_str());
        port = htons(stoi(literal.substr(pos+1)));
    }

    friend ostream& operator << (ostream& os, const Server &s) {
        os << s.literal; return os;
    }

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar & literal;
    }
};

// for master
struct ServerInfo {
    // data
    int index;
    Server server;
    Range range;
    
    bool alive = true;
    bool isPrimary = false;
    int sockfd = -1;
  
    ServerInfo() {}
    ServerInfo(int i, Server s): index(i), server(s) {}
    bool isHealthyBackup() { return alive && sockfd > 0 && !isPrimary; }
    bool isHealthy() { return alive && sockfd > 0; }

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar & server;
        ar & alive;
        ar & range;
        bool isPrimary;
    }
    friend ostream& operator << (ostream& os, const ServerInfo &si) {
        os << si.server << " | ";
        os << "range = " << si.range << " | ";
        os << "alive = " << si.alive << " | ";
        os << "isPrimary = " << si.isPrimary << " | ";
        os << "sockfd = " << si.sockfd << " | ";
        os << endl;
        return os;
    }
};
#endif
#ifndef __base_structs_h
#define __base_structs_h

#include "common_r.h"

// Key
struct Key { 
    string row; 
    string col; 
    string literal;
    bool operator <<EOB>#ifndef __base_structs_h
#define __base_structs_h

#include "common_r.h"

// Key
struct Key { 
    string row; 
    string col; 
    string literal;
    bool operator < (const Key &a) const {
        return tie(row, col) < tie(a.row, a.col);
    }
    bool operator == (const Key &a) const {
        return row == a.row && col == a.col;
    }
    bool operator <= (const Key &a) const {
        return *this == a || *this < a;
    }
    Key() {}
    Key(string r, string c) { row = r; col = c; literal = r + "," + c; }
    Key(string s) {
        int pos = s.find(COMMA);
        literal = s;
        row = s.substr(0,pos);
        col = s.substr(pos+1);
    }
    friend ostream& operator << (ostream& os, const Key &k) {
        os << k.literal;
        return os;
    }
    
    string toString() { return literal; }

    private:
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive &ar, const unsigned int version) {
            ar & row;
            ar & col;
        }
};

// Blob
typedef vector<unsigned char> Blob;
inline bool operator == (const Blob &a, const Blob &b) {
    if (a.size() != b.size()) return false;
    for (int i = 0; i < a.size(); i++) {
        if (a[i] != b[i]) return false;
    }
    return true;
}
inline ostream& operator << (ostream& os, const Blob &b) {
    for (int i = 0; i < b.size(); i++) {
        os << b[i];
    }
    return os;
}


// Range
struct Range {
    pair<char,char> range;
    string literal;

    Range() {}
    Range(string input) {
        literal = input;
        int pos = input.find("-");
        range = make_pair(input[0],input[pos+1]);
    }

    string& toString() {
        return literal;
    }
    
    string toString() const {
        return literal;
    }
    
    bool inRange(Key &k) {
        return range.first <= k.row[0] && k.row[0] <= range.second;
    }
    
    bool inRange(Key &k) const {
        return range.first <= k.row[0] && k.row[0] <= range.second;
    }
    bool operator < (const Range &k) const {
        return range < k.range;
    }
    bool operator == (const Range &k) const {
        return range == k.range;
    }
    bool operator <= (const Range &k) const {
        return range <= k.range;
    }
    friend ostream& operator << (ostream& os, const Range &s) {
        os << s.literal; return os;
    }

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar & literal;
    }
};


// Server
struct Server {
    string literal;
    uint32_t address; // in network byte order
    uint16_t port; // in network byte order

    bool operator == (const Server &a) const {
        return literal == a.literal;
    }
   
    Server() {}
    // arguments are in network byte order
    Server(uint32_t a, uint16_t p) {
        address = a;
        port = p;
        in_addr wrapper; wrapper.s_addr = a;
        literal = inet_ntoa(wrapper) + string(":") + to_string(ntohs(p));
    }

    Server(string input) {
        literal = input;
        int pos = literal.find(COLON);
        // convert to network byte order
        address = inet_addr(literal.substr(0,pos).c_str());
        port = htons(stoi(literal.substr(pos+1)));
    }

    friend ostream& operator << (ostream& os, const Server &s) {
        os << s.literal; return os;
    }

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar & literal;
    }
};

// for master
struct ServerInfo {
    // data
    int index;
    Server server;
    Range range;
    
    bool alive = true;
    bool isPrimary = false;
    int sockfd = -1;
  
    ServerInfo() {}
    ServerInfo(int i, Server s): index(i), server(s) {}
    bool isHealthyBackup() { return alive && sockfd > 0 && !isPrimary; }
    bool isHealthy() { return alive && sockfd > 0; }

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar & server;
        ar & alive;
        ar & range;
        bool isPrimary;
    }
    friend ostream& operator << (ostream& os, const ServerInfo &si) {
        os << si.server << " | ";
        os << "range = " << si.range << " | ";
        os << "alive = " << si.alive << " | ";
        os << "isPrimary = " << si.isPrimary << " | ";
        os << "sockfd = " << si.sockfd << " | ";
        os << endl;
        return os;
    }
};
#endif
#ifndef __base_structs_h
#define __base_structs_h

#include "common_r.h"

// Key
struct Key { 
    string row; 
    string col; 
    string literal;
    bool operator <<EOB>