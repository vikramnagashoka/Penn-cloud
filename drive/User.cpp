//
// Created by Vikram Nag on 4/26/17.
//
#include <iostream>
#include <stdio.h>
//#include <curl/curl.h>
#include <string.h>
#include <arpa/inet.h>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <bitset>
#include<iosfwd>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sstream>
#include <string>
#include <vector>
//#include "binmake/binmake/include/BinStream.h"
//#include <curl/curl.h>
#include "User.h"
#include <arpa/inet.h>
using namespace std;
//using namespace BS;

void User::handleUpload(int serverSocket,string userName,string folderPath,string fileName,int sizeOfFile){
    FILE * pTextFile, *pBinaryFile;
    //char buffer[1];
    //////////////////////////////////////////

    //////////////////////////////////////////

    //Receive file from front end
    int bytesRead = 0;
    char bufferUpload[sizeOfFile];
    string data = "";
    /*while(bytesRead <= sizeOfFile) {
        int num = read(serverSocket, bufferUpload, sizeOfFile);
        bytesRead += num;
        data += string(bufferUpload);
        cout<<"bytesRead : "<<bytesRead<<endl;
    }*/


    //cout<<"bytes read is "<<data<<endl;
    //Receive file from front end
    string filename = userName + "listOfFolders.txt";
    //string binaryfname = userName + "listOfFolders.bin";
    cout<<"filename :"<<filename<<endl;
    //cout<<"binary file name is : "<<binaryfname<<endl;

    //string data = "100101100110010110011001011001100101100110010110011001011001100101100110010110011001011"
//            "100101101";

    ;

    int dataSize = strlen(data.c_str());
    this->invocation_id = this->invocation_id + 1;
    string filePath = folderPath + fileName;

    //cout<<"data size is"<<dataSize<<endl;
    //cout<<"command sent is: "<<getDataCommand<<endl;
    // int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    // struct sockaddr_in keyValueStore;
    // keyValueStore.sin_family = PF_INET;
    // keyValueStore.sin_port = htons(5001);
    // keyValueStore.sin_addr.s_addr = htons(INADDR_ANY);
    // inet_pton(PF_INET, "127.0.0.1", &(keyValueStore.sin_addr));
    // socklen_t keyValueStoreAddr;
    int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    bind(sockfd, (struct sockaddr *) &this->storage, sizeof(this->storage));
    if (connect(sockfd, (struct sockaddr*)&this->storage, sizeof(this->storage)) < 0) {
        perror("ERROR connecting");
        exit(1);
    }
    else{
        cout<<"connection successful"<<endl;
    }

    /////////////
    char recvBuff[10000];
    FILE *fp;
    fp = fopen(fileName.c_str(), "ab+");
    cout<<"filename is "<<fileName<<endl;
    if(NULL == fp)
    {
        printf("Error opening file");
    }
    else{
        cout<<"file opened"<<endl;
    }

    /* Receive data in chunks of 256 bytes */
    int bytesReceived;
    int endOfFile = string::npos;
    while(endOfFile == string::npos)
    {   cout<<"before read"<<endl;
        bytesReceived = read(serverSocket, recvBuff, 10000);
        endOfFile = string(recvBuff).find("<EOB>");
        //printf("Bytes received %d\n",bytesReceived);
        // recvBuff[n] = 0;
        fwrite(recvBuff, 1,bytesReceived,fp);
        data += string(recvBuff);
        // printf("%s \n", recvBuff);
    }

    data = data.substr(0,endOfFile);
    cout<<"data is: "<<data<<endl;
    string binString = "";
    for (std::size_t i = 0; i < data.size(); ++i)
    {
        bitset<8> b(data.c_str()[i]);
        binString+= b.to_string();
    }

    if(bytesReceived < 0)
    {
        printf("\n Read Error \n");
    }

     fclose(fp);
    /////////////

    int numBytesWrite = write(sockfd, "yo\r\n", strlen("yo\r\n"));


    int found = string::npos,
            numBytesRead;
    string input = "";
    char buffer[1000];
    while(found == string::npos){
        numBytesRead = read(sockfd, buffer,1000);
        if(numBytesRead > 0) {
            input += string(buffer);
            found = input.find("+OK");
            cout<<input<<endl;
        }
    }
    bzero(buffer,1000);
    found = string::npos;
    input = "";
    dataSize = strlen(binString.c_str());


    string getDataCommand =  "put " + userName + "," + filePath +  " " + std::to_string(dataSize) + " " +std::to_string(this->invocation_id) + "\r\n";//;" 10\r\n";

    numBytesWrite = write(sockfd, getDataCommand.c_str(), strlen(getDataCommand.c_str()));

    while(found == string::npos){
        //numBytesWrite = write(sockfd, data.c_str(), strlen(data.c_str()));
        numBytesRead = read(sockfd, buffer,1000);
        if(numBytesRead > 0){
            input += string(buffer);
            found = input.find("+OK");
            //found = string(buffer).find("+OK");
            cout<<input<<endl;
        }
    }
    found = string::npos;
    bzero(buffer,1000);
    input = "";
    numBytesWrite = write(sockfd, binString.c_str(), strlen(binString.c_str()));
    while(found == string::npos){
        numBytesRead = read(sockfd,buffer,1000);
        if(numBytesRead > 0){
            input += string(buffer);
            found = input.find("+OK");
            //found = string(buffer).find("+OK");
            cout<<input<<endl;
        }
    }

    if(found != string::npos){

    }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // string data = input.substr(0,found - 1);
    // input = input.substr(found - 1);


    // // ////// GET DATA
    // curl_global_init(CURL_GLOBAL_ALL);

    // /* get a curl handle */
    // curl = curl_easy_init();
    // if(curl) {

    //     //const char *data = "data to send data to send data";
    //     //curl_easy_setopt(curl, CURLOPT_URL, "http://postit.example.com/moo.cgi");

    //     char* server;
    //     inet_ntop(AF_INET, &(serverToConnect.sin_addr), server, INET_ADDRSTRLEN);

    //     curl_easy_setopt(curl, CURLOPT_URL, server);
    //     /* Now specify the POST data */
    //     curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long) strlen(data.c_str()));
    //     curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());

    //     /* Perform the request, res will get the return code */
    //     res = curl_easy_perform(curl);
    //     /* Check for errors */
    //     if(res != CURLE_OK)
    //         fprintf(stderr, "curl_easy_perform() failed: %s\n",
    //                 curl_easy_strerror(res));

    //     /* always cleanup */
    //     curl_easy_cleanup(curl);
    //     //cout<<“abcd success”;
    // }
    // curl_global_cleanup();
    cout<<"in handle upload"<<dataSize<<endl;
    /* updating user's file*/
    cout<<"in upload updatefunction"<<endl;
    ifstream file;
    file.open(filename.c_str());
    string line;
    //string reply = "#AddComplete# + username=" + userName + "path=" + folderPath + "foldername="+folderName + "#End#";
    //string reply = "#AddComplete#\r\n";
    int folderFoundFlag = 0;
    vector<string> filecontents;
    while(std::getline(file, line)){
        filecontents.push_back(line);
        cout<<line<<endl;
    }
    file.close();
    ofstream outfile;
    outfile.open(filename.c_str(), std::ofstream::out | std::ofstream::trunc);
    outfile.close();
    outfile.open(filename.c_str());
    for(unsigned i = 0;i<filecontents.size();i++){
        cout<<filecontents[i]<<endl;
        if(strcmp(filecontents[i].c_str(),folderPath.c_str()) == 0){
            outfile << filecontents[i]<<"\n";
            outfile << fileName <<"\n";
            //folderFoundFlag = 1;
        }
        else{
            outfile << filecontents[i]<<"\n";
        }
    }
    outfile.close();
    cout<<"upload complete"<<endl;
    //cout<<"reply sent is : "<<reply<<endl;
    //int num = write(serverSocket,reply.c_str(),strlen(reply.c_str()));


    close(sockfd);
    //BinStream bin;
    //ifstream inf(filename.c_str());
    //ofstream ouf("example.bin");
    //bin << inf >> ouf;

//    pTextFile = fopen(filename.c_str(), "r");
//    pBinaryFile = fopen(binaryfname.c_str(), "wb");
//    while (!feof(pTextFile))
//    {
//        //fread(buffer, 1, 1, pTextFile);
//        //fwrite(buffer, 1, 1, pBinaryFile);
//    }
//    vector<string> fileContents;
//
//    fclose(pTextFile);
//    fclose(pBinaryFile);
//
//    std::ifstream in(filename.c_str());
//    std::ofstream out(binaryfname.c_str(), std::ios::binary);
//
//    double d;
//    while(in >> d) {
//        out.write((char*)&d, sizeof d);
//    }
//
//    ifstream file;
//    file.open(binaryfname.c_str());
//    string line = "";
//    while (std::getline(file, line)){
//        fileContents.push_back(line);
//        cout<<line<<endl;
//    }
//
//    char buffers;
//    ifstream inn(filename.c_str());
//    ofstream outt("binfile.bin", ios::out|ios::binary);
//    int nums[3];
//    while (!inn.eof())
//    {
//        inn >> nums[0] >> nums[1] >> nums[2];
//
//        outt.write(reinterpret_cast<const char*>(nums), 3*sizeof(int));
//
//    }


}

void User::handleDownload(int serverSocket,std::string userName,std::string filePath,struct sockaddr_in serverToConnect,socklen_t serverToConnectAddr){
    // CURL *curl;
    // CURLcode res;
    // fstream file;
    // file.open("config.txt",std::fstream::app);
    // // ///GET DATA

    string data = "";

    ;
    int dataSize = strlen(data.c_str());
    this->invocation_id++;
    string getDataCommand =  "get " + userName + "," + filePath + "\r\n";//;" 10\r\n";
     cout<<"data size is"<<dataSize<<endl;
     cout<<"command sent is: "<<getDataCommand<<endl;
    // int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    // struct sockaddr_in keyValueStore;
    // keyValueStore.sin_family = PF_INET;
    // keyValueStore.sin_port = htons(5001);
    // keyValueStore.sin_addr.s_addr = htons(INADDR_ANY);
    // inet_pton(PF_INET, "127.0.0.1", &(keyValueStore.sin_addr));
    // socklen_t keyValueStoreAddr;
    int sockfd = socket(PF_INET, SOCK_STREAM, 0);

    bind(sockfd, (struct sockaddr *) &this->storage, sizeof(this->storage));
    if (connect(sockfd, (struct sockaddr*)&this->storage, sizeof(this->storage)) < 0) {
         perror("ERROR connecting");
         exit(1);
    }
    else{
        cout<<"connection successful"<<endl;
    }

    int numBytesWrite = write(sockfd, "yo\r\n", strlen("yo\r\n"));


     int found = string::npos,
         numBytesRead;
     string input = "";
     char buffer[10000];
     while(found == string::npos){
         numBytesRead = read(sockfd, buffer,10000);
         if(numBytesRead > 0) {
             input += string(buffer);
             found = input.find("+OK");
             cout<<input<<endl;
         }
    }
    bzero(buffer,10000);
    found = string::npos;
    input = "";
    numBytesWrite = write(sockfd, getDataCommand.c_str(), strlen(getDataCommand.c_str()));

//    while(found == string::npos){
//        //numBytesWrite = write(sockfd, data.c_str(), strlen(data.c_str()));
//        numBytesRead = read(sockfd, buffer,1000);
//        if(numBytesRead > 0){
//            input += string(buffer);
//            found = input.find("+OK");
//            //found = string(buffer).find("+OK");
//            cout<<input<<endl;
//        }
//    }

    found = string::npos;
    bzero(buffer,10000);
    //unsigned char downBuff[256];
    input = "";
    //numBytesWrite = write(sockfd, data.c_str(), strlen(data.c_str()));
    while(found == string::npos){
        numBytesRead = read(sockfd,buffer,10000);
        if(numBytesRead > 0){
            input += string(buffer);
            found = input.find("<EOB>");
            bzero(buffer,10000);
            //found = string(buffer).find("+OK");
            cout<<found<<endl;
        }
    }
    cout<<"Data received is : "<<input<<endl;
    input = input.substr(0,found);
    ///convert to text
    //std::string data = "01110100011001010111001101110100";
    std::stringstream sstream(input);
    std::string output;
    while(sstream.good())
    {
        std::bitset<8> bits;
        sstream >> bits;
        char c = char(bits.to_ulong());
        output += c;
    }

    cout<<"converted back data is:"<<output<<endl;




    /////




    output = output + "<EOF>";
    cout<<"\n\ndata sent to frontend    : "<<output<<endl;
    int num = write(serverSocket,output.c_str(),strlen(output.c_str()));
    //cout<<input<<endl;
    // string data = input.substr(0,found - 1);
    // input = input.substr(found - 1);


    // // ////// GET DATA
    // curl_global_init(CURL_GLOBAL_ALL);

    // /* get a curl handle */
    // curl = curl_easy_init();
    // if(curl) {

    //     //const char *data = "data to send data to send data";
    //     //curl_easy_setopt(curl, CURLOPT_URL, "http://postit.example.com/moo.cgi");

    //     char* server;
    //     inet_ntop(AF_INET, &(serverToConnect.sin_addr), server, INET_ADDRSTRLEN);

    //     curl_easy_setopt(curl, CURLOPT_URL, server);
    //     /* Now specify the POST data */
    //     curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long) strlen(data.c_str()));
    //     curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());

    //     /* Perform the request, res will get the return code */
    //     res = curl_easy_perform(curl);
    //     /* Check for errors */
    //     if(res != CURLE_OK)
    //         fprintf(stderr, "curl_easy_perform() failed: %s\n",
    //                 curl_easy_strerror(res));

    //     /* always cleanup */
    //     curl_easy_cleanup(curl);
    //     //cout<<“abcd success”;
    // }
    // curl_global_cleanup();
    cout<<"in handle download"<<dataSize<<endl;

    close(sockfd);
}

 void User::handleContents(int serverSocket,string userName,string folderPath){
     ifstream file;
     string filename = userName + "listOfFolders.txt";
     cout<<"handle contents func";
     file.open(filename.c_str());
     string line;
     string contents = "";
     int folderFoundFlag = 0;
     while (std::getline(file, line))
     {
         //std::istringstream iss(line);
         //cout<<"line from file : "<<line<<endl;
         if(strcmp(folderPath.c_str(),line.c_str()) == 0){
             contents = "";
             folderFoundFlag = 1;
         }
         else if(strcmp(line.c_str(),"###") == 0 && folderFoundFlag){
             cout<<"folder contents is : "<<contents<<endl;
             contents = "#Folder#"+folderPath + "#" + contents + "#End#";

             folderFoundFlag = 0;
         }
         else if(folderFoundFlag){
             if(strcmp(contents.c_str(),"") == 0){
                 contents = contents + line;
             }
             else{
                 contents = contents + "#" + line;
             }

         }
         //cout<<line<<endl;
     }

     cout<<"contents of the folder is "<<contents<<endl;
     if(strcmp(contents.c_str(),"") == 0){
         contents = "#Folder#"+folderPath + "#" + "#End#";
     }
     int num = write(serverSocket,contents.c_str(),strlen(contents.c_str()));

 }

void User::handleAddFolder(int serverSocket,string userName,string folderPath,string folderName){
     cout<<"in function"<<endl;
     ifstream file;
     string filename = userName + "listOfFolders.txt";
     file.open(filename.c_str());
     string line;
     //string reply = "#AddComplete# + username=" + userName + "path=" + folderPath + "foldername="+folderName + "#End#";
     string reply = "#AddComplete#\r\n";
     int folderFoundFlag = 0;


     vector<string> filecontents;
     while(std::getline(file, line)){
         filecontents.push_back(line);
         cout<<line<<endl;
     }
     file.close();
     ofstream outfile;
     outfile.open(filename.c_str(), std::ofstream::out | std::ofstream::trunc);
     outfile.close();
     outfile.open(filename.c_str());
     for(unsigned i = 0;i<filecontents.size();i++){
         cout<<filecontents[i]<<endl;
         if(strcmp(filecontents[i].c_str(),folderPath.c_str()) == 0){
             outfile << filecontents[i]<<"\n";
             outfile << folderName <<"\n";
             folderFoundFlag = 1;
         }
         else if(folderFoundFlag == 1 && strcmp(filecontents[i].c_str(),"###") == 0){
             folderFoundFlag = 0;
             outfile << filecontents[i]<<"\n";
             outfile << folderPath + folderName <<"\n";
             outfile << "###" <<"\n";
         }
         else{
             outfile << filecontents[i]<<"\n";
         }
     }
     outfile.close();
     cout<<"reply sent is : "<<reply<<endl;
     int num = write(serverSocket,reply.c_str(),strlen(reply.c_str()));
}

void User::handleMoveFile(int serverSocket,string userName,string folderPath,string newFolderPath,string fileName){
    /*int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    string filen = newFolderPath + fileName;
    bind(sockfd, (struct sockaddr *) &this->storage, sizeof(this->storage));
    if (connect(sockfd, (struct sockaddr*)&this->storage, sizeof(this->storage)) < 0) {
        perror("ERROR connecting");
        exit(1);
    }
    else{
        cout<<"connection successful"<<endl;
    }

    int numBytesWrite = write(sockfd, "yo\r\n", strlen("yo\r\n"));


    int found = string::npos,
            numBytesRead;
    string input = "";
    char buffer[10000];
    while(found == string::npos){
        numBytesRead = read(sockfd, buffer,10000);
        if(numBytesRead > 0) {
            input += string(buffer);
            found = input.find("+OK");
            cout<<input<<endl;
        }
    }
    bzero(buffer,10000);
    found = string::npos;
    input = "";
    string getDataCommand = "get " + userName + "," + folderPath + fileName + "\r\n";
    int eobfound = string::npos;
    while(eobfound == string::npos){
        input += string(buffer);
    }

    std::stringstream sstream(input);
    std::string output;
    while(sstream.good())
    {
        std::bitset<8> bits;
        sstream >> bits;
        char c = char(bits.to_ulong());
        output += c;
    }



    string putDataCommand =  "put " + userName + "," + filen +  " " + std::to_string(strlen(filen.c_str())) + " " +std::to_string(this->invocation_id++) + "\r\n";//;" 10\r\n";

    numBytesWrite = write(sockfd, putDataCommand.c_str(), strlen(putDataCommand.c_str()));
    int fund = string::npos, funda = string::npos;
    while(fund == string::npos){
        int numb = read(sockfd,buffer,10000);
        fund = string(buffer).find("+OK");
        funda = string(buffer).find("-ERR");
        if(funda != string::npos){
            numBytesWrite = write(sockfd, putDataCommand.c_str(), strlen(putDataCommand.c_str()));
        }
    }

    write(sockfd,output.c_str(),strlen(output.c_str()));*/


     ifstream file;
     string filename = userName + "listOfFolders.txt";
     file.open(filename.c_str());
     string line;
     string reply = "#MoveComplete#\r\n";
     int folderFoundFlag = 0,
             newFolderFlag = 0;

     vector<string> filecontents;
     while(std::getline(file, line)){
         filecontents.push_back(line);
     }
     file.close();
     ofstream outfile;

     outfile.open(filename.c_str(), std::ofstream::out | std::ofstream::trunc);
     outfile.close();
     outfile.open(filename.c_str());
     for(unsigned i = 0;i<filecontents.size();i++){
         if(strcmp(filecontents[i].c_str(),folderPath.c_str()) == 0){
             folderFoundFlag = 1;
             outfile << filecontents[i]<<"\n";
             //outfile << fileName <<"\n";
         }
         else if(folderFoundFlag){
             if(strcmp(filecontents[i].c_str(),fileName.c_str()) == 0){
                 //outfile << "";
             }
             else if(strcmp(filecontents[i].c_str(),"###") == 0){
                 folderFoundFlag = 0;
                 outfile << filecontents[i]<<"\n";
             }
             else{
                 outfile << filecontents[i]<<"\n";
             }
         }
         else if(strcmp(filecontents[i].c_str(),newFolderPath.c_str()) == 0){
             newFolderFlag = 1;
             outfile << filecontents[i]<<"\n";
         }
         else if(newFolderFlag){
             outfile << fileName<<"\n";
             newFolderFlag = 0;
             outfile << filecontents[i]<<"\n";
         }
         else{
             outfile << filecontents[i]<<"\n";
         }
     }
     outfile.close();
    cout<<"reply sent is "<<reply<<endl;
    int num = write(serverSocket,reply.c_str(),strlen(reply.c_str()));
}

void User::handleDeleteFolder(int serverSocket,string userName,string folderPath,string folderName){
     ifstream file;
     string filename = userName + "listOfFolders.txt";
     file.open(filename.c_str());
     string command = "";
     string line;
     string reply = "#DeleteComplete#\r\n";
     int folderFoundFlag = 0,
             newFolderFlag = 0;

    int sockfd = socket(PF_INET, SOCK_STREAM, 0);

    bind(sockfd, (struct sockaddr *) &this->storage, sizeof(this->storage));
    if (connect(sockfd, (struct sockaddr*)&this->storage, sizeof(this->storage)) < 0) {
        perror("ERROR connecting");
        exit(1);
    }
    else{
        cout<<"connection successful"<<endl;
    }

    int numBytesWrite = write(sockfd, "yo\r\n", strlen("yo\r\n"));


    int found = string::npos,
            numBytesRead;
    string input = "";
    char buffer[1000];
    while(found == string::npos){
        numBytesRead = read(sockfd, buffer,1000);
        if(numBytesRead > 0) {
            input += string(buffer);
            found = input.find("+OK");
            cout<<input<<endl;
        }
    }
    bzero(buffer,1000);
    found = string::npos;
    input = "";

     vector<string> filecontents;
     while(std::getline(file, line)){
         filecontents.push_back(line);
     }
     file.close();
     ofstream outfile;

     outfile.open(filename.c_str(), std::ofstream::out | std::ofstream::trunc);
     outfile.close();
     outfile.open(filename.c_str());
     for(unsigned i = 0;i<filecontents.size();i++){
         if(strcmp(filecontents[i].c_str(),folderPath.c_str()) == 0){
             folderFoundFlag = 1;
             outfile << filecontents[i]<<"\n";
             //outfile << fileName <<"\n";
         }
         else if(folderFoundFlag){
             if(strcmp(filecontents[i].c_str(),folderName.c_str()) == 0){
                 //outfile << "";
                 this->invocation_id = this->invocation_id + 1;
                 command = "del " + userName + "," + folderPath + folderName + " " + std::to_string(this->invocation_id) + "\r\n";
                 int key = write(sockfd,command.c_str(),strlen(command.c_str()));
                 cout<<"command sent is : "<<command<<endl;
             }
             else if(strcmp(filecontents[i].c_str(),"###") == 0){
                 folderFoundFlag = 0;
                 outfile << filecontents[i]<<"\n";
             }
             else{
                 outfile << filecontents[i]<<"\n";
             }
         }
         else if(strcmp(filecontents[i].c_str(),(folderPath+folderName).c_str()) == 0){
             newFolderFlag = 1;
             //outfile << filecontents[i]<<"\n";
         }
         else if(newFolderFlag){
             if(strcmp(filecontents[i].c_str(),"###") == 0){
                 newFolderFlag = 0;
             }
         }
         else{
             outfile << filecontents[i]<<"\n";
         }
     }
     outfile.close();
    cout<<"reply is :"<<reply<<endl;
    int num = write(serverSocket,reply.c_str(),strlen(reply.c_str()));
}

void User::handleDeleteFile(int serverSocket,string userName,string folderPath,string fileName){
     ifstream file;

     string filename = userName + "listOfFolders.txt";
     file.open(filename.c_str());
     string line;
     string reply = "#DeleteFileComplete#\r\n";
     int folderFoundFlag = 0,
             newFolderFlag = 0;
     vector<string> filecontents;
     while(std::getline(file, line)){
         filecontents.push_back(line);
     }
     file.close();
     ofstream outfile;

     outfile.open(filename.c_str(), std::ofstream::out | std::ofstream::trunc);
     outfile.close();
     outfile.open(filename.c_str());
     for(unsigned i = 0;i<filecontents.size();i++){
         if(strcmp(filecontents[i].c_str(),folderPath.c_str()) == 0){
             folderFoundFlag = 1;
             outfile << filecontents[i]<<"\n";
             //outfile << fileName <<"\n";
         }
         else if(folderFoundFlag){
             if(strcmp(filecontents[i].c_str(),fileName.c_str()) == 0){
                 //outfile << "";
             }
             else if(strcmp(filecontents[i].c_str(),"###") == 0){
                 folderFoundFlag = 0;
                 outfile << filecontents[i]<<"\n";
             }
             else{
                 outfile << filecontents[i]<<"\n";
             }
         }
         else{
             outfile << filecontents[i]<<"\n";
         }
     }
     outfile.close();
     cout<<"reply is "<<reply<<endl;
     int num = write(serverSocket,reply.c_str(),strlen(reply.c_str()));
}