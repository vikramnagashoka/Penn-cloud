#include<iostream>
#include<fstream>
#include<vector>
using namespace std;
int main(){
    string userName = "vikram";
    ifstream file;
     string filename = userName + "listOfFolders.txt";
     file.open(filename.c_str());
     string line;
     string reply = "#DeleteComplete#\r\n";
     string folderPath = "/root/abc/";
     string folderName = "mnop/";
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
             if(strcmp(filecontents[i].c_str(),folderName.c_str()) == 0){
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

     
     }
