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
#include "User.h"
#define PINFINITY 999

#define MAX_PROCESSES 10


vector<string> listOfUsers;

using namespace std;

struct Server{

};

struct serverAddress{
    string forwardAddress = "";
    string bindAddress = "";
    int forwardPortNumber = 0;
    int bindPortNumber = 0;
};

struct clientStruct{
    struct sockaddr_in address;
    int chatRoomId = -1;
    string nickName = "";
};

/*void *startServer(void *client) {  
    /////cout<<"Start of thread execution \r\n";

    User *clientData;
    clientData = (struct thread_data *)client;

    bzero(clientData->buffer, 1000);
    long int numBytesRead = 0,
        found = 0, dataFound = string::npos;

    int clientSocket = 0,
        quitFlag = 0,
        dataCommandFlag = 0;

    clientSocket = clientData->clientSocket;*/




int main(int argc,char* argv[]) {

    ///server creation
    /*ifstream serverFile;
    serverFile.open(argv[1]);
    serverFile.unsetf(std::ios_base::skipws);
    unsigned int serverCount = std::count(istream_iterator<char>(serverFile),istream_iterator<char>(), '\n');
    serverFile.close();

    serverFile.open(argv[1]);
    struct serverAddress listOfServers[serverCount + 2];
    string forwardAddrFromFile = "",
        bindAddrFromFile = "",
        serverAddressFromFile,
        orderingMode = "";
    

    char token;
    int findToken = -1,
        findPortNumToken = -1,
        totalOrderFlag = 0,
        fifoOrderFlag = 1,
        unorderedFlag = 0;
    


    for(int argIndex = 0;argIndex < argc;argIndex++){
        if(strcmp(argv[argIndex], "-o") == 0){
            orderingMode = string(argv[argIndex + 1]);
        }
    }

    
    while(getline(serverFile, serverAddressFromFile)){
        findToken = serverAddressFromFile.find(',');
        findPortNumToken = serverAddressFromFile.find(':');
        if(findToken != string::npos){
            listOfServers[serverIndex].forwardAddress = serverAddressFromFile.substr(0,findPortNumToken);
            listOfServers[serverIndex].forwardPortNumber = stoi(serverAddressFromFile.substr(findPortNumToken + 1,findToken));

            serverAddressFromFile = serverAddressFromFile.substr(findToken+1);
            
            listOfServers[serverIndex].bindAddress = serverAddressFromFile.substr(0, findPortNumToken);
        }
        else{
            listOfServers[serverIndex].forwardAddress = serverAddressFromFile.substr(0,findPortNumToken);
            listOfServers[serverIndex].forwardPortNumber = stoi(serverAddressFromFile.substr(findPortNumToken+1));
            
            listOfServers[serverIndex].bindAddress = serverAddressFromFile.substr(0,findPortNumToken);  
        }

        
        findPortNumToken = serverAddressFromFile.find(':');
        listOfServers[serverIndex].bindPortNumber = stoi(serverAddressFromFile.substr(findPortNumToken+1));
        serverIndex++;  
    }*/


    //server creation
    setvbuf(stdout, NULL, _IONBF, 0);
    int k = 0;
    int flag_threads = 1;


    long long int sum = 0;   // Total number of inputs/numbers to be sorted
    string listOfFiles[100];

    int portNo = 10000;
    int sockfd = socket(PF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        cout << "cannot open socket\n";
        exit(-1);
    }

    struct sockaddr_in servaddr;
    vector<struct sockaddr_in> clientAddr;
    clientAddr.reserve(sizeof(int) * PINFINITY);
    bzero(&servaddr, sizeof(servaddr));
    int enable = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(int));

    servaddr.sin_family = PF_INET;
    servaddr.sin_port = htons(portNo);
    servaddr.sin_addr.s_addr = htons(INADDR_ANY);

    inet_pton(PF_INET, "127.0.0.1", &(servaddr.sin_addr));
    /*bind and listen*/
    bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

    listen(sockfd, 1000);

    int serverSocket;
    vector<socklen_t> clientAddrLen;
    clientAddrLen.reserve(sizeof(servaddr) * PINFINITY);


    vector<pthread_t> client_thread;
    client_thread.reserve(sizeof(int) * PINFINITY);
    int result;
    int empty = 0,
            temp = k,
            numOfClients = -1;

    pthread_t threads[PINFINITY];
    vector<User> clients;
    User currentClient;
    struct sockaddr_in serverToConnect;
    socklen_t serverToConnectAddr;

    struct sockaddr_in serverToFetch;
    socklen_t serverToFetchAddr;

    

    auto numBytes = 0;

    //int count = 0;
    string userName = "";
    string folderPath = "",
        newFolderPath = "";
    string filePath = "",
        fileName = "",
        folderName = "";
    string commandToSend = "";
    char buffer[10000];
    string input = "", dataString = "";
    int dataSize;
    auto found = string::npos;
    int commandSize = 0;
    auto commandFound = string::npos;
    auto nameFound = string::npos;
    auto delimFound = string::npos;
    auto endOfDataFound = string::npos;
    auto fileTokenFound = string::npos;
    string command = "";
    auto count = 0;
    vector<fstream> files;
    const string commands[] = {"#Upload#","#Download#","#Contents#","#AddFolder#","#MoveFile#","#DeleteFolder#","#DeleteFile#","#Quit#"};
    const string replies[] = {"#DeleteFileComplete#","#DeleteComplete#","#MoveComplete"};

    while(true){
        serverSocket = accept(sockfd, (struct sockaddr *) &serverToConnect, &serverToConnectAddr);
      while(true){  
        while(found == string::npos){
            cout<<"reading data"<<endl;
            cout<<input<<endl;
            numBytes = read(serverSocket, buffer, 10000);
            input = input + string(buffer);
            bzero(buffer,1000);
            found = input.find("#EndOfCommand");
            //commandFound = string::npos;
        }

        cout<<"command received"<<input<<endl;
    
        // //////////


        // //////
        for(int i = 0;i < 8;i++){
            commandFound = input.find(commands[i]);
            if(commandFound != string::npos){
                commandSize = strlen(commands[i].c_str());
                cout<<"command is : "<<commands[i]<<endl;    
            }
        }

        command = input.substr(0,commandSize);
        input = input.substr(commandSize + 1);

        
        nameFound = input.find("username=");
        auto length = nameFound + strlen("username=");
        input = input.substr(length);
        delimFound = input.find("+");
        cout<<"length is "<<length<<endl;
        //auto endFound = input.find("#EndOfCommand#");
        auto endIndex = strlen(input.c_str()) - 2;
        cout<<"input is :"<<input<<endl;
        cout<<"length is "<<endIndex<<endl;
        //userName = input.substr(length,(delimFound - length));
        userName = input.substr(0,delimFound);//endIndex-strlen("#EndOfCommand#"));
        cout<<"username found is : "<<userName<<endl;
    

        for(int i = 0;i < listOfUsers.size();i++){
            if(strcmp(userName.c_str(), listOfUsers[i].c_str()) != 0){
                count++;
            }
        }

        /*Check if user exists*/
        if(count == listOfUsers.size()){
            listOfUsers.push_back(userName);
            count = 0;
            currentClient.userName = userName;
            currentClient.fileName = userName + "listOfFolders.txt";
            cout<<"file name is :"<<currentClient.fileName<<endl;

            //clients.push_back(currentClient);
        }
        else if(!currentClient.fileFetched){
            currentClient.userName = userName;
            currentClient.fileName = userName + "listOfFolders.txt";
            currentClient.fileFetched = true;
        }

    //     /* Extracting information from a command such as #Upload#+username=vikram+path=/root/abc/+filename=pqr.txt+#EndOfCommand# */

        if(strcmp(command.c_str(),commands[0].c_str()) == 0){
            input = input.substr(delimFound + 1);
            auto length = input.find("path=");
            input = input.substr(0 + strlen("path="));
            delimFound = input.find("filename=");
            folderPath = input.substr(0,delimFound - 1);
            cout<<"folder path is :"<<folderPath<<endl;
            input = input.substr(delimFound + strlen("filename="));
            length = input.find("#EndOfCommand#");
            fileName = input.substr(0,length - 1);

            cout<<"filename is :"<<fileName<<endl;
            
            input = input.substr(length + strlen("#EndOfCommand#"));
            //currentClient.handleUpload(serverSocket,currentClient.userName,folderPath,fileName);

        }
            /* Extracting information from a command such as #Download#+username=vikram+path=/root/abc/+filename=pqr.txt+#EndOfCommand# */

        else if(strcmp(command.c_str(),commands[1].c_str()) == 0){
            input = input.substr(delimFound + 1);
            auto length = input.find("path=");
            input = input.substr(0 + strlen("path="));
            delimFound = input.find("filename=");
            folderPath = input.substr(0,delimFound - 1);
            cout<<"folder path is :"<<folderPath<<endl;
            input = input.substr(delimFound + strlen("filename="));
            length = input.find("#EndOfCommand#");
            filePath = input.substr(0,length - 1);
            cout<<"file path is :"<<filePath<<endl;
            input = input.substr(length + strlen("#EndOfCommand#"));
            cout<<"socket is "<<serverSocket<<endl;
            string pathToSend = folderPath + filePath;
            string name = currentClient.userName;
            //currentClient.handleDownload(serverSocket,name,pathToSend);//,serverToConnect,serverToConnectAddr);

        }

            /* Extracting information from a command such as #Contents#+username=vikram+path=/root/abc/#EndOfCommand#*/

        else if(strcmp(command.c_str(),commands[2].c_str()) == 0){
            input = input.substr(delimFound + 1);
            length = input.find("path=");
            input = input.substr(0 + strlen("path="));
            length = input.find("#EndOfCommand#");
            folderPath = input.substr(0,length - 1);
            input = input.substr(length+strlen("#EndOfCommand#"));
            //currentClient.handleContents(serverSocket,currentClient.userName,folderPath);


        }
            /* Extracting information from a command such as #AddFolder#+username=vikram+path=/root/abc/+foldeername=lmn/#EndOfCommand#*/

        else if(strcmp(command.c_str(),commands[3].c_str()) == 0){
            input = input.substr(delimFound + 1);
            length = input.find("path=");
            input = input.substr(0 + strlen("path="));
            delimFound = input.find("foldername=");
            folderPath = input.substr(0,delimFound - 1);
            input = input.substr(delimFound + strlen("foldername="));
            length = input.find("#EndOfCommand#");
            folderName = input.substr(0,length - 1);
            input = input.substr(length + strlen("#EndOfCommand#"));
            //currentClient.handleAddFolder(serverSocket,currentClient.userName,folderPath,folderName);
        }
            /* Extracting information from a command such as #AddFolder#+username=vikram+path=/root/abc/+newpath=/root/lmn/+filename=pq.txt+#EndOfCommand#*/

        else if(strcmp(command.c_str(),commands[4].c_str()) == 0){
            input = input.substr(delimFound + 1);
            length = input.find("path=");

            input = input.substr(0 + strlen("path="));
            cout<<"after username : "<<input<<endl;
            length = input.find("newpath=");
            folderPath = input.substr(0,length - 1);


            input = input.substr(length+strlen("newpath="));

            newFolderPath = input.substr(0,input.find("+"));

            delimFound = input.find("filename=");
            input = input.substr(delimFound + strlen("filename="));
            length = input.find("#EndOfCommand#");
            fileName = input.substr(0,length - 1);
            input = input.substr(length + strlen("#EndOfCommand#"));

            //currentClient.handleMoveFile(serverSocket,currentClient.userName,folderPath,newFolderPath,fileName);
        }
            /* Extracting information from a command such as #AddFolder#+username=vikram+path=/root/abc/+foldername=lmn/#EndOfCommand#*/

        else if(strcmp(command.c_str(),commands[5].c_str()) == 0){
            input = input.substr(delimFound + 1);
            length = input.find("path=");
            input = input.substr(0 + strlen("path="));
            delimFound = input.find("foldername=");
            folderPath = input.substr(0,delimFound - 1);
            input = input.substr(delimFound + strlen("foldername="));
            length = input.find("#EndOfCommand#");
            folderName = input.substr(0,length - 1);
            input = input.substr(length + strlen("#EndOfCommand#"));
            //currentClient.handleDeleteFolder(serverSocket,currentClient.userName,folderPath,folderName);
        }
            /* Extracting information from a command such as #AddFolder#+username=vikram+path=/root/abc/+filename=pq.txt#EndOfCommand#*/

        else if(strcmp(command.c_str(),commands[6].c_str()) == 0){
            input = input.substr(delimFound + 1);
            length = input.find("path=");
            input = input.substr(0 + strlen("path="));
            delimFound = input.find("filename=");
            folderPath = input.substr(0,delimFound - 1);
            input = input.substr(delimFound + strlen("filename="));
            length = input.find("#EndOfCommand#");
            fileName = input.substr(0,length - 1);
            input = input.substr(length + strlen("#EndOfCommand#"));

            //currentClient.handleDeleteFile(serverSocket,currentClient.userName,folderPath,fileName);
        }
        else if(strcmp(command.c_str(),commands[7].c_str()) == 0){
            break;
        }
           found = string::npos;
            input = "";
        }
    }
    close(sockfd);

    //     found = string::npos;
    // }
    // close(sockfd);

    //exit(0);

}


