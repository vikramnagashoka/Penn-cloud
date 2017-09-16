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
#include<fstream>
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
string ReplaceString(string subject, string search,
                          string replace) {
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != string::npos) {
         subject.replace(pos, search.length(), replace);
         pos += replace.length();
    }
    return subject;
}

int count_depth(string s) {
  int count = 0;

  for (int i = 0; i < s.size(); i++)
    if (s[i] == '/') count++;

  return count;
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

	char buf[99999];

	int storagesock = socket(PF_INET, SOCK_DGRAM, 0);
	struct sockaddr_in storage;
	storage.sin_family = PF_INET;
	storage.sin_port = htons(5000);
	inet_pton(PF_INET, "127.0.0.1", &(storage.sin_addr));

	int mailsock = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in mail;
	mail.sin_family = AF_INET;
	mail.sin_port = htons(6000);//port number of mail server
	inet_pton(AF_INET, "127.0.0.1", &(mail.sin_addr));
	int mconnect = connect (mailsock, (struct sockaddr *) &mail, sizeof(mail));

	int drivesock = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in drive;
	drive.sin_family = AF_INET;
	drive.sin_port = htons(7000); //port number of drive server
	inet_pton(AF_INET, "127.0.0.1", &(drive.sin_addr));
	int dconnect = connect (drivesock, (struct sockaddr *) &drive, sizeof(drive));



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

                            		printf("\nusername%s", uname);
                            	}

                            	s = uname;
                            	s.append("\r\n");
                            	cout<<"\n"<<s<<"\n";
                            	char *c = (char*)s.c_str();
               					write (mailsock, c, s.size());
            					cout<<"\nwaiting to receive response\n";

            	                int rlen = recv(mailsock, buf, 99999, 0);
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

                            else if ((strstr(path, "/createfolder.html") != NULL)){

                            	t = "<!DOCTYPE html><html><body><form method=\"post\">Folder Name:<br><input type=\"text\" name=\"foldername\" value=\"\"><br><input type=\"submit\" value=\"Submit\"></form> </body></html>";
                            	char *c = (char*)t.c_str();
                            	send(clients[slot], "HTTP/1.1 200 OK\n\n", 17, 0);
                            	write (clients[slot], c, t.size());
                            }


                            else if ((strstr(path, "movefile") != NULL)){

                                s = "<!DOCTYPE html><html><meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" /><form method=\"post\">New Location:<br><input type=\"text\" name=\"newpath\" value=\"\"><br><input type=\"submit\" value=\"Submit\"></form>";

                            	char *c = (char*)s.c_str();
                            	send(clients[slot], "HTTP/1.1 200 OK\n\n", 17, 0);
                            	write (clients[slot], c, s.size());
                            }


                            else if ((strstr(path, "movefolder") != NULL)){

                                s = "<!DOCTYPE html><html><meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" /><form method=\"post\">New Location:<br><input type=\"text\" name=\"newpath\" value=\"\"><br><input type=\"submit\" value=\"Submit\"></form>";

                            	char *c = (char*)s.c_str();
                            	send(clients[slot], "HTTP/1.1 200 OK\n\n", 17, 0);
                            	write (clients[slot], c, s.size());
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

                            	write (mailsock, c, s.size());
            					cout<<"\nwaiting to receive response\n";

            	                int rlen = recv(mailsock, buf, 99999, 0);
            					cout<<"\nreceived response\n";
            					buf[rlen] = '\0';
            					cout<<"\n"<<rlen<<"\n";
            					s = buf;
            					cout<<"\nInbox:"<<s<<"end\n";

            					send(clients[slot], "HTTP/1.1 200 OK\n\n", 17, 0);
            					t = "<!DOCTYPE html><html><body><h1>INBOX</h1><a href=\"welcome.html\">Home</a><a href=\"mail.html\">Mailbox</a><a href=\"index.html\">Logout</a><br />";
            					c = (char*)t.c_str();
                            	write (clients[slot], c, t.size());

            					string delimiter = "\r\n";
            					size_t pos = 0;
            					string token;
            					while ((pos = s.find(delimiter)) != std::string::npos) {
            					    token = s.substr(0, pos);
            					    cout << token << endl;
            					    t = "<a href=\"/viewmail/";
            					    t.append(token);
            					    t.append(".html\">");
            					    t.append(token);
            					    t.append("</a><br />");
            					    c = (char*)t.c_str();
            					    write (clients[slot], c, t.size());
            					    s.erase(0, pos + delimiter.length());
            					}
            					t = "</body></html>";
            					c = (char*)t.c_str();
            					write (clients[slot], c, t.size());

                            }


                            else if ((strstr(path, "viewmail") != NULL)){

                            	char *temp, *head;

                            	if ( (temp = strcasestr(path,"viewmail/")) != NULL ){
                            		temp+=9;
                            		int count = 0;
                            		char *temp1 = (char*)malloc (1 + strlen (temp));
                            		strcpy(temp1, temp);
                            		while (temp[0] != '.'){
                            			count ++;
                            			temp++;
                            		}
                            		head = (char*)malloc(count);
                            		strncpy(head, temp1, count);

                            		printf("\n%s", head);
                            	}


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

                            	s = "GET_EMAIL~";
                            	s.append(uname);
                            	s.append("~");
                            	s.append(head);
                            	s.append("\r\n");
                            	cout<<"\n"<<s<<"\n";
                            	char *c = (char*)s.c_str();

                            	write (mailsock, c, s.size());
            					cout<<"\nwaiting to receive response\n";

            	                int rlen = recv(mailsock, buf, 99999, 0);
            					cout<<"\nreceived response\n";
            					buf[rlen] = '\0';
            					cout<<"\n"<<rlen<<"\n";
            					s = buf;
            					cout<<"\nMail:"<<s<<"end\n";

            					send(clients[slot], "HTTP/1.1 200 OK\n\n", 17, 0);
            					t = "<!DOCTYPE html><html><body><a href=\"../welcome.html\">Home</a><a href=\"../mail.html\">Mailbox</a><a href=\"../index.html\">Logout</a><br />";
            					t.append("<a href=\"../inbox.html\">Inbox</a><a href=\"../sentbox.html\">Sentbox</a>");
            					c = (char*)t.c_str();
                            	write (clients[slot], c, t.size());

            					string delimiter = "~";
            					int flag = 0;
            					size_t pos = 0;
            					string token;
            					while ((pos = s.find(delimiter)) != std::string::npos) {
            						flag++;
            						token = s.substr(0, pos);
            					    cout << token << endl;
            					    if (flag == 1){
            					    	t = "<p>Sender: ";
            					    }
            					    else{
            					    	t = "<p>Receiver: ";
            					    }

            					    t.append(token);
            					    t.append("</p>");
            					    c = (char*)t.c_str();
            					    write (clients[slot], c, t.size());
            					    s.erase(0, pos + delimiter.length());

            					}

            					t = "<p>Subject: ";
            					t.append(head);
            					t.append("</p>");

            					t.append("<p>Body: ");
            					t.append(s);
            					t.append("</p>");

            					c = (char*)t.c_str();
            					write (clients[slot], c, t.size());

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

                            	write (mailsock, c, s.size());
            					cout<<"\nwaiting to receive response\n";

            	                int rlen = recv(mailsock, buf, 99999, 0);
            					cout<<"\nreceived response\n";
            					buf[rlen] = '\0';
            					cout<<"\n"<<rlen<<"\n";
            					s = buf;
            					cout<<"\nSentbox:"<<s<<"end\n";

            					send(clients[slot], "HTTP/1.1 200 OK\n\n", 17, 0);
            					t = "<!DOCTYPE html><html><body><h1>SENT BOX</h1><a href=\"welcome.html\">Home</a><a href=\"mail.html\">Mailbox</a><a href=\"index.html\">Logout</a><br />";
            					c = (char*)t.c_str();
                            	write (clients[slot], c, t.size());

            					string delimiter = "\r\n";
            					size_t pos = 0;
            					string token;
            					while ((pos = s.find(delimiter)) != std::string::npos) {
            					    token = s.substr(0, pos);
            					    cout << token << endl;
            					    t = "<a href=\"/viewmail/";
            					    t.append(token);
            					    t.append(".html\">");
            					    t.append(token);
            					    t.append("</a><br />");
            					    c = (char*)t.c_str();
            					    write (clients[slot], c, t.size());
            					    s.erase(0, pos + delimiter.length());
            					}
            					t = "</body></html>";
            					c = (char*)t.c_str();
            					write (clients[slot], c, t.size());

                            }

                            else if ((strstr(path, "/fileupload.html") != NULL)){
                            	t = "<html><body><form method=\"post\"><p>File: <input type=\"file\" name=\"userfile\" /></p><p><input type=\"submit\" value=\"Upload\" /></p></form></body></html>";
                            	char *c = (char*)t.c_str();
                            	send(clients[slot], "HTTP/1.1 200 OK\n\n", 17, 0);
                            	write (clients[slot], c, t.size());
            				}

                            else if ((strstr(path, "/welcome.html") != NULL)){

            					if ( (fd=open(path, O_RDONLY))!=-1 ){
                            		send(clients[slot], "HTTP/1.1 200 OK\n\n", 17, 0);
                            		while ( (bytes_read=read(fd, data_to_send, BYTES))>0 ){
                            			write (clients[slot], data_to_send, bytes_read);
                            		}
                            	}
                            }


                            else if ((strstr(path, "downloadfile") != NULL)){

                            	char *temp, *depth, *flname;
                            	int loc = 0;

                            	if ( (temp = strcasestr(path,"downloadfile")) != NULL ){
                            		temp+=12;
                            		s = temp;
                            		cout<<"\nTEMP:"<<s<<"end\n";
                            		for (int i = 0; i<s.size(); i++){
                            			if(temp[i] == '/'){
                            				loc = i+1;
                            			}
                            		}
                            		cout<<"\nLOC:"<<loc<<"end\n";
                            		depth = (char*)malloc(loc);
                            		strncpy(depth, temp, loc);

                            		printf("\ndepth:%send", depth);
                            	}

                            	string u = depth;
                            	temp += u.size();
                            	u = temp;
                            	flname = (char*)u.c_str();
                            	cout<<"FILENAME:"<<flname<<"end\n";

                            	if ( (temp = strcasestr(store,"id=")) != NULL ){
                            		temp+=3;
                            		int count = 0;
                            		char *temp1 = (char*)malloc (1 + strlen (temp));
                            		strcpy(temp1, temp);
                            		while (temp[0] != '\r' && temp[0] != '\n'){
                            			count ++;
                            			temp++;
                            		}
                            		uname = (char*)malloc(count);
                            		strncpy(uname, temp1, count);

                            		printf("\nuname%send", uname);
                            	}
                            	//#Download#+username=vikram+path=/root/abc/+filename=pqr.txt+#EndOfCommand#
                            	s = "#Download#+username=";
                            	s.append(uname);
                            	s.append("+path=");
                            	s.append(depth);
                            	s.append("+filename=");
                            	s.append(flname);
                            	s.append("+#EndOfCommand#");
                            	cout<<"\nSTRING: "<<s<<"\n";
                            	char *c = (char*)s.c_str();
                            	write (drivesock, c, s.size());

                            	cout<<"\nSENT\n";

                            	int bytesReceived = 0;
                            	char recvBuff[10000];
                            	memset(recvBuff, '0', sizeof(recvBuff));

                            	FILE *fp;
                            	fp = fopen(flname, "ab+");
                            	if(NULL == fp)
                            	{
                            		printf("Error opening file");
                            		return 1;
                            	}
                            	else{
                            		cout<<"\nFile opened\n";
                            	}
				int zeroFound = string::npos;
				int eoFound = string::npos;
                            	while((bytesReceived = read(drivesock, recvBuff, 10000)) > 0){
                            	        printf("Bytes received %d\n",bytesReceived);
                            	        s = string(recvBuff);

                            	        printf("%s \n", recvBuff);
                            	        if (s.find("<EOF>") != string::npos) {
                            	        	cout << "found!" << '\n';
                            	        	int eofFound = s.find("<EOF>");
                            	        	//recvBuff[eofFound]='\0';
                            	        }
                            	        fwrite(recvBuff, 1,bytesReceived,fp);
					zeroFound = string(recvBuff).find("000000");
					
					if(zeroFound != string::npos){
						cout<<"get out"<<endl;
						break;	
					}
                            	}

                            	if(bytesReceived < 0){
                            		printf("\n Read Error \n");
                            	}

                            	t = "<!DOCTYPE html><html><body><h1>";
                            	t.append(flname);
                            	t.append(" downloaded successfully!</h1> <a href=\"/viewfolder");
                            	t.append(depth);
                            	t.append("\">Back</a></body></html>");
                            	char *c1 = (char*)t.c_str();
                            	//cout<<"\n"<<t;
                            	send(clients[slot], "HTTP/1.1 200 OK\n\n", 17, 0);
                            	write (clients[slot], c1, t.size());


                            }

                            else if ((strstr(path, "viewfolder") != NULL)){

                            	char *temp, *depth;

                            	if ( (temp = strcasestr(path,"viewfolder")) != NULL ){
                            		temp+=10;
                            		depth = temp;

                            		printf("\ndepth:%send", depth);
                            	}
                            	string u = depth;
                            	int v = count_depth(u);
                            	u = "../";
                            	//v = v-2;

                            	if ( (temp = strcasestr(store,"id=")) != NULL ){
                            		temp+=3;
                            		int count = 0;
                            		char *temp1 = (char*)malloc (1 + strlen (temp));
                            		strcpy(temp1, temp);
                            		while (temp[0] != '\r' && temp[0] != '\n'){
                            			count ++;
                            			temp++;
                            		}
                            		uname = (char*)malloc(count);
                            		strncpy(uname, temp1, count);

                            		printf("\nuname%send", uname);
                            	}

                            	s = "#Contents#+username=";
                            	s.append(uname);
                            	s.append("+path=");
                            	s.append(depth);
                            	s.append("+#EndOfCommand#");
                            	cout<<"\nSTRING: "<<s<<"\n";
                            	char *c = (char*)s.c_str();
                            	write (drivesock, c, s.size());
            					cout<<"\nSENT\n";
                            	int rlen = recv(drivesock, buf, 100, 0);
            					s=buf;
            					cout<<"\nReceived: "<<s<<"\n";

            					t = "#Folder#";
            					t.append(depth);
            					c = (char*)t.c_str();

                            	if ( (temp = strcasestr(buf, c)) != NULL ){
                            		temp+=t.size();
                            		printf("\nAfter stripping folder: %s", temp);
                            	}
                            	s = temp;
                            	s.erase(s.length()-4);

            					send(clients[slot], "HTTP/1.1 200 OK\n\n", 17, 0);

                        		t = "<!DOCTYPE html><html><body><h1>DRIVE</h1><a href=\"";

                        		for (int i = 0; i<v; i++){
                        			t.append(u);
                        		}
                        		t.append("welcome.html\">Home</a><a href=\"");

                        		for (int i = 0; i<v; i++){
                        			t.append(u);
                        		}
                        		t.append("index.html\">Logout</a><br />");

                        		t.append("<br /><a href=\"fileupload.html\">Upload</a><a href=\"createfolder.html\">Create Folder</a><br />");
            					c = (char*)t.c_str();
                            	write (clients[slot], c, t.size());

            					string delimiter = "#";
            					size_t pos = 0;
            					string token;
            					while ((pos = s.find(delimiter)) != std::string::npos) {
            					    token = s.substr(0, pos);
            					    cout << token << endl;
            					    char *d = (char*)token.c_str();

            					    if(strcasestr(d, "/") != NULL){
                					    t = "<a href=\"/viewfolder";
                					    t.append(depth);
                					    t.append(token);
                					    t.append("\">");
                					    t.append(token);
                					    t.append("</a>&nbsp;<a href=\"/deletefolder");
                					    t.append(depth);
                					    t.append(token);
                					    t.append("\">DELETE</a>&nbsp;<a href=\"/movefolder");
                					    t.append(depth);
                					    t.append(token);
                					    t.append("\">MOVE</a>");
                					    t.append("<br />");
            					    }
            					    else{
                					    t = "<a href=\"/downloadfile";
                					    t.append(depth);
                					    t.append(token);
                					    t.append("\">");
                					    t.append(token);
                					    t.append("</a>&nbsp;<a href=\"/deletefile");
                					    t.append(depth);
                					    t.append(token);
                					    t.append("\">DELETE</a>&nbsp;<a href=\"/movefile");
                					    t.append(depth);
                					    t.append(token);
                					    t.append("\">MOVE</a>");
                					    t.append("<br />");
            					    }
            					    c = (char*)t.c_str();
            					    write (clients[slot], c, t.size());
            					    s.erase(0, pos + delimiter.length());
            					}
            					t = "</body></html>";
            					c = (char*)t.c_str();
            					write (clients[slot], c, t.size());

                            }

                            else if ((strstr(path, "deletefolder") != NULL)){

                            	char *temp, *depth, *flname;
                            	int loc = 0;

                            	if ( (temp = strcasestr(path,"deletefolder")) != NULL ){
                            		temp+=12;
                            		s = temp;
                            		for (int i = 0; i<s.size()-1; i++){
                            			if(temp[i] == '/'){
                            				loc = i+1;
                            			}
                            		}
                            		depth = (char*)malloc(loc);
                            		strncpy(depth, temp, loc);

                            		printf("\ndepth:%send", depth);
                            	}
                            	string u = temp;
                            	string v = u.substr(loc, s.size());
                            	cout<<"\nFoldername:"<<v;
                            	flname = (char*)v.c_str();

                            	if ( (temp = strcasestr(store,"id=")) != NULL ){
                            		temp+=3;
                            		int count = 0;
                            		char *temp1 = (char*)malloc (1 + strlen (temp));
                            		strcpy(temp1, temp);
                            		while (temp[0] != '\r' && temp[0] != '\n'){
                            			count ++;
                            			temp++;
                            		}
                            		uname = (char*)malloc(count);
                            		strncpy(uname, temp1, count);

                            		printf("\nuname%send", uname);
                            	}
                            	//#DeleteFolder#+username=vikram+path=/root/abc/+foldername=lmn/+#EndOfCommand#
                            	s = "#DeleteFolder#+username=";
                            	s.append(uname);
                            	s.append("+path=");
                            	s.append(depth);
                            	s.append("+foldername=");
                            	s.append(flname);
                            	s.append("+#EndOfCommand#");
                            	cout<<"\nSTRING: "<<s<<"\n";
                            	char *c = (char*)s.c_str();
                            	write (drivesock, c, s.size());
            					cout<<"\nSENT\n";
                            	int rlen = recv(drivesock, buf, 99999, 0);
            					s=buf;
            					cout<<"\nReceived: "<<s<<"\n";

                            	t = "<!DOCTYPE html><html><body><h1>";
                            	t.append(flname);
                            	t.append(" deleted successfully!</h1> <a href=\"/viewfolder");
                            	t.append(depth);
                            	t.append("\">Back</a></body></html>");
                            	char *c1 = (char*)t.c_str();
                            	//cout<<"\n"<<t;
                            	send(clients[slot], "HTTP/1.1 200 OK\n\n", 17, 0);
                            	write (clients[slot], c1, t.size());



                            }


                            else if ((strstr(path, "deletefile") != NULL)){

                            	char *temp, *depth, *flname;
                            	int loc = 0;

                            	if ( (temp = strcasestr(path,"deletefile")) != NULL ){
                            		temp+=10;
                            		s = temp;
                            		for (int i = 0; i<s.size(); i++){
                            			if(temp[i] == '/'){
                            				loc = i+1;
                            			}
                            		}
                            		depth = (char*)malloc(loc);
                            		strncpy(depth, temp, loc);

                            		printf("\ndepth:%send", depth);
                            	}
                            	string u = temp;
                            	string v = u.substr(loc, s.size());
                            	cout<<"\nFilename:"<<v;
                            	flname = (char*)v.c_str();

                            	if ( (temp = strcasestr(store,"id=")) != NULL ){
                            		temp+=3;
                            		int count = 0;
                            		char *temp1 = (char*)malloc (1 + strlen (temp));
                            		strcpy(temp1, temp);
                            		while (temp[0] != '\r' && temp[0] != '\n'){
                            			count ++;
                            			temp++;
                            		}
                            		uname = (char*)malloc(count);
                            		strncpy(uname, temp1, count);

                            		printf("\nuname%send", uname);
                            	}
                            	//#DeleteFile#+username=vikram+path=root/movies/hindi/+filename=list.txt +#EndOfCommand#
                            	s = "#DeleteFile#+username=";
                            	s.append(uname);
                            	s.append("+path=");
                            	s.append(depth);
                            	s.append("+filename=");
                            	s.append(flname);
                            	s.append("+#EndOfCommand#");
                            	cout<<"\nSTRING: "<<s<<"\n";
                            	char *c = (char*)s.c_str();
                            	write (drivesock, c, s.size());
            					cout<<"\nSENT\n";
                            	int rlen = recv(drivesock, buf, 99999, 0);
            					s=buf;
            					cout<<"\nReceived: "<<s<<"\n";

                            	t = "<!DOCTYPE html><html><body><h1>";
                            	t.append(flname);
                            	t.append(" deleted successfully!</h1> <a href=\"/viewfolder");
                            	t.append(depth);
                            	t.append("\">Back</a></body></html>");
                            	char *c1 = (char*)t.c_str();
                            	//cout<<"\n"<<t;
                            	send(clients[slot], "HTTP/1.1 200 OK\n\n", 17, 0);
                            	write (clients[slot], c1, t.size());



                            }

                            else{
/*
                            	char* temp;
                            	if ( (temp = strcasestr(store,"id=")) != NULL ){
                            		cout<<"\nID exists\n";
                            		temp+=3;
                            		int count = 0;
                            		char *temp1 = (char*)malloc (1 + strlen (temp));
                            		strcpy(temp1, temp);
                            		while (temp[0] != '\r' && temp[0] != ';'){
                            			count ++;
                            			temp++;
                            		}
                            		uname = (char*)malloc(count);
                            		strncpy(uname, temp1, count);

                            		printf("\nUsername%send", uname);
                            		if(strcmp(uname, "demo") != 0){
                            			cout<<"\nID is not demo\n";
                                		s = "QUIT~";
                                    	s.append(uname);
                                    	s.append("\r\n");
                                    	cout<<"\n"<<s<<"\n";
                                    	char *c = (char*)s.c_str();

                                    	write (mailsock, c, s.size());
                    					cout<<"\nwaiting to receive response\n";

                    	                int rlen = recv(mailsock, buf, 99999, 0);
                    					cout<<"\nreceived response\n";
                    					cout<<"\n"<<rlen<<"\n";
                    					s = buf;
                    					cout<<"\nQuit message:"<<s<<"end\n";

                            		}
                            		else{
                            			cout<<"\nID is demo so quit not sent\n";
                            		}
                            	}
*/
                            	if ( (fd=open(path, O_RDONLY))!=-1 ){
                                	send(clients[slot], "HTTP/1.1 200 OK\n", 16, 0);
                                	send(clients[slot], "set-cookie: id=demo\n\n", 21, 0);
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

                            	s = "query ";
                            	s.append(uname);
                            	s.append("\r\n");
                            	char *c = (char*)s.c_str();
            					sendto(storagesock, c, s.size(), 0, (struct sockaddr*)&storage, sizeof(storage));
            					cout<<"\n"<<storagesock<<"\n"<<s<<"\n";

            					int rlen = recv(storagesock, buf, 99999, 0);

            					char *pno;
            					if ( (temp = strcasestr(buf,":")) != NULL ){
            						temp+=1;
            					    int count = 0;
            					    char *temp1 = (char*)malloc (1 + strlen (temp));
            					    strcpy(temp1, temp);
            					    while (temp[0] != '\n'){
            					    	count ++;
            					        temp++;
            					    }
            					    pno = (char*)malloc(count);
            					    strncpy(pno, temp1, count);
            					    printf("\n%s\n", pno);
            					}

            					int p = atoi(pno);
            					cout<<"\n"<<p<<"\n";

            					s = "get ";
                            	s.append(uname);
                            	s.append(",password\r\n");
                            	c = (char*)s.c_str();

                            	int sock = socket(AF_INET, SOCK_STREAM, 0);
                            	struct sockaddr_in dest;
                            	dest.sin_family = AF_INET;
            					dest.sin_port = htons(p);
            					inet_pton(AF_INET, "127.0.0.1", &(dest.sin_addr));


            					int x = connect (sock, (struct sockaddr *) &dest, sizeof(dest));
            					cout<<"\n"<<x<<"\n";
            					cout<<"\nSending\n";



                            	write (sock, "yo\r\n", 4);

                            	rlen = recv(sock, buf, 99999, 0);

            					write (sock, c, s.size());
            					cout<<"\nwaiting to receive response\n";

            					char rbuffer[100];
            				    char cbuffer[100];
            				    bzero(rbuffer, sizeof(rbuffer)); bzero(cbuffer, sizeof(cbuffer));
            				    string command;

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

            	                cout<<"\nreceived response\n";

            					cout<<"\n"<<command<<"\n";

            					bool auth;
            					s = command;
            					if (s.find("-ERR") != std::string::npos) {
            					    auth = false;
            					    cout<<"\nUser does not exist\n";
            					}
            					else{
            						cout<<"\n"<<s<<"\n";

            						if(s.compare(pswd)){
            							auth = false;
            							cout<<"\nPassword does not match records\n";
            						}
            						else{
            							auth = true;
            						}
            					}

                            	if (auth == true){

                            	    s = "set-cookie: id=";
                                    s.append(uname);
                                    s.append("\n\n");
                                    char *c = (char*)s.c_str();

                                    send(clients[slot], "HTTP/1.1 200 OK\n", 16, 0);
                                    send(clients[slot], c, s.size(), 0);


                                	s = "get ";
                                	s.append(uname);
                                	s.append(",firstname\r\n");
                                	c = (char*)s.c_str();

                					write (sock, "yo\r\n", 4);
                  					int rlen = recv(sock, buf, 99999, 0);

                					write (sock, c, s.size());

                					char rbuffer[100];
                				    char cbuffer[100];
                				    bzero(rbuffer, sizeof(rbuffer)); bzero(cbuffer, sizeof(cbuffer));
                				    string command;

                				    while (read(sock, rbuffer, sizeof(rbuffer)) > 0) {
                				        s = rbuffer;
                				    	cout<<"rbuffer\n"<<s<<"\n";
                				    	char *found = strstr(rbuffer, "<EOB>");
                				        if (found) {
                				            int len = found - rbuffer;
                				            strncat(cbuffer, rbuffer, len);
                				            command = string(cbuffer);
                				            break;
                				        } else {
                				            strncat(cbuffer, rbuffer, strlen(rbuffer));
                				        }
                				        bzero(rbuffer, sizeof(rbuffer));
                				    }

                					cout<<"\nreceived response\n";
                					cout<<"\n"<<command<<"\n";

                					t = "<!DOCTYPE html><html><body><h1>Hello ";
                                    t.append(command);
                                    t.append("</h1><a href=\"/viewfolder/root/\">Drive</a><a href=\"mail.html\">Mail</a></body></html>");

                                    char *c1 = (char*)t.c_str();
                                    write (clients[slot], c1, t.size());

                          		}
                            	else{
                            		if ( (fd=open(path, O_RDONLY))!=-1 ){
                            			send(clients[slot], "HTTP/1.1 200 OK\n\n", 17, 0);
                            		    while ( (bytes_read=read(fd, data_to_send, BYTES))>0 ){
                            		    	write (clients[slot], data_to_send, bytes_read);
                            		    }
                            		}
                            	}
                            }

                            else if (strstr(path, "/signup.html") != NULL){
                            	//if username has a password associated, refresh signup page and say username taken
                            	//else save the username locally, save user details with storage and take to welcome.
                            	char* temp;

                            	if((temp = strcasestr(store,"firstname=")) != NULL){
                            		temp+=10;
                            		int count = 0;
                            		char *temp1 = (char*)malloc (1 + strlen (temp));
                            		strcpy(temp1, temp);
                            		while (temp[0] != '&'){
                            			count ++;
                            			temp++;
                            		}
                            		fname = (char*)malloc(count);
                            		strncpy(fname, temp1, count);
                            		printf("\n%s\n", fname);
                            	}

                            	if((temp = strcasestr(store,"lastname=")) != NULL){
                            		temp+=9;
                            		int count = 0;
                            		char *temp1 = (char*)malloc (1 + strlen (temp));
                            		strcpy(temp1, temp);
                            		while (temp[0] != '&'){
                            			count ++;
                            			temp++;
                            		}
                            		lname = (char*)malloc(count);
                            		strncpy(lname, temp1, count);
                            		printf("\n%s\n", lname);
                            	}

                            	if((temp = strcasestr(store,"username=")) != NULL){
                            		temp+=9;
                            		int count = 0;
                            		char *temp1 = (char*)malloc (1 + strlen (temp));
                            		strcpy(temp1, temp);
                            		while (temp[0] != '&'){
                            			count ++;
                            			temp++;
                            		}
                            		uname = (char*)malloc(count);
                            		strncpy(uname, temp1, count);
                            		printf("\n%s\n", uname);
                            	}

                            	if((temp = strcasestr(store,"password=")) != NULL){
                            		temp+=9;

                            		int count = 0;
                            		char *temp1 = (char*)malloc (1 + strlen (temp));
                            		strcpy(temp1, temp);
                            		while (temp[0] != '&'){
                            			count ++;
                            			temp++;
                            		}
                            		pswd = (char*)malloc(count);
                            		strncpy(pswd, temp1, count);
                            		printf("\n%s\n", pswd);

                            	}

                            	if((temp = strcasestr(store,"cpassword=")) != NULL){
                            		temp+=10;
                            		cpswd = (char*)malloc(strlen(temp));
                            		strcpy(cpswd, temp);
                            		printf("\n%s\n", cpswd);
                            	}

                            	if (strcmp(pswd, cpswd) == 0){
                            		printf("\n\nPASSWORDS MATCH\n\n");
                            		//send to storage
                            		//if id doesnt exist, store data and log in

                                	s = "query ";
                                	s.append(uname);
                                	s.append("\r\n");
                                	char *c = (char*)s.c_str();
                					sendto(storagesock, c, s.size(), 0, (struct sockaddr*)&storage, sizeof(storage));
                					cout<<"\n"<<storagesock<<"\n"<<s<<"\n";

                					int rlen = recv(storagesock, buf, 99999, 0);

                					char *pno;
                					if ( (temp = strcasestr(buf,":")) != NULL ){
                						temp+=1;
                					    int count = 0;
                					    char *temp1 = (char*)malloc (1 + strlen (temp));
                					    strcpy(temp1, temp);
                					    while (temp[0] != '\n'){
                					    	count ++;
                					        temp++;
                					    }
                					    pno = (char*)malloc(count);
                					    strncpy(pno, temp1, count);
                					    printf("\n%s\n", pno);
                					}

                					int p = atoi(pno);
                					cout<<"\n"<<p<<"\n";

                					s = "get ";
                                	s.append(uname);
                                	s.append(",password\r\n");
                                	c = (char*)s.c_str();

                                	int sock = socket(AF_INET, SOCK_STREAM, 0);
                                	struct sockaddr_in dest;
                                	dest.sin_family = AF_INET;
                					dest.sin_port = htons(p);
                					inet_pton(AF_INET, "127.0.0.1", &(dest.sin_addr));
\
                					int x = connect (sock, (struct sockaddr *) &dest, sizeof(dest));
                					cout<<"\n"<<x<<"\n";
                					cout<<"\nSending\n";

                                	write (sock, "yo\r\n", 4);
                                	rlen = recv(sock, buf, 99999, 0);
                					write (sock, c, s.size());
                					cout<<"\nwaiting to receive response\n";
                	                rlen = recv(sock, buf, 99999, 0);
                					cout<<"\nreceived response\n";
                					buf[rlen] = '\0';
                					cout<<"\n"<<rlen<<"\n";

                					bool auth;
                					s = buf;
                					if (s.find("-ERR") != std::string::npos) {
                					    auth = true;
                					}
                					else{
                						auth = false;
                						cout<<"\nUser already exists\n";
                					}

                            		//STUB
                                	if (auth == true){

                                    	s = "put ";
                                    	t = fname;
                                    	s.append(uname);
                                    	s.append(",firstname ");
                                    	s.append(to_string(t.size()));
                                    	s.append(" 1");
                                    	s.append("\r\n");
                                    	c = (char*)s.c_str();
                    					cout<<"\nSending\n";
                    					write (sock, c, s.size());
                    					cout<<"\nwaiting to receive response\n";

                    	                rlen = recv(sock, buf, 99999, 0);
                    					cout<<"\nreceived response\n";
                    					buf[rlen] = '\0';
                    					cout<<"\n"<<rlen<<"\n";
                    					write (sock, fname, t.size());
        //wait for confirmation
                    					rlen = recv(sock, buf, 99999, 0);
                    					cout<<"\nreceived response\n";
                    					cout<<"\n"<<rlen<<"\n";

                                    	s = "put ";
                                    	t = lname;
                                    	s.append(uname);
                                    	s.append(",lastname ");
                                    	s.append(to_string(t.size()));
                                    	s.append(" 2");
                                    	s.append("\r\n");
                                    	c = (char*)s.c_str();

                                    	write (sock, c, s.size());
                    					cout<<"\nwaiting to receive response\n";

                    	                rlen = recv(sock, buf, 99999, 0);
                    					cout<<"\nreceived response\n";
                    					buf[rlen] = '\0';
                    					cout<<"\n"<<rlen<<"\n";
                    					write (sock, lname, t.size());
        //wait for confirmation
                    					rlen = recv(sock, buf, 99999, 0);
                    					cout<<"\nreceived response\n";
                    					cout<<"\n"<<rlen<<"\n";

                                    	s = "put ";
                                    	t = pswd;
                                    	s.append(uname);
                                    	s.append(",password ");
                                    	s.append(to_string(t.size()));
                                    	s.append(" 3");
                                    	s.append("\r\n");
                                    	c = (char*)s.c_str();

                                    	write (sock, c, s.size());
                    					cout<<"\nwaiting to receive response\n";

                    	                rlen = recv(sock, buf, 99999, 0);
                    					cout<<"\nreceived response\n";
                    					buf[rlen] = '\0';
                    					cout<<"\n"<<rlen<<"\n";
                    					write (sock, pswd, t.size());
        //wait for confirmation
                    					rlen = recv(sock, buf, 99999, 0);
                    					cout<<"\nreceived response\n";
                    					cout<<"\n"<<rlen<<"\n";

                                	    s = "set-cookie: id=";
                                        s.append(uname);
                                        s.append("\n\n");
                                        char *c = (char*)s.c_str();

                                        t = "<!DOCTYPE html><html><body><h1>Hello ";
                                        t.append(fname);
                                        t.append("</h1><a href=\"/viewfolder/root/\">Drive</a><a href=\"mail.html\">Mail</a></body></html>");

                                        char *c1 = (char*)t.c_str();
                                        send(clients[slot], "HTTP/1.1 200 OK\n", 16, 0);
                                        send(clients[slot], c, s.size(), 0);
                                        write (clients[slot], c1, t.size());

                                	}
                                	else{
                                		if ( (fd=open(path, O_RDONLY))!=-1 ){
                                			send(clients[slot], "HTTP/1.1 200 OK\n\n", 17, 0);
                                			while ( (bytes_read=read(fd, data_to_send, BYTES))>0 ){
                                				write (clients[slot], data_to_send, bytes_read);
                                			}
                                		}
                                	}
                            	}

                            	else{
                                	if ( (fd=open(path, O_RDONLY))!=-1 ){
                                		send(clients[slot], "HTTP/1.1 200 OK\n\n", 17, 0);
                                		while ( (bytes_read=read(fd, data_to_send, BYTES))>0 ){
                                			write (clients[slot], data_to_send, bytes_read);
                                		}
                                	}
                            	}
                            }

                            else if (strstr(path, "/newmail.html") != NULL){

                            	char *temp, *rcvr, *head, *text;

                            	if ( (temp = strcasestr(store,"receiver=")) != NULL ){

                            		temp+=9;
                            		int count = 0;
                            		char *temp1 = (char*)malloc (1 + strlen (temp));
                            		strcpy(temp1, temp);
                            		while (temp[0] != '&'){
                            			count ++;
                            			temp++;
                            		}
                            		rcvr = (char*)malloc(count);
                            		strncpy(rcvr, temp1, count);
                            		printf("\n%s\n", rcvr);

                            	}

                            	if ( (temp = strcasestr(store,"header=")) != NULL ){
                            		temp+=7;
                            		int count = 0;
                            		char *temp1 = (char*)malloc (1 + strlen (temp));
                            		strcpy(temp1, temp);
                            		while (temp[0] != '&'){
                            			count ++;
                            			temp++;
                            		}
                            		head = (char*)malloc(count);
                            		strncpy(head, temp1, count);
                            		printf("\n%s\n", head);

                            	}


                            	if ( (temp = strcasestr(store,"text=")) != NULL ){
                            		temp+=5;

                            		int count = 0;
                            		char *temp1 = (char*)malloc (1 + strlen (temp));
                            		strcpy(temp1, temp);
                            		while (temp[0] != '&'){
                            			count ++;
                            			temp++;
                            		}
                            		text = (char*)malloc(count);
                            		strncpy(text, temp1, count);
                            		printf("\n%s\n", text);

                            	}

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

                            	s = "SEND_EMAIL~";
                            	s.append(uname);
                            	s.append("@localhost~");
                            	s.append(rcvr);
                            	s.append("~");
                            	s.append(head);
                            	s.append("~");
                            	s.append(text);
                            	cout<<"\nSTRING: "<<s<<"\n";

                            	char *c = (char*)s.c_str();
                            	write (mailsock, c, s.size());

            					int rlen = recv(mailsock, buf, 99999, 0);

            					reqline[1] = "/mail.html";
            		    		strcpy(path, ROOT);
            		            strcpy(&path[strlen(ROOT)], reqline[1]);

                            	if ( (fd=open(path, O_RDONLY))!=-1 ){
                            		send(clients[slot], "HTTP/1.1 200 OK\n\n", 17, 0);
                            	    while ( (bytes_read=read(fd, data_to_send, BYTES))>0 ){
                            	    	write (clients[slot], data_to_send, bytes_read);
                            	    }
                            	}
                            }

                            else if ((strstr(path, "movefolder") != NULL)){

                            	char *temp, *depth, *flname, *newpath;
                            	int loc = 0;

                            	if ( (temp = strcasestr(path,"movefolder")) != NULL ){
                            		temp+=10;
                            		s = temp;
                            		for (int i = 0; i<s.size()-1; i++){
                            			if(temp[i] == '/'){
                            				loc = i+1;
                            			}
                            		}
                            		depth = (char*)malloc(loc);
                            		strncpy(depth, temp, loc);

                            		printf("\ndepth:%send", depth);
                            	}
                            	string u = temp;
                            	string v = u.substr(loc, s.size());
                            	cout<<"\nFoldername:"<<v;
                            	flname = (char*)v.c_str();

                            	if ( (temp = strcasestr(store,"id=")) != NULL ){
                            		temp+=3;
                            		int count = 0;
                            		char *temp1 = (char*)malloc (1 + strlen (temp));
                            		strcpy(temp1, temp);
                            		while (temp[0] != '\r' && temp[0] != '\n'){
                            			count ++;
                            			temp++;
                            		}
                            		uname = (char*)malloc(count);
                            		strncpy(uname, temp1, count);

                            		printf("\nuname%send", uname);
                            	}

                            	if ( (temp = strcasestr(store,"newpath=")) != NULL ){
                            		temp+=8;
                            		newpath = temp;
                            		printf("\nuname%send", newpath);
                            	}

                            	s = newpath;
                            	string y = "%2F";
                            	string z = "/";
                            	t = ReplaceString(s, y, z);

                            	//#MoveFolder#+username=vikram+path=/root/abc/+newpath=/root/def/+foldername=pqr.txt+#EndOfCommand#

                            	s = "#MoveFolder#+username=";
                            	s.append(uname);
                            	s.append("+path=");
                            	s.append(depth);
                            	s.append("+newpath=");
                            	s.append(t);
                            	s.append("+foldername=");
                            	s.append(flname);
                            	s.append("+#EndOfCommand#");
                            	cout<<"\nSTRING: "<<s<<"\n";
                            	char *c = (char*)s.c_str();
                            	write (drivesock, c, s.size());
            					cout<<"\nSENT\n";
                            	int rlen = recv(drivesock, buf, 99999, 0);
            					s=buf;
            					cout<<"\nReceived: "<<s<<"\n";
            					t = "<!DOCTYPE html><html><body><h1>";
            					t.append(flname);
            					t.append(" moved successfully!</h1> <a href=\"/viewfolder");
            					t.append(depth);
            					t.append("\">Back</a></body></html>");
            					char *c1 = (char*)t.c_str();
            					send(clients[slot], "HTTP/1.1 200 OK\n\n", 17, 0);
            					write (clients[slot], c1, t.size());



                            }

                            else if ((strstr(path, "movefile") != NULL)){
                            	char *temp, *depth, *flname, *newpath;
                            	int loc = 0;

                            	if ( (temp = strcasestr(path,"movefile")) != NULL ){
                            		temp+=8;
                            		s = temp;
                            		for (int i = 0; i<s.size(); i++){
                            			if(temp[i] == '/'){
                            				loc = i+1;
                            			}
                            		}
                            		depth = (char*)malloc(loc);
                            		strncpy(depth, temp, loc);

                            		printf("\ndepth:%send", depth);
                            	}
                            	string u = temp;
                            	string v = u.substr(loc, s.size());
                            	cout<<"\nFilename:"<<v;
                            	flname = (char*)v.c_str();

                            	if ( (temp = strcasestr(store,"id=")) != NULL ){
                            		temp+=3;
                            		int count = 0;
                            		char *temp1 = (char*)malloc (1 + strlen (temp));
                            		strcpy(temp1, temp);
                            		while (temp[0] != '\r' && temp[0] != '\n'){
                            			count ++;
                            			temp++;
                            		}
                            		uname = (char*)malloc(count);
                            		strncpy(uname, temp1, count);

                            		printf("\nuname%send", uname);
                            	}

                            	if ( (temp = strcasestr(store,"newpath=")) != NULL ){
                            		temp+=8;
                            		newpath = temp;
                            		printf("\nuname%send", newpath);
                            	}
                            	s = newpath;
                            	string y = "%2F";
                            	string z = "/";
                            	t = ReplaceString(s, y, z);

                            	//#MoveFile#+username=vikram+path=/root/abc/+newpath=/root/def/+filename=pqr.txt+#EndOfCommand#
                            	s = "#MoveFile#+username=";
                            	s.append(uname);
                            	s.append("+path=");
                            	s.append(depth);
                            	s.append("+newpath=");
                            	s.append(t);
                            	s.append("+filename=");
                            	s.append(flname);
                            	s.append("+#EndOfCommand#");
                            	cout<<"\nSTRING: "<<s<<"\n";
                            	char *c = (char*)s.c_str();
                            	write (drivesock, c, s.size());
            					cout<<"\nSENT\n";
                            	int rlen = recv(drivesock, buf, 15, 0);
            					s=buf;
            					cout<<"\nReceived: "<<s<<"\n";
            					t = "<!DOCTYPE html><html><body><h1>";
            					t.append(flname);
            					t.append(" moved successfully!</h1> <a href=\"/viewfolder");
            					t.append(depth);
            					t.append("\">Back</a></body></html>");
            					char *c1 = (char*)t.c_str();
            					send(clients[slot], "HTTP/1.1 200 OK\n\n", 17, 0);
            					write (clients[slot], c1, t.size());


                            }

                            else if (strstr(path, "/createfolder.html") != NULL){

                            	char *temp, *fdr, *depth;

                            	if ( (temp = strcasestr(store,"foldername=")) != NULL ){
                            		temp+=11;
                            		fdr = temp;
                            		cout<<"\nSTART"<<fdr<<"END\n";
                            	}

                            	string u = path;
                            	u.erase(u.length()-17);
                            	char *c = (char*)u.c_str();
                            	if ( (temp = strcasestr(c,"viewfolder")) != NULL ){
                            		temp+=10;
                            		depth = temp;
                            		printf("\ndepth:%send", depth);
                            	}
                            	u = depth;

                            	if ( (temp = strcasestr(store,"id=")) != NULL ){
                            		temp+=3;
                            		int count = 0;
                            		char *temp1 = (char*)malloc (1 + strlen (temp));
                            		strcpy(temp1, temp);
                            		while (temp[0] != '\r' && temp[0] != '\n'){
                            			count ++;
                            			temp++;
                            		}
                            		uname = (char*)malloc(count);
                            		strncpy(uname, temp1, count);

                            		printf("\n%s", uname);
                            	}

                            	s = "#AddFolder#+username=";
                            	s.append(uname);
                            	s.append("+path=");
                            	s.append(depth);
                            	s.append("+foldername=");
                            	s.append(fdr);
                            	s.append("/+#EndOfCommand#");
                            	cout<<"\nSTRING: "<<s<<"\n";
                            	c = (char*)s.c_str();

                            	write (drivesock, c, s.size());

            					int rlen = recv(drivesock, buf, 99999, 0);
            					cout<<"\n"<<rlen<<"\n";
            					cout<<"\n\nDEPTH:"<<depth<<"end";
                                t = "<!DOCTYPE html><html><body><h1>";
                                t.append(fdr);
                                t.append(" added successfully!</h1> <a href=\"/viewfolder");
                                t.append(depth);
                                t.append("\">Back</a></body></html>");
                                char *c1 = (char*)t.c_str();
                                send(clients[slot], "HTTP/1.1 200 OK\n\n", 17, 0);
                                write (clients[slot], c1, t.size());

                            }

                            else if (strstr(path, "/fileupload.html") != NULL){
                            	cout<<"\nHERE\n";
                            	char *temp, *flname, *depth, *clength, *content, *boundary;

                            	if ( (temp = strcasestr(store,"userfile=")) != NULL ){
                            		temp+=9;
                            		flname = temp;
                            		printf("\nFilename: %s", flname);
                            	}

                            	if ( (temp = strcasestr(store,"Content-Length: ")) != NULL ){
                            		temp+=16;
                            		int count = 0;
                            		char *temp1 = (char*)malloc (1 + strlen (temp));
                            		strcpy(temp1, temp);
                            		while (temp[0] != '\n' && temp[0] != '\r'){
                            			count ++;
                            			temp++;
                            		}
                            		clength = (char*)malloc(count);
                            		strncpy(clength, temp1, count);

                            		printf("\n%s", clength);
                            	}

                            	string u = path;
                            	u.erase(u.length()-15);
                            	char *c = (char*)u.c_str();
                            	if ( (temp = strcasestr(c,"viewfolder")) != NULL ){
                            		temp+=10;
                            		depth = temp;
                            		printf("\ndepth:%send", depth);
                            	}
                            	u = depth;

                            	if ( (temp = strcasestr(store,"id=")) != NULL ){
                            		temp+=3;
                            		int count = 0;
                            		char *temp1 = (char*)malloc (1 + strlen (temp));
                            		strcpy(temp1, temp);
                            		while (temp[0] != '\r' && temp[0] != '\n'){
                            			count ++;
                            			temp++;
                            		}
                            		uname = (char*)malloc(count);
                            		strncpy(uname, temp1, count);

                            		printf("\n%s", uname);
                            	}

                            	s = "#Upload#+username=";
                            	s.append(uname);
                            	s.append("+path=");
                            	s.append(depth);
                            	s.append("+filename=");
                            	s.append(flname);
                            	s.append("+size=");
                            	s.append(clength);
                            	s.append("+#EndOfCommand#");
                            	cout<<"\nSTRING: "<<s<<"\n";
                            	c = (char*)s.c_str();

                            	write (drivesock, c, s.size());

            					int rlen = recv(drivesock, buf, 99999, 0);
            					cout<<"\n"<<rlen<<"\n";

            					FILE *fp = fopen(flname,"rb");
                            	if(fp==NULL)
                            	{
                            		printf("File open error");
                            		return 1;
                            	}

                            	while(1)
                            	{
                            		unsigned char buff[256]={0};
                            		int nread = fread(buff,1,256,fp);
                            		printf("Bytes read %d \n", nread);

                            		if(nread > 0)
                            		{
                            			printf("Sending \n");
                            			write(drivesock, buff, nread);
                            			cout<<buff;
                            		}

                            		if (nread < 256)
                            		{
                            			if (feof(fp))
                            				printf("End of file\n");
                            			if (ferror(fp))
                            				printf("Error reading\n");
                            			break;
                            		}
                            	}
                            	cout<<"\nout of loop\n";
                            	write(drivesock, "<EOB>", 5);
                            	cout<<"\n\nDEPTH:"<<depth<<"end";

                            	t = "<!DOCTYPE html><html><body><h1>";
                            	t.append(flname);
                            	t.append(" uploaded successfully!</h1> <a href=\"/viewfolder");
                            	t.append(depth);
                            	t.append("\">Back</a></body></html>");
                            	char *c1 = (char*)t.c_str();
                            	//cout<<"\n"<<t;
                            	send(clients[slot], "HTTP/1.1 200 OK\n\n", 17, 0);
                            	write (clients[slot], c1, t.size());

                            }
                        }
                    }
                }
                shutdown (clients[slot], SHUT_RDWR);
                close(clients[slot]);
                clients[slot]=-1;
                exit(0);
            }
        }

        while (clients[slot]!=-1){
        	slot = (slot+1)%CONNMAX;
        }
    }
    return 0;
}
