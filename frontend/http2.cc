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
#include<iomanip>
#include<sstream>
#include<locale>
#include<string>
#include <SFML/Network.hpp>
#include"MasterConfig.h"
using namespace std;

#define CONNMAX 1000
#define BYTES 1024

char *ROOT;
int listenfd, clients[CONNMAX];
void startServer(char *);
void respond(int);
bool auth;
char *fname, *lname, *uname, *pswd, *cpswd;

int ports[10], freeport, totports;
bool status[10];
static bool port_is_open(const string& address, int port)
{
    return (sf::TcpSocket().connect(address, port) == sf::Socket::Done);
}
int main(int argc, char* argv[]){
    struct sockaddr_in clientaddr;
    socklen_t addrlen;
    char c;
    //Default Values PATH = ~/ and PORT=10000
    char PORT[6];
    ROOT = getenv("PWD");
    strcpy(PORT,"11000");

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

    MasterConfig mc = MasterConfig::fetchFromMaster("127.0.0.1:5000");
    mc.print();

    srand (time(NULL));
    int base = rand() % 90 + 1;
    //base = 11;
    base = base + 10010;
    for (int i = 0; i<10; i++){
    	ports[i] = base;
        string s = "./http4 -p ";
        string p = static_cast<ostringstream*>(&(ostringstream()<<ports[i]))->str();
        s.append(p);
        s.append(" &");
        cout<<s<<endl;
        char *c = (char*)s.c_str();
        std::system(c);
        base++;
     }
    totports = 10;
    for (int i = 0; i<10; i++){
    	status[i] = true;
    }

    while (1){
        addrlen = sizeof(clientaddr);
        clients[slot] = accept (listenfd, (struct sockaddr *) &clientaddr, &addrlen);

        if (clients[slot]<0){
            //error ("accept() error");
        }
        else{
            if ( fork()==0 ){
            	//cout<<"slot"<<slot<<endl;
            	for(int i = 0; i<10; i++){
            		if(port_is_open("127.0.0.1", ports[i]) == true){
            			status[i] = true;
            		}
            		else{
            			status[i] = false;
            			totports --;
            		}
            	}

            	freeport = slot%10;
            	if(totports < 10){
            		freeport += (10-totports);
            		while(!status[freeport]){
            			freeport += 1;
            			freeport %= 10;
            		}

            	}
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

                            strcpy(path, ROOT);
                            strcpy(&path[strlen(ROOT)], reqline[1]);
                            printf("file: %s\n", path);


            				if ((strstr(path, "admin") != NULL)){
            					string s, t;
            					t = "<h4>STORAGE SERVER STATUS</h4><p>";
            					for(int i = 0; i<mc.serversInfo.size(); i++){
            						//t.append("<li>");
            						t.append(mc.serversInfo[i].server.literal);
            						t.append(" ");
            						if(mc.serversInfo[i].alive == true){
            							t.append("Alive");
            						}
            						else{
            							t.append("Dead");
            						}
            						t.append("<br />");
            					}
            					t.append("</p>");

            					t.append("<h4>FRONT END SERVER STATUS</h4><p>");
            					for(int i = 0; i<10; i++){
            						t.append("127.0.0.1:");
            						t.append(to_string(ports[i]));
            						t.append(" ");
            						if(port_is_open("127.0.0.1", ports[i]) == true){
            							t.append("Alive");
            						}
            						else{
            							t.append("Dead");
            						}
            						t.append("<br />");
            					}
            					t.append("</p>");
            					char *c1 = (char*)t.c_str();
            					char *temp, *pno;
            					for(int i = 0; i<mc.serversInfo.size(); i++){
            						c1 = (char*)mc.serversInfo[i].server.literal.c_str();
                                	if((temp = strcasestr(c1,"127.0.0.1:")) != NULL){
                                		temp+=10;
                                		pno = temp;
                                	}
                                	int p = atoi(pno);
                                	cout<<"\nPort number: "<<p;
                                	int sock = socket(AF_INET, SOCK_STREAM, 0);
                                	struct sockaddr_in dest;
                                	dest.sin_family = AF_INET;
                					dest.sin_port = htons(p);
                					inet_pton(AF_INET, "127.0.0.1", &(dest.sin_addr));


                					int x = connect (sock, (struct sockaddr *) &dest, sizeof(dest));
                					cout<<"\n"<<x<<"\n";
                					cout<<"\nSending\n";
                					char buf[99999];


                                	write (sock, "yo\r\n", 4);

                                	int rlen = recv(sock, buf, 99999, 0);
                                	string s= buf;
                                	cout<<"\n"<<rlen<<"\n"<<s;
                                	s = "kval\r\n";
                                	char *c = (char*)s.c_str();
                					write (sock, c, s.size());
                					cout<<"\nwaiting to receive response\n";

                					char rbuffer[100];
                				    char cbuffer[100];
                				    bzero(rbuffer, sizeof(rbuffer)); bzero(cbuffer, sizeof(cbuffer));
                				    string command;
                				    t.append("<p>");
                				    // main loop to parse commands
                				    int d = read(sock, rbuffer, sizeof(rbuffer));
                				    while (d > 0) {
                				        s = rbuffer;
                				    	cout<<"rbuffer\n"<<s<<"\n";
                				    	char *found = strstr(rbuffer, "<EOB>");
                				        if (found) {
                				            int len = found - rbuffer;
                				            strncat(cbuffer, rbuffer, len);
                				            command = string(cbuffer);
                				           // logVerbose("[%d] FIRST: %s", sock, command.c_str());
                				            break;
                				        }
                				        else {
                				            strncat(cbuffer, rbuffer, strlen(rbuffer));
                				        }
                				        bzero(rbuffer, sizeof(rbuffer));
                				        d--;
                				    }

                	                cout<<"\nreceived response:";
                					cout<<"\n"<<command<<"\n";
                					t.append(command);
                					t.append("</p>");
            					}

            					c1 = (char*)t.c_str();
            					send(clients[slot], "HTTP/1.1 200 OK\n\n", 17, 0);

            					write (clients[slot], c1, t.size());


            				}
            				else{
            					string s = "<!DOCTYPE html><html><head><meta charset=\"utf-8\"><meta http-equiv=\"refresh\" content=\"0; url=http://127.0.0.1:";
            					string p = static_cast<ostringstream*>(&(ostringstream()<<ports[freeport]))->str();
            					s.append(p);
            					s.append("\" /></head></html>");

            					char *c = (char*)s.c_str();
            					send(clients[slot], "HTTP/1.1 200 OK\n\n", 17, 0);
            					write (clients[slot], c, s.size());

            				}
            			}
            		}
            	}

            	shutdown (clients[slot], SHUT_RDWR);
            	close(clients[slot]);
            	clients[slot]=-1;

            	//cout<<"freeport"<<freeport<<endl;
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
