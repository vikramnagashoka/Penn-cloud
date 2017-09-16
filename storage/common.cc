#include "common.h"

void throwMyError(const char* msg) { fprintf(stderr, "%s\n", msg); exit(1); }
void throwSysError(const char* msg) { perror(msg); exit(1); }

void printLine(string data) {
    printLine("", data.c_str(), data.size(), "\n");
}

void printLine(const char *prefix, const char *data, int len, const char *suffix) {
  printf("%s", prefix);
  for (int i=0; i<len; i++) {
    if (data[i] == '\n')
      printf("<LF>");
    else if (data[i] == '\r') 
      printf("<CR>");
    else if (isprint(data[i])) 
      printf("%c", data[i]);
    else 
      printf("<0x%02X>", (unsigned int)(unsigned char)data[i]);
  }
  printf("%s", suffix);
}

string getFormattedTime() {
    char buffer1[80];
    struct timeval rawtime;
    gettimeofday(&rawtime, NULL);
    struct tm *timeinfo = localtime(&rawtime.tv_sec);
    strftime(buffer1,80,"%R:%S",timeinfo);
    string out(buffer1);
    return out;
}

string rtrim(std::string &s) {
    s.erase(find_if(s.rbegin(), s.rend(),
                std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
}

// TCP connection
// Returns:
// 1 - got the right data
// 0 - time expired
// -1 - error
int expectToRead1(int sockfd, const char *data) {
    char rbuffer[100];
    char cbuffer[100];
    bzero(rbuffer, sizeof(rbuffer));
    bzero(cbuffer, sizeof(cbuffer));
    
    struct pollfd ufd[1];
    ufd[0].fd = sockfd;
    ufd[0].events = POLLIN;
    int status = poll(ufd, 1, 15000);

    if (status == -1) {
        logVerbose("[%d] Error expecting data", sockfd);
        return -1;
    } else if (status == 0) {
        logVerbose("[%d] Timeout occurred while expecting data", sockfd);
        return 0;
    } else {
        if (ufd[0].revents & POLLIN) {
            while (read(sockfd, rbuffer, sizeof(rbuffer)) > 0) {
                char *found = strstr(rbuffer, CRLF);
                if (found) {
                    int len = found - rbuffer;
                    strncat(cbuffer, rbuffer, len);
                    string msg = string(cbuffer);
                    logVerbose("[%d] C: %s", sockfd, msg.c_str());
                    if (msg == data) return 1;
                    else return -1;
                } 
                else { strncat(cbuffer, rbuffer, strlen(rbuffer)); }
                bzero(rbuffer, sizeof(rbuffer));
            }
        }
    }
    return 0;
}

int expectToRead(int sockfd, const char *data, int secs) {
    char rbuffer[100];
    char cbuffer[100];
    bzero(rbuffer, sizeof(rbuffer));
    bzero(cbuffer, sizeof(cbuffer));
    
    struct pollfd ufd[1];
    ufd[0].fd = sockfd;
    ufd[0].events = POLLIN;
    int status = poll(ufd, 1, secs * 1000);

    if (status == -1) { return -1;
    } else if (status == 0) { return 0;
    } else {
        if (ufd[0].revents & POLLIN) {
            while (read(sockfd, rbuffer, sizeof(rbuffer)) > 0) {
                char *found = strstr(rbuffer, CRLF);
                if (found) {
                    int len = found - rbuffer;
                    strncat(cbuffer, rbuffer, len);
                    // logVerbose("[%d] C: %s", sockfd, cbuffer);
                    if (strcmp(cbuffer, data) == 0) return 1;
                    else {
                        return -1;
                    }
                } 
                else { strncat(cbuffer, rbuffer, strlen(rbuffer)); }
                bzero(rbuffer, sizeof(rbuffer));
            }
        }
    }
    return 0;
}
