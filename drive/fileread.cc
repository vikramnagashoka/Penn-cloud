#include<iostream>
#include<fstream>
#include<string>
#include <fstream>
//#include<ofstream>
using namespace std;
int main(){
	//string ifs1 = "abcdefgh\r\n";
	 ifstream ifs1;
	 ifs1.open("abc.txt");
	 ofstream ofs1;
	 ofs1.open("abcdef.bin");	
	// string line= "";
	// while(getline(ifs1,line)){
	// 	ofs1 << line;
	// 	ofs1 << "\n";
	// }

	// //for(auto it = std::istream_iterator<char>(ifs1); it != std::istream_iterator<char>(); ++it)
 //    //ofs1 << std::bitset<CHAR_BIT>(*it) << std::endl; // CHAR_BIT is defined in <climits>
	
	// ofs1.close();
	string myString = "#";
	string oneBigString = "";
	string myString1 = "Bye bye world\r\n";
	string myString2 = "Bye bye world\r\n";

  std::string binary_outputInformations;
  std::string binary_outputInformations1;
  std::string binary_outputInformations2;
  for (std::size_t i = 0; i < myString.size(); ++i)
  {
  bitset<8> b(myString.c_str()[i]);
      binary_outputInformations+= b.to_string();
  }

  for(std::size_t i = 0; i < myString1.size(); ++i){
  	bitset<8> b1(myString1.c_str()[i]);
      binary_outputInformations1+= b1.to_string();
    bitset<8> b2(myString2.c_str()[i]);
      binary_outputInformations2+= b2.to_string();    	    	
  }

  std::cout<<binary_outputInformations;
  std::cout<<"\nstr length is :"<<strlen(binary_outputInformations.c_str())<<endl;
  std::cout<<binary_outputInformations1;
  std::cout<<"\nstr length is :"<<strlen(binary_outputInformations1.c_str())<<endl;



  for(auto it = std::istream_iterator<std::string>(ifs1); it != std::istream_iterator<std::string>(); ++it)
    ofs1 << static_cast<char>(std::bitset<CHAR_BIT>(*it).to_ulong());


	return 0;
}