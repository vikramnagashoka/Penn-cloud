#ifndef __common_h
#define __common_h

#include <errno.h>
#include <sysexits.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sstream>
#include <iostream>
#include <fstream>
#include <string.h>
#include <algorithm>
#include <vector>
#include <regex>
#include <map>
#include <set>
#include <dirent.h>
#include <ctime>
#include <iomanip>

#include <sys/types.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <arpa/inet.h>

#include <jsoncpp/json/json.h>
#include <boost/array.hpp>
#include <boost/thread.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <spdlog/spdlog.h>

#define COMMA ","
#define COLON ":"
#define CRLF "\r\n"

using namespace std;
namespace spd = spdlog;

extern int verbose;

#define logVerbose(a...) do { if (verbose) { struct timeval tv; gettimeofday(&tv, NULL); printf("%d.%03d ", (int)tv.tv_sec, (int)(tv.tv_usec/1000)); printf(a); printf("\n"); } } while(0)

void throwSysError(const char* msg);
void throwMyError(const char* msg);
void printLine(const char *prefix, const char *data, int len, const char *suffix);
void printLine(string data);
int expectToRead(int sockfd, const char *data, int secs);
string getFormattedTime();
string rtrim(string &s);

#include "base_structs.h"

// methods
#endif
