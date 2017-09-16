#ifndef USER_H
#define USER_H

#include <iostream>
#include <stdio.h>
#include <fstream>
#include <string>
#include <arpa/inet.h>
using namespace std;

class User {
public:
    string UUID;
    fstream files;
    string rootFolder;
    string userName;
    string fileName;
    string serverAddress;
    bool fileFetched;
    int fd;
    int numBytesRead;
    int serverSocket;
    bool transactionState;
    int storageSocket;
    struct sockaddr_in storage;
    int invocation_id;


    void handleUpload(int serverSocket,string userName,string folderPath,string fileName,int sizeOfFile);
    void handleDownload(int serverSocket,string userName,string filePath,struct sockaddr_in serverToConnect,socklen_t serverToConnectAddr);
    //void handleDownload(int serverSocket,std::string userName,std::string filePath);
    void handleContents(int serverSocket,string userName,string folderPath);
    void handleAddFolder(int serverSocket,string userName,string folderPath,string folderName);
    void handleMoveFile(int serverSocket,string userName,string folderPath,string newFolderPath,string fileName);
    void handleDeleteFolder(int serverSocket,string userName,string folderPath,string folderName);
    void handleDeleteFile(int serverSocket,string userName,string folderPath,string fileName);
};

//void handleDownload(int serverSocket,std::string userName,std::string filePath);



#endif
