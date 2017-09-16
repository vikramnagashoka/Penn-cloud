#ifndef __common_r_h
#define __common_r_h

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sstream>
#include <iostream>
#include <fstream>
#include <string.h>
#include <algorithm>
#include <vector>
#include <map>
#include <set>
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

#include <boost/array.hpp>
#include <boost/thread.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>


#define COMMA ","
#define COLON ":"

using namespace std;
extern int verbose;

#define logVerbose(a...) do { if (verbose) { struct timeval tv; gettimeofday(&tv, NULL); printf("%d.%03d ", (int)tv.tv_sec, (int)(tv.tv_usec/1000)); printf(a); printf("\n"); } } while(0)

#include "base_structs.h"

#endif
#ifndef __common_r_h
#define __common_r_h

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sstream>
#include <iostream>
#include <fstream>
#include <string.h>
#include <algorithm>
#include <vector>
#include <map>
#include <set>
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

#include <boost/array.hpp>
#include <boost/thread.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>


#define COMMA ","
#define COLON ":"

using namespace std;
extern int verbose;

#define logVerbose(a...) do { if (verbose) { struct timeval tv; gettimeofday(&tv, NULL); printf("%d.%03d ", (int)tv.tv_sec, (int)(tv.tv_usec/1000)); printf(a); printf("\n"); } } while(0)

#include "base_structs.h"

#endif
