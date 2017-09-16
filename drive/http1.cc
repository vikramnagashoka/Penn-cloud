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
void respond(int);
char *fname, *lname, *uname, *pswd, *cpswd;
string s, t;
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
            	char mesg[99999], *reqline[3], data_to_send[BYTES], path[99999];
                int rcvd, fd, bytes_read;

                memset( (void*)mesg, (int)'\0', 99999 );

                rcvd=recv(clients[slot], mesg, 99999, 0);

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
                            write(clients[slot], "HTTP/1.1 400 Bad Request\n", 25);
                        }
                        else{

                            if( strncmp(reqline[1], "/\0", 2)==0 ){
                            	reqline[1] = "/index.html";
                            }

                            strcpy(path, ROOT);
                            strcpy(&path[strlen(ROOT)], reqline[1]);
                            printf("file: %s\n", path);

                            if ((strstr(path, "/login.html") != NULL) || (strstr(path, "/signup.html") != NULL)){


                                if ( (fd=open(path, O_RDONLY))!=-1 ){
                            		send(clients[slot], "HTTP/1.1 200 OK\n\n", 17, 0);
                            		while ( (bytes_read=read(fd, data_to_send, BYTES))>0 ){
                            			write (clients[slot], data_to_send, bytes_read);
                            		}
                            	}
                            }

                            else if ((strstr(path, "/mail.html") != NULL)){
                            	cout<<"\n\nMAIL\n\n";


                            	char* temp;
                            	if ( (temp = strcasestr(store,"id=")) != NULL ){
                            		temp+=3;
                            		int count = 0;
                            		char *temp1 = (char*)malloc (1 + strlen (temp));
                            		strcpy(temp1, temp);
                            		while (temp[0] != '\n'){
                            			count ++;
                            			temp++;
                            		}
                            		uname = (char*)malloc(count-1);
                            		strncpy(uname, temp1, count-1);

                            		printf("\n%s", uname);
                            	}

                            	s = uname;
                            	s.append("\r\n");
                            	cout<<"\n"<<s<<"\n";
                            	char *c = (char*)s.c_str();
                            	int sock = socket(AF_INET, SOCK_STREAM, 0);
                            	struct sockaddr_in dest;
                            	socklen_t destlen = sizeof(dest);
                            	char buf[99999];
                            	dest.sin_family = AF_INET;
            					dest.sin_port = htons(6000);//port number of mail server
            					inet_pton(AF_INET, "127.0.0.1", &(dest.sin_addr));


            					int x = connect (sock, (struct sockaddr *) &dest, sizeof(dest));
            					cout<<"\n"<<x<<"\n";
            					cout<<"\nSending\n";

               					write (sock, c, s.size());
            					cout<<"\nwaiting to receive response\n";

            	                int rlen = recv(sock, buf, 99999, 0);
            					cout<<"\nreceived response\n";
            					buf[rlen] = '\0';
            					cout<<"\n"<<rlen<<"\n";
            					s = buf;
            					cout<<"\n"<<s<<"\n";

                            	if ( (fd=open(path, O_RDONLY))!=-1 ){
                            		send(clients[slot], "HTTP/1.1 200 OK\n\n", 17, 0);
                            		while ( (bytes_read=read(fd, data_to_send, BYTES))>0 ){
                            			write (clients[slot], data_to_send, bytes_read);
                            		}
                            	}
                            }

                            else if ((strstr(path, "/drive.html") != NULL)){

                            	//#Contents#+username=vikram+path=/root/abc/+#EndOfCommand


                            	char *temp, *depth;

                            	if ( (temp = strcasestr(store,"id=")) != NULL ){
                            		temp+=3;
                            		int count = 0;
                            		char *temp1 = (char*)malloc (1 + strlen (temp));
                            		strcpy(temp1, temp);
                            		while (temp[0] != ';' && temp[0] != '\n'){
                            			count ++;
                            			temp++;
                            		}
                            		uname = (char*)malloc(count);
                            		strncpy(uname, temp1, count);

                            		printf("\n%s", uname);
                            	}


                            	if ( (temp = strcasestr(store,"path=")) != NULL ){
                            		temp+=5;
                            		int count = 0;
                            		char *temp1 = (char*)malloc (1 + strlen (temp));
                            		strcpy(temp1, temp);
                            		while (temp[0] != '\n'){
                            			count ++;
                            			temp++;
                            		}
                            		depth = (char*)malloc(count-1);
                            		strncpy(depth, temp1, count-1);

                            		printf("\n%s", depth);
                            	}
                            	else{
                            		depth = "/root/";
                            		printf("\nElse: %s", depth);
                            	}

                            	s = "#Contents#+username=";
                            	s.append(uname);
                            	s.append("+path=");
                            	s.append(depth);
                            	s.append("+#EndOfCommand#");
                            	cout<<"\nSTRING: "<<s<<"\n";

                            	char *c = (char*)s.c_str();
                            	int sock = socket(AF_INET, SOCK_STREAM, 0);
                            	struct sockaddr_in dest;
                            	socklen_t destlen = sizeof(dest);
                            	char buf[99999];
                            	dest.sin_family = AF_INET;
            					dest.sin_port = htons(7000);
            					inet_pton(AF_INET, "127.0.0.1", &(dest.sin_addr));

            					int x = connect (sock, (struct sockaddr *) &dest, sizeof(dest));
            					cout<<"\n"<<x<<"\n";
            					cout<<"\nSending\n";
            					write (sock, c, s.size());

            					int rlen = recv(sock, buf, 99999, 0);
            					s=buf;
            					cout<<"\nReceived: "<<s<<"\n";


                            	if ( (fd=open(path, O_RDONLY))!=-1 ){
                            		send(clients[slot], "HTTP/1.1 200 OK\n", 16, 0);
                            		send(clients[slot], "set-cookie: path=/root/;\n\n", 26, 0);

                            		while ( (bytes_read=read(fd, data_to_send, BYTES))>0 ){
                            			write (clients[slot], data_to_send, bytes_read);
                            		}
                            	}
                            }


                            else if ((strstr(path, "/createfolder.html") != NULL)){

                            	if ( (fd=open(path, O_RDONLY))!=-1 ){
                            		send(clients[slot], "HTTP/1.1 200 OK\n\n", 17, 0);


                            		while ( (bytes_read=read(fd, data_to_send, BYTES))>0 ){
                            			write (clients[slot], data_to_send, bytes_read);
                            		}
                            	}
                            }


                            else if ((strstr(path, "/newmail.html") != NULL)){

/*                            	char* temp;
                            	if ( (temp = strcasestr(store,"id=")) != NULL ){
                            		temp+=3;
                            		uname = (char*)malloc(strlen(temp));
                            		strcpy(pswd, temp);
                            		printf("\n%s\n", uname);
                            	}
*/
                            	if ( (fd=open(path, O_RDONLY))!=-1 ){
                            		send(clients[slot], "HTTP/1.1 200 OK\n\n", 17, 0);
                            		while ( (bytes_read=read(fd, data_to_send, BYTES))>0 ){
                            			write (clients[slot], data_to_send, bytes_read);
                            		}
                            	}
                            }

                            else if ((strstr(path, "/inbox.html") != NULL)){

                            	char* temp;
                            	if ( (temp = strcasestr(store,"id=")) != NULL ){
                            		temp+=3;
                            		int count = 0;
                            		char *temp1 = (char*)malloc (1 + strlen (temp));
                            		strcpy(temp1, temp);
                            		while (temp[0] != '\n'){
                            			count ++;
                            			temp++;
                            		}
                            		uname = (char*)malloc(count-1);
                            		strncpy(uname, temp1, count-1);

                            		printf("\n%s", uname);
                            	}

                            	s = "GET_INBOX~";
                            	s.append(uname);
                            	s.append("\r\n");
                            	cout<<"\n"<<s<<"\n";
                            	char *c = (char*)s.c_str();
                            	int sock = socket(AF_INET, SOCK_STREAM, 0);
                            	struct sockaddr_in dest;
                            	socklen_t destlen = sizeof(dest);
                            	char buf[99999];
                            	dest.sin_family = AF_INET;
            					dest.sin_port = htons(6000);//port number of mail server
            					inet_pton(AF_INET, "127.0.0.1", &(dest.sin_addr));


            					int x = connect (sock, (struct sockaddr *) &dest, sizeof(dest));
            					cout<<"\n"<<x<<"\n";
            					cout<<"\nSending\n";

               					write (sock, c, s.size());
            					cout<<"\nwaiting to receive response\n";

            	                int rlen = recv(sock, buf, 99999, 0);
            					cout<<"\nreceived response\n";
            					buf[rlen] = '\0';
            					cout<<"\n"<<rlen<<"\n";
            					s = buf;
            					cout<<"\n"<<s<<"\n";

            					if ( (fd=open(path, O_RDONLY))!=-1 ){
                            		send(clients[slot], "HTTP/1.1 200 OK\n\n", 17, 0);
                            		while ( (bytes_read=read(fd, data_to_send, BYTES))>0 ){
                            			write (clients[slot], data_to_send, bytes_read);
                            		}
                            	}
                            }


                            else if ((strstr(path, "/sentbox.html") != NULL)){

                            	char* temp;
                            	if ( (temp = strcasestr(store,"id=")) != NULL ){
                            		temp+=3;
                            		int count = 0;
                            		char *temp1 = (char*)malloc (1 + strlen (temp));
                            		strcpy(temp1, temp);
                            		while (temp[0] != '\n'){
                            			count ++;
                            			temp++;
                            		}
                            		uname = (char*)malloc(count-1);
                            		strncpy(uname, temp1, count-1);

                            		printf("\n%s", uname);
                            	}

                            	s = "GET_SENT~";
                            	s.append(uname);
                            	s.append("\r\n");
                            	cout<<"\n"<<s<<"\n";
                            	char *c = (char*)s.c_str();
                            	int sock = socket(AF_INET, SOCK_STREAM, 0);
                            	struct sockaddr_in dest;
                            	socklen_t destlen = sizeof(dest);
                            	char buf[99999];
                            	dest.sin_family = AF_INET;
            					dest.sin_port = htons(6000);//port number of mail server
            					inet_pton(AF_INET, "127.0.0.1", &(dest.sin_addr));


            					int x = connect (sock, (struct sockaddr *) &dest, sizeof(dest));
            					cout<<"\n"<<x<<"\n";
            					cout<<"\nSending\n";

               					write (sock, c, s.size());
            					cout<<"\nwaiting to receive response\n";

            	                int rlen = recv(sock, buf, 99999, 0);
            					cout<<"\nreceived response\n";
            					buf[rlen] = '\0';
            					cout<<"\n"<<rlen<<"\n";
            					s = buf;
            					cout<<"\n"<<s<<"\n";

            					if ( (fd=open(path, O_RDONLY))!=-1 ){
                            		send(clients[slot], "HTTP/1.1 200 OK\n\n", 17, 0);
                            		while ( (bytes_read=read(fd, data_to_send, BYTES))>0 ){
                            			write (clients[slot], data_to_send, bytes_read);
                            		}
                            	}
                            }


                            else if ((strstr(path, "/fileupload.html") != NULL)){

            					if ( (fd=open(path, O_RDONLY))!=-1 ){
                            		send(clients[slot], "HTTP/1.1 200 OK\n\n", 17, 0);
                            		while ( (bytes_read=read(fd, data_to_send, BYTES))>0 ){
                            			write (clients[slot], data_to_send, bytes_read);
                            		}
                            	}
                            }

                            else{

                            	if ( (fd=open(path, O_RDONLY))!=-1 ){
                                	send(clients[slot], "HTTP/1.1 200 OK\n", 16, 0);
                           			send(clients[slot], "set-cookie: id=demo\n", 21, 0);
                           			while ( (bytes_read=read(fd, data_to_send, BYTES))>0 ){
                                		write (clients[slot], data_to_send, bytes_read);
                                	}
                            	}

                            	else{
                            		write(clients[slot], "HTTP/1.1 404 Not Found\n", 23);
                            	}
                            }
                        }
                    }

                    else if( strncmp(reqline[0], "POST\0", 5)==0 ){
                        //printf("HELLO");
                    	reqline[1] = strtok (NULL, " \t");
                    	reqline[2] = strtok (NULL, " \t\n");

                    	if ( strncmp( reqline[2], "HTTP/1.0", 8)!=0 && strncmp( reqline[2], "HTTP/1.1", 8)!=0 ){
                            write(clients[slot], "HTTP/1.1 400 Bad Request\n", 25);
                        }

                    	else{

                    		if ( strncmp(reqline[1], "/\0", 2)==0 ){
                            	reqline[1] = "/index.html";
                            }
                            strcpy(path, ROOT);
                            strcpy(&path[strlen(ROOT)], reqline[1]);
                            printf("\n\nfile: %s\n\n", path);

                            if (strstr(path, "/login.html") != NULL){
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

                            	s = "get ";
                            	s.append(uname);
                            	s.append(",password\r\n");
                            	char *c = (char*)s.c_str();
                            	int sock = socket(AF_INET, SOCK_STREAM, 0);
                            	struct sockaddr_in dest;
                            	socklen_t destlen = sizeof(dest);
                            	char buf[1000];
                            	dest.sin_family = AF_INET;
            					dest.sin_port = htons(5001);
            					inet_pton(AF_INET, "127.0.0.1", &(dest.sin_addr));


            					int x = connect (sock, (struct sockaddr *) &dest, sizeof(dest));
            					cout<<"\n"<<x<<"\n";
            					cout<<"\nSending\n";

            					write (sock, "yo\r\n", 4);
              					int rlen = recv(sock, buf, 99999, 0);

            					write (sock, c, s.size());
            					cout<<"\nwaiting to receive response\n";

            					char rbuffer[100];
            				    char cbuffer[100];
            				    bzero(rbuffer, sizeof(rbuffer)); bzero(cbuffer, sizeof(cbuffer));
            				    string command;

            				    // main loop to parse commands
            				    while (read(sock, rbuffer, sizeof(rbuffer)) > 0) {
            				        s = rbuffer;
            				    	cout<<"rbuffer\n"<<s<<"\n";
            				    	char *found = strstr(rbuffer, "<EOB>");
            				        if (found) {
            				            int len = found - rbuffer;
            				            strncat(cbuffer, rbuffer, len)