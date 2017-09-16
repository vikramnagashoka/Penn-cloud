#include<iostream>
#include<stdlib.h>
#include<stdio.h>
#include<cstdlib>
#include<cstdio>
#include<iomanip>
#include<unistd.h>
#include<cstring>
#include<vector>
#include<istream>
#include<ostream>
#include<algorithm>
#include<fstream>
#include<string>
#include<vector>
#include <arpa/inet.h>
int main()
{	
	int portNo = 8000;
    int sockfd = socket(PF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        std::cout << "cannot open socket\n";
        exit(-1);
    }

    struct sockaddr_in servaddr;
    
    bzero(&servaddr, sizeof(servaddr));
    int enable = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(int));

    servaddr.sin_family = PF_INET;
    servaddr.sin_port = htons(portNo);
    servaddr.sin_addr.s_addr = htons(INADDR_ANY);

    inet_pton(PF_INET, "127.0.0.1", &(servaddr.sin_addr));
    /*bind and listen*/
    if(connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0){
    	perror("Unable to connect");
    	exit(-1);
    }
	return 0;
}