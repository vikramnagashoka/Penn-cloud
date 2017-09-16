#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<signal.h>
#include<fcntl.h>
#include<iostream>
#include<string>
#include<iomanip>
#include<sstream>
#include <netinet/in.h>
using namespace std;
#define CONNMAX 1000
#define BYTES 1024

char *ROOT;
int listenfd, clients[CONNMAX];
void startServer(char *);
void respond(int);
char *fname, *lname, *uname, *pswd, *cpswd;

int main(int argc, char* argv[]){
    struct sockaddr_in clientaddr;
    socklen_t addrlen;
    char c;

    //Default Values PATH = ~/ and PORT=10000
    char PORT[6];
    ROOT = getenv("PWD");
    strcpy(PORT,"10000");

    int slot=0;

    while ((c = getopt (argc, argv, "p:")) != -1){
        switch (c){
            case 'p':
                strcpy(PORT,optarg);
                break;
            case '?':
                fprintf(stderr,"Wrong arguments given!!!\n");
                exit(1);
            default:
                exit(1);
        }
    }
    printf("Server started at port no. %s with root directory as %s\n",PORT,ROOT);
    int i;
    for (i=0; i<CONNMAX; i++){
    	clients[i]=-1;
    }
    startServer(PORT);

    while (1){
        addrlen = sizeof(clientaddr);
        clients[slot] = accept (listenfd, (struct sockaddr *) &clientaddr, &addrlen);

        if (clients[slot]<0){
            //error ("accept() error");
        }
        else{
            if ( fork()==0 ){
                respond(slot);
                exit(0);
            }
        }

        while (clients[slot]!=-1){
        	slot = (slot+1)%CONNMAX;
        }
    }
    return 0;
}

void startServer(char *port){
    struct addrinfo hints, *res, *p;

    memset (&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if (getaddrinfo( NULL, port, &hints, &res) != 0){
        perror ("getaddrinfo() error");
        exit(1);
    }
    for (p = res; p!=NULL; p=p->ai_next){
        listenfd = socket (p->ai_family, p->ai_socktype, 0);
        if (listenfd == -1){
        	continue;
        }
        if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0){
        	break;
        }
    }
    if (p==NULL){
        perror ("socket() or bind()");
        exit(1);
    }
    freeaddrinfo(res);

    // listen for incoming connections
    if ( listen (listenfd, 1000000) != 0 ){
        perror("listen() error");
        exit(1);
    }
}

void respond(int n){

	char mesg[99999], *reqline[3], data_to_send[BYTES], path[99999];
    int rcvd, fd, bytes_read;

    memset( (void*)mesg, (int)'\0', 99999 );

    rcvd=recv(clients[n], mesg, 99999, 0);

    if (rcvd<0){
    	fprintf(stderr,("recv() error\n"));
    }
    else if (rcvd==0){
    	fprintf(stderr,"Client disconnected unexpectedly.\n");
    }
    else{
        printf("%s", mesg);
        char *store = (char*)malloc (1 + strlen (mesg));
        strcpy(store, mesg);

        reqline[0] = strtok (mesg, " \t\n");
        if ( strncmp(reqline[0], "GET\0", 4)==0 ){

        	reqline[1] = strtok (NULL, " \t");
        	reqline[2] = strtok (NULL, " \t\n");

            if ( strncmp( reqline[2], "HTTP/1.0", 8)!=0 && strncmp( reqline[2], "HTTP/1.1", 8)!=0 ){
                write(clients[n], "HTTP/1.1 400 Bad Request\n", 25);
            }
            else{

                if( strncmp(reqline[1], "/\0", 2)==0 ){
                	reqline[1] = "/index.html";
                }

                strcpy(path, ROOT);
                strcpy(&path[strlen(ROOT)], reqline[1]);
                printf("file: %s\n", path);

                if ((strcmp(path, "/home/cis505/git/http/login.html") == 0) || (strcmp(path, "/home/cis505/git/http/signup.html") == 0)){


                    if ( (fd=open(path, O_RDONLY))!=-1 ){
                		send(clients[n], "HTTP/1.1 200 OK\n\n", 17, 0);
                		while ( (bytes_read=read(fd, data_to_send, BYTES))>0 ){
                			write (clients[n], data_to_send, bytes_read);
                		}
                	}
                }

                else if ((strcmp(path, "/home/cis505/git/http/mail.html") == 0)){
                	cout<<"\n\nMAIL\n\n";
                	if ( (fd=open(path, O_RDONLY))!=-1 ){
                		send(clients[n], "HTTP/1.1 200 OK\n\n", 17, 0);
                		while ( (bytes_read=read(fd, data_to_send, BYTES))>0 ){
                			write (clients[n], data_to_send, bytes_read);
                		}
                	}
                }
                else if ((strcmp(path, "/home/cis505/git/http/drive.html") == 0)){

                	if ( (fd=open(path, O_RDONLY))!=-1 ){
                		send(clients[n], "HTTP/1.1 200 OK\n\n", 17, 0);
                		while ( (bytes_read=read(fd, data_to_send, BYTES))>0 ){
                			write (clients[n], data_to_send, bytes_read);
                		}
                	}
                }

                else{

                	if ( (fd=open(path, O_RDONLY))!=-1 ){
                    	send(clients[n], "HTTP/1.1 200 OK\n", 16, 0);
               			send(clients[n], "set-cookie: id=demo\n\n", 21, 0);
                    	while ( (bytes_read=read(fd, data_to_send, BYTES))>0 ){
                    		write (clients[n], data_to_send, bytes_read);
                    	}
                	}

                	else{
                		write(clients[n], "HTTP/1.1 404 Not Found\n", 23);
                	}
                }
            }
        }

        else if( strncmp(reqline[0], "POST\0", 5)==0 ){
            //printf("HELLO");
        	reqline[1] = strtok (NULL, " \t");
        	reqline[2] = strtok (NULL, " \t\n");

        	if ( strncmp( reqline[2], "HTTP/1.0", 8)!=0 && strncmp( reqline[2], "HTTP/1.1", 8)!=0 ){
                write(clients[n], "HTTP/1.1 400 Bad Request\n", 25);
            }

        	else{

        		if ( strncmp(reqline[1], "/\0", 2)==0 ){
                	reqline[1] = "/index.html";
                }
                strcpy(path, ROOT);
                strcpy(&path[strlen(ROOT)], reqline[1]);
                printf("file: %s\n", path);

                if (strcmp(path, "/home/cis505/git/http/login.html") == 0){
                	//check details with storage
                	//if logged in, save username locally first. then take to welcome
                	//else refresh login page.

                	char* temp;

                	if ( (temp = strcasestr(store,"username=")) != NULL ){
                		temp+=9;
                		//printf("\n\nHI%s\n\n", temp);
                		int count = 0;
                		char *temp1 = (char*)malloc (1 + strlen (temp));
                		strcpy(temp1, temp);
                		while (temp[0] != '&'){
                			count ++;
                			temp++;
                		}
                		//printf("\n\nHELLO%s%d\n\n", temp1, count);
                		uname = (char*)malloc(count);
                		strncpy(uname, temp1, count);
                		printf("\n%s\n", uname);
                	}

                	if ( (temp = strcasestr(store,"password=")) != NULL ){
                		temp+=9;
                		pswd = (char*)malloc(strlen(temp));
                		strcpy(pswd, temp);
                		printf("\n%s\n", pswd);
                	}
                	string s = "row ";
                	s.append(uname);
                	char *c = (char*)s.c_str();

                	/*int sock = socket(PF_INET, SOCK_STREAM, 0);
                	struct sockaddr_in dest;
                	dest.sin_family = AF_INET;
                	dest.sin_port = htons(5000);
                	inet_pton(AF_INET, "127.0.0.1", &(dest.sin_addr));
                	sendto(sock, c, sizeof(c), 0, (struct sockaddr*)&dest, sizeof(dest));
                	printf("Hello message sent\n");
                	char buffer[1024] = {0};
                	int valread = read(sock , buffer, 1024);
                	printf("%s\n",buffer );*/

                	//STUB
                	bool auth = true;
                	if (auth == true){
                		reqline[1] = "/welcome.html";
                		strcpy(path, ROOT);
                		strcpy(&path[strlen(ROOT)], reqline[1]);
//               		printf("file: %s\n", path);
                        string s = "set-cookie: id=";
                        s.append(uname);
                        s.append(";\n\n");
                        char *c = (char*)s.c_str();
               			//send(clients[n], "HTTP/1.1 200 OK\n", 16, 0);
               			//send(clients[n], "set-cookie: id=uname;\n\n", 23, 0);
           				//write (clients[n], c, s.size());
                        if ( (fd=open(path, O_RDONLY))!=-1 ){
           					send(clients[n], "HTTP/1.1 200 OK\n", 16, 0);
           				    send(clients[n], c, s.size(), 0);
           				    while ( (bytes_read=read(fd, data_to_send, BYTES))>0 ){
           				    	write (clients[n], data_to_send, bytes_read);
           				    }
           				}
              		}
                	else{
                		if ( (fd=open(path, O_RDONLY))!=-1 ){
                			send(clients[n], "HTTP/1.1 200 OK\n", 16, 0);
                		    while ( (bytes_read=read(fd, data_to_send, BYTES))>0 ){
                		    	write (clients[n], data_to_send, bytes_read);
                		    }
                		}
                	}
                }
                else if (strcmp(path, "/home/cis505/git/http/signup.html") == 0){
                	//if username has a password associated, refresh signup page and say username taken
                	//else save the username locally, save user details with storage and take to welcome.
                	char* temp;

                	if((temp = strcasestr(store,"firstname=")) != NULL){
                		temp+=10;
                		//printf("\n\nHI%s\n\n", temp);
                		int count = 0;
                		char *temp1 = (char*)malloc (1 + strlen (temp));
                		strcpy(temp1, temp);
                		while (temp[0] != '&'){
                			count ++;
                			temp++;
                		}
                		//printf("\n\nHELLO%s%d\n\n", temp1, count);
                		fname = (char*)malloc(count);
                		strncpy(fname, temp1, count);
                		printf("\n%s\n", fname);
                	}

                	if((temp = strcasestr(store,"lastname=")) != NULL){
                		temp+=9;
                		//printf("\n\nHI%s\n\n", temp);
                		int count = 0;
                		char *temp1 = (char*)malloc (1 + strlen (temp));
                		strcpy(temp1, temp);
                		while (temp[0] != '&'){
                			count ++;
                			temp++;
                		}
                		//printf("\n\nHELLO%s%d\n\n", temp1, count);
                		lname = (char*)malloc(count);
                		strncpy(lname, temp1, count);
                		printf("\n%s\n", lname);
                	}

                	if((temp = strcasestr(store,"username=")) != NULL){
                		temp+=9;
                		//printf("\n\nHI%s\n\n", temp);
                		int count = 0;
                		char *temp1 = (char*)malloc (1 + strlen (temp));
                		strcpy(temp1, temp);
                		while (temp[0] != '&'){
                			count ++;
                			temp++;
                		}
                		//printf("\n\nHELLO%s%d\n\n", temp1, count);
                		uname = (char*)malloc(count);
                		strncpy(uname, temp1, count);
                		printf("\n%s\n", uname);
                	}

                	if((temp = strcasestr(store,"password=")) != NULL){
                		temp+=9;
                		//printf("\n\nHI%s\n\n", temp);
                		int count = 0;
                		char *temp1 = (char*)malloc (1 + strlen (temp));
                		strcpy(temp1, temp);
                		while (temp[0] != '&'){
                			count ++;
                			temp++;
                		}
                		//printf("\n\nHELLO%s%d\n\n", temp1, count);
                		pswd = (char*)malloc(count);
                		strncpy(pswd, temp1, count);
                		printf("\n%s\n", pswd);
                	}

                	if((temp = strcasestr(store,"cpassword=")) != NULL){
                		temp+=10;
                		cpswd = (char*)malloc(strlen(temp));
                		strcpy(cpswd, temp);
                		//uname[count + 1] = '\0';
                		printf("\n%s\n", cpswd);
                	}

                	if (strcmp(pswd, cpswd) == 0){
                		printf("\n\nPASSWORDS MATCH\n\n");
                		//send to storage
                		//if id doesnt exist, store data and log in
                    	//STUB
                    	bool auth = true;
                    	if (auth == true){
                    		reqline[1] = "/welcome.html";
                    		strcpy(path, ROOT);
                    		strcpy(&path[strlen(ROOT)], reqline[1]);
     //               		printf("file: %s\n", path);
                    		if ( (fd=open(path, O_RDONLY))!=-1 ){
                    			send(clients[n], "HTTP/1.1 200 OK\n\n", 17, 0);
                    			while ( (bytes_read=read(fd, data_to_send, BYTES))>0 ){
                    				write (clients[n], data_to_send, bytes_read);
                    			}
                    		}
                  		}
                	}

                	else{
                    	if ( (fd=open(path, O_RDONLY))!=-1 ){
                    		send(clients[n], "HTTP/1.1 200 OK\n\n", 17, 0);
                    		while ( (bytes_read=read(fd, data_to_send, BYTES))>0 ){
                    			write (clients[n], data_to_send, bytes_read);
                    		}
                    	}
                	}
                }
            }
        }
    }
    shutdown (clients[n], SHUT_RDWR);
    close(clients[n]);
    clients[n]=-1;
}
