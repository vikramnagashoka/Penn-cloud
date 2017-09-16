#include <string>
#include <vector>
#include <string.h>
#include <sstream>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <iostream>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <algorithm>
#include <fstream>
#include <map>
#include <bitset>

#include <dirent.h>
#include <sys/types.h>
using namespace std;

// GLOBAL VARIABLES
vector<string> usernames;
string inbox_list;
string sent_list;

// Keeps track of current folder
string current_folder;

// List of threads and sockets
vector<pthread_t> threads;
vector<int> comm_fd;
// Keeps track of number of threads
int numThreads = 0;

// Counter to send to storage
int counter = 1;

// Flag for debugging
int vflag = 0;

// Writes back to client (from Lecture 6 slides)
bool do_write(int fd, char *buf, int bufsize)
{
	int sent = 0;
	while (sent < bufsize-1)
	{
		int n = write(fd, &buf[sent], bufsize-sent-1);
		if (n < 0)
		{
			return false;
		}
		sent += n;
	}
	return true;
}

// Reads input from client
string do_read(int fd, char *buf, int bufsize)
{
	int n = 0;
	while(n == 0)
	{
		n = read(fd, buf, bufsize);
	}

	string current_line = buf;
	return current_line;
}

// Convert a string to a string of 0's and 1's
string string_to_binary(string str)
{
	// Gets each character by splitting command
	stringstream stream(str);
	string seg;
	string new_str = "";

	while (getline(stream, seg, '\n'))
	{
		if (new_str == "")
		{
			new_str = seg;
		}
		else
		{
			new_str = new_str + seg;
		}
	}

	string binary = "";
	for (size_t i = 0; i < new_str.size(); ++i)
	{
		if (binary == "")
		{
			binary = binary + bitset<8>(new_str.c_str()[i]).to_string();
		}
		else
		{
			binary = binary + " " + bitset<8>(new_str.c_str()[i]).to_string();
		}
	}

	return binary;
}

string binary_to_string(string binary)
{
	// Gets each byte by splitting command
	stringstream stream(binary);
	string seg;
	vector<string> seglist;

	while (getline(stream, seg, ' '))
	{
		seglist.push_back(seg);
	}

	string str = "";

	for (int i = 0; i < seglist.size(); ++i)
	{
		char c = strtol(seglist.at(i).c_str(), 0, 2);

		if (c == '\r')
		{
			str = str + "\r\n";
		}
		else
		{
			str = str + c;
		}
	}

	return str;
}



// Sends GET from storage
string get(int cfd, string username, string current_folder)
{
	string get = "get " + username + "," + current_folder + "\r\n";
	char *line = new char[get.size() + 1];
	memcpy(line, get.c_str(), get.size() + 1);
	do_write(cfd, line, get.size() + 1);

	if (vflag == 1)
	{
		fprintf(stderr, "[%d] Web_to_Storage: %s", cfd, get.c_str());
	}

	char buf[1024] = {0};
	string temp = do_read(cfd, buf, sizeof(buf));

	//cout << "READ: " << o << endl;
	string o = temp;

	if (0 != temp.find("-ERR"))
	{
		o.pop_back();
		o.pop_back();
		o.pop_back();
		o.pop_back();
		o.pop_back();
		o = binary_to_string(o);
		o = o + "\r\n";
	}

//	string o = "";
//	for (int i = 0; i < output.size(); i++)
//	{
//		o = o + output.at(i);
//	}

	if (vflag == 1)
	{
		fprintf(stderr, "[%d] Storage: %s\r\n", cfd, temp.c_str());
		fprintf(stderr, "[%d] Storage: %s", cfd, o.c_str());
	}



	return o;
}

void cput(int cfd, string username, string header, string v, string ov)
{
	ov.pop_back();
	ov.pop_back();

	string old_value = string_to_binary(ov);
	string value = string_to_binary(v);

	int size = old_value.size();
	int size2 = value.size();
	string cput = "cput " + username + "," + header + " " + to_string(counter) + "\r\n";

	// Sends put command to server (with only size of email)
	char *line = new char[cput.size() + 1];
	memcpy(line, cput.c_str(), cput.size() + 1);
	do_write(cfd, line, cput.size() + 1);

	counter += 1;

	if (vflag == 1)
	{
		fprintf(stderr, "[%d] Web_to_Storage: %s", cfd, cput.c_str());
	}

	// Reads back message from server
	char buf[1024] = {0};
	string o = do_read(cfd, buf, sizeof(buf));

	if (vflag == 1)
	{
		fprintf(stderr, "[%d] Storage: %s", cfd, o.c_str());
	}

	if (0 == o.find("+OK"))
	{
		// If server message is +OK, send old value size and old value
		string oldval = "oldval " + to_string(size) + "\r\n";
		char *line2 = new char[oldval.size() + 1];
		memcpy(line2, oldval.c_str(), oldval.size() + 1);
		do_write(cfd, line2, oldval.size() + 1);

		if (vflag == 1)
		{
			fprintf(stderr, "[%d] Web_to_Storage: %s", cfd, oldval.c_str());
		}

		char *line3 = new char[old_value.size() + 1];
		memcpy(line3, old_value.c_str(), old_value.size() + 1);
		do_write(cfd, line3, old_value.size() + 1);

		if (vflag == 1)
		{
			fprintf(stderr, "[%d] Web_to_Storage: %s\r\n", cfd, ov.c_str());
			fprintf(stderr, "[%d] Web_to_Storage (test): %s\r\n", cfd, old_value.c_str());
		}

		char buf2[1024] = {0};
		string o2 = do_read(cfd, buf2, sizeof(buf2));

		if (vflag == 1)
		{
			fprintf(stderr, "[%d] Storage: %s", cfd, o2.c_str());
		}

		if (0 == o2.find("+OK"))
		{
			string new_val = "newval " + to_string(size2) + "\r\n";
			char *line4 = new char[new_val.size() + 1];
			memcpy(line4, new_val.c_str(), new_val.size() + 1);
			do_write(cfd, line4, new_val.size() + 1);

			if (vflag == 1)
			{
				fprintf(stderr, "[%d] Web_to_Storage: %s", cfd, new_val.c_str());
			}

			char *line5 = new char[value.size() + 1];
			memcpy(line5, value.c_str(), value.size() + 1);
			do_write(cfd, line5, value.size() + 1);

			if (vflag == 1)
			{
				fprintf(stderr, "[%d] Web_to_Storage: %s\r\n", cfd, v.c_str());
				fprintf(stderr, "[%d] Web_to_Storage (test): %s\r\n", cfd, value.c_str());
			}

			char buf3[1024] = {0};
			string o3 = do_read(cfd, buf3, sizeof(buf3));

			if (vflag == 1)
			{
				fprintf(stderr, "[%d] Storage: %s", cfd, o3.c_str());
			}
		}
	}
}

// Sends PUT to storage
string put(int cfd, string username, string header, string l)
{
	string listy = string_to_binary(l);
	int size = listy.size();
	string put = "put " + username + "," + header + " " + to_string(size) + " " + to_string(counter) + "\r\n";


	// Sends put command to server (with only size of email)
	char *line = new char[put.size() + 1];
	memcpy(line, put.c_str(), put.size() + 1);
	do_write(cfd, line, put.size() + 1);

	counter += 1;

	if (vflag == 1)
	{
		fprintf(stderr, "[%d] Web_to_Storage: %s", cfd, put.c_str());
	}

	// Reads back message from server
	char buf[1024] = {0};
	string o = do_read(cfd, buf, sizeof(buf));

//	string o = "";
//	for (int i = 0; i < output.size(); i++)
//	{
//		o = o + output.at(i);
//	}

	if (vflag == 1)
	{
		fprintf(stderr, "[%d] Storage: %s", cfd, o.c_str());
	}

	// If server message is +OK shoot, send text of email,
	// or updated email list

	if (0 == o.find("+OK"))
	{
		char *line2 = new char[listy.size() + 1];
		memcpy(line2, listy.c_str(), listy.size() + 1);
		do_write(cfd, line2, listy.size() + 1);

		if (vflag == 1)
		{
			fprintf(stderr, "[%d] Web_to_Storage: %s\r\n", cfd, l.c_str());
			fprintf(stderr, "[%d] Web_to_Storage (test): %s\r\n", cfd, listy.c_str());
		}

		char buf2[1024] = {0};
		string o2 = do_read(cfd, buf2, sizeof(buf2));

		if (vflag == 1)
		{
			fprintf(stderr, "[%d] Storage: %s", cfd, o2.c_str());
		}

	}
}

string get_username(string segment)
{
	// Gets username by splitting command
	stringstream stream(segment);
	string seg, username;
	string folder = "";
	int i = 0;

	//cout << segment << endl;

	i == 0;
	while (getline(stream, seg, '@'))
	{
		if (i == 0)
		{
			username = seg;
		}
		i += 1;
	}

	return username;
}

// Used by each thread
void *worker(void *arg)
{
	int cfd = *(int*)arg;

	// Reads in a line from the client
	char ibuf[1024] = {0};
	char ibuf2[1024] = {0};
	string a = do_read(cfd, ibuf, sizeof(ibuf));

	if (vflag == 1)
	{
		fprintf(stderr, "[%d] Frontend: (not used) %s", cfd, a.c_str());
	}

	string ignore_line = "+OK Ignored\r\n";
	char *ignore = new char[ignore_line.size() + 1];
	memcpy(ignore, ignore_line.c_str(), ignore_line.size() + 1);
	do_write(cfd, ignore, ignore_line.size() + 1);

	if (vflag == 1)
	{
		fprintf(stderr, "[%d] Web_to_front: %s", cfd, ignore_line.c_str());
	}

	string user = do_read(cfd, ibuf2, sizeof(ibuf2));
	if (vflag == 1)
	{
		fprintf(stderr, "[%d] Frontend: %s", cfd, user.c_str());
	}
	user.pop_back();
	user.pop_back();

	// Prints out intro message
	string intro_line = ("+OK Mailserver Ready\r\n");
	char *intro = new char[intro_line.size() + 1];
	memcpy(intro, intro_line.c_str(), intro_line.size() + 1);
	do_write(cfd, intro, intro_line.size() + 1);

	if (vflag == 1)
	{
		fprintf(stderr, "[%d] Web_to_front: %s", cfd, intro_line.c_str());
	}

	string current_line;

	// This socket is to connect to storage master server (UDP)
	int listen_master = socket(PF_INET, SOCK_DGRAM, 0);
	// Master port is 5000
	int port2 = 5000;
	struct sockaddr_in dest;
	bzero(&dest, sizeof(dest));
	dest.sin_family = AF_INET;
	dest.sin_port = htons(port2);
	inet_pton(AF_INET, "127.0.0.1:5000", &(dest.sin_addr));

//	// Builds structure needed to use sockets
//	struct sockaddr_in servaddr2;
//	bzero(&servaddr2, sizeof(servaddr2));
//  servaddr2.sin_family = AF_INET;
//	servaddr2.sin_addr.s_addr = htons(INADDR_ANY);
//	servaddr2.sin_port = htons(port2);
//	bind(listen_master, (struct sockaddr*) &servaddr2, sizeof(servaddr2));

	// Connects to storage, and listens for new connection
	//listen(listen_master, 10);

	// Initializes a new socket
	//struct sockaddr_in clientaddr2;
	//socklen_t clientaddrlen2 = sizeof(clientaddr2);
	//int comm_fd2 = accept(listen_fd2, (struct sockaddr*) &clientaddr2, &clientaddrlen2);

	// Sends intro message to master storage
	string storage_intro = "query " + user + "\r\n";
	sendto(listen_master, storage_intro.c_str(), strlen(storage_intro.c_str()), 0, (struct sockaddr*) &dest, sizeof(dest));
	//do_write(listen_master, intro2, storage_intro.size() + 1);

	if (vflag == 1)
	{
		fprintf(stderr, "[%d] Web_To_Master: %s", listen_master, storage_intro.c_str());
	}

	// Reads back message from master server
	char intro_buf[1024] = {0};
	// Receives response from server
	struct sockaddr_in src;
	socklen_t srcSize = sizeof(src);
	int rlen = recvfrom(listen_master, intro_buf, sizeof(intro_buf), 0, (struct sockaddr*) &src, &srcSize);
	//vector<string> output = do_read(listen_master, intro_buf, sizeof(intro_buf));
	string intro_response(intro_buf);

	if (vflag == 1)
	{
		fprintf(stderr, "[%d] Storage: %s\r\n", listen_master, intro_response.c_str());
	}

	// Gets port by splitting command
	stringstream streamy(intro_response);
	string segy;
	vector<string> seglisty;

	while (getline(streamy, segy, ':'))
	{
		seglisty.push_back(segy);
	}

	// This socket is to connect to storage server
	int listen_fd2 = socket(PF_INET, SOCK_STREAM, 0);
	// Server port is received from master
	int server_port = atoi(seglisty[1].c_str());
	// Builds structure needed to use sockets
	struct sockaddr_in servaddr3;
	bzero(&servaddr3, sizeof(servaddr3));
	servaddr3.sin_family = AF_INET;
	servaddr3.sin_addr.s_addr = htons(INADDR_ANY);
	servaddr3.sin_port = htons(server_port);
	bind(listen_fd2, (struct sockaddr*) &servaddr3, sizeof(servaddr3));

	if (connect(listen_fd2, (struct sockaddr*) &servaddr3, sizeof(servaddr3)) < 0)
	{
		cout << "Error connecting";
		exit(1);
	}

	// Sends intro message to storage server
	string storage_intro2 = "yo\r\n";
	char *liney = new char[storage_intro2.size() + 1];
	memcpy(liney, storage_intro2.c_str(), storage_intro2.size() + 1);
	do_write(listen_fd2, liney, storage_intro2.size() + 1);

	if (vflag == 1)
	{
		fprintf(stderr, "[%d] Web_to_Storage: %s", listen_fd2, storage_intro2.c_str());
	}

	char buf[1024] = {0};
	string o = do_read(listen_fd2, buf, sizeof(buf));

//	string o = "";
//	for (int i = 0; i < output.size(); i++)
//	{
//		o = o + output.at(i);
//	}

	if (vflag == 1)
	{
		fprintf(stderr, "[%d] Storage: %s", cfd, o.c_str());
	}

	// Need to initialize inbox and sent, if not already done so.
	string test_inbox = get(listen_fd2, user, "INBOX");

	if (0 == test_inbox.find("-ERR"))
	{
		//cout << "STRING: " << str << endl;
		//cout << "BINARY: " << bin << endl;
		//cout << "STRING: " << str2 << endl;

		put(listen_fd2, user, "INBOX", ".");
		put(listen_fd2, user, "SENT", ".");

		inbox_list = ".";
		sent_list = ".";
	}

	cout << endl;


	while (true)
	{
		// Reads in a line from the client
		char buf[1024] = {0};
		string current_line = do_read(cfd, buf, sizeof(buf));

//		string current_line = "";
//		for (int j = 0; j < lines.size(); ++j)
//		{
//			if (j == lines.size()-1)
//			{
//				current_line = current_line + line.at(j);
//			}
//			else
//			{
//				current_line = current_line + line.at(j) + "\r\n";
//			}
//		}

			// doread can read in multiple lines
			// Appends the last part of the previous line to the first line,
			// if there is anything
			//current_line = next_line + lines.at(j);
			//next_line = "";

			// Save the lower-case version of the current line
			// (commands are case-insensitive)
			string lower = current_line;
			transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

			// If we get the command: "GET_INBOX~linhphan":
			if (0 == lower.find("get_inbox"))
			{
				if (vflag == 1)
				{
					fprintf(stderr, "[%d] Frontend: %s", cfd, current_line.c_str());
				}
				// Sets current folder:
				current_folder = "INBOX";

				// Gets username by splitting command
				stringstream stream(current_line);
				string seg;
				string username;

				int i = 0;
				while (getline(stream, seg, '~'))
				{
					if (i == 1)
					{
						username = seg;
					}
					i += 1;
				}

				username.pop_back();
				username.pop_back();

				// Extract emails from INBOX
				string emails = get(listen_fd2, username, current_folder);

				// Sends email list to frontend server
				char *ems = new char[emails.size() + 1];
				memcpy(ems, emails.c_str(), emails.size() + 1);
				do_write(cfd, ems, emails.size() + 1);

				if (vflag == 1)
				{
					fprintf(stderr, "[%d] Web_to_Front: %s", cfd, emails.c_str());
				}
			}
			// If we get the command: "GET_SENT~linhphan":
			else if (0 == lower.find("get_sent"))
			{
				if (vflag == 1)
				{
					fprintf(stderr, "[%d] Frontend: %s", cfd, current_line.c_str());
				}

				// Sets current folder:
				current_folder = "SENT";

				// Gets username by splitting command
				stringstream stream(current_line);
				string seg;
				string username;

				int i = 0;
				while (getline(stream, seg, '~'))
				{
					if (i == 1)
					{
						username = seg;
					}
					i += 1;
				}

				username.pop_back();
				username.pop_back();

				// Extract list of SENT emails from storage
				string emails = get(listen_fd2, username, current_folder);

				// Sends email list to frontend server
				char *ems = new char[emails.size() + 1];
				memcpy(ems, emails.c_str(), emails.size() + 1);
				do_write(cfd, ems, emails.size() + 1);

				if (vflag == 1)
				{
					fprintf(stderr, "[%d] Web_to_Front: %s", cfd, emails.c_str());
				}
			}
			// If we get the command: "GET_FOLDER~USERNAME~FOLDER"
			else if (0 == lower.find("get_folder"))
			{
				// Gets username by splitting command
				stringstream stream(current_line);
				string seg, username;
				string folder = "";
				int i = 0;

				while (getline(stream, seg, '~'))
				{
					if (i == 1)
					{
						username = seg;
					}
					else if (i >= 2)
					{
						folder += " " + seg;
					}
					i += 1;
				}

				current_folder = current_folder + "//" + folder;

				// Extract list of emails/folders from new folder
				string emails = get(listen_fd2, username, current_folder);

				// Sends email list to frontend server
				char *ems = new char[emails.size() + 1];
				memcpy(ems, emails.c_str(), emails.size() + 1);
				do_write(cfd, ems, emails.size() + 1);

				if (vflag == 1)
				{
					fprintf(stderr, "[%d] Frontend: %s\r\n", cfd, current_line.c_str());
					fprintf(stderr, "[%d] Web_to_Front: %s\r\n", cfd, emails.c_str());
				}
			}
			// If we get the command: "GET_EMAIL~USERNAME~HEADER":
			else if (0 == lower.find("get_email"))
			{
				if (vflag == 1)
				{
					fprintf(stderr, "[%d] Frontend: %s", cfd, current_line.c_str());
				}

				// Gets username by splitting command
				stringstream stream(current_line);
				string seg, username;
				string header = "";
				int i = 0;

				while (getline(stream, seg, '~'))
				{
					if (i == 1)
					{
						username = seg;
					}
					else if (i == 2)
					{
						header = seg;
					}
					i += 1;
				}

				char c = header.back();
				if (c == '\n')
				{
					header.pop_back();
					header.pop_back();
				}

				// Extract email text from storage
				string email = get(listen_fd2, username, header);

				char *ems = new char[email.size() + 1];
				memcpy(ems, email.c_str(), email.size() + 1);
				do_write(cfd, ems, email.size() + 1);

				if (vflag == 1)
				{
					fprintf(stderr, "[%d] Web_to_Front: %s", cfd, email.c_str());
				}
			}
			// If we get the command: "SEND_EMAIL~SENDER~RECEIVER~HEADER~TEXT"
			else if (0 == lower.find("send_email"))
			{
				if (vflag == 1)
				{
					fprintf(stderr, "[%d] Frontend: %s\r\n", cfd, current_line.c_str());
				}

				// Gets username by splitting command
				stringstream stream(current_line);
				string seg, sender, receiver, header, username;
				string text = "";

				int i = 0;
				while (getline(stream, seg, '~'))
				{
					if (i == 1)
					{
						sender = get_username(seg);
						text = sender;
					}
					else if (i == 2)
					{
						receiver = get_username(seg);
						text = text + "~" + receiver;
					}
					else if (i == 3)
					{
						header = seg;
					}
					else if (i >= 4)
					{
						text = text + "~" + seg;
					}
					i += 1;
				}

				char c = text.back();
				if (c == '\n')
				{
					text.pop_back();
					text.pop_back();
				}

				// Check if sender has an account
				string sent_emails = get(listen_fd2, sender, "SENT");

				// If sender has account, put email into sent folder
				if (0 != sent_emails.find("-ERR"))
				{
					string new_sent = "";
					if (0 == sent_emails.find("."))
					{
						new_sent = header;
					}
					else
					{
						new_sent = header + "\r\n" + sent_emails;
						new_sent.pop_back();
						new_sent.pop_back();
					}
					cput(listen_fd2, sender, "SENT", new_sent, sent_emails);
					put(listen_fd2, sender, header, text);
				}

				// Check if receiver has an account
				string inbox_emails = get(listen_fd2, receiver, "INBOX");

				// If sender has account, put email into sent folder
				if (0 != inbox_emails.find("-ERR"))
				{
					string new_inbox;
					if (0 == inbox_emails.find("."))
					{
						new_inbox = header;
					}
					else
					{
						new_inbox = header + "\r\n" + inbox_emails;
						new_inbox.pop_back();
						new_inbox.pop_back();
					}
					cput(listen_fd2, receiver, "INBOX", new_inbox, inbox_emails);
					put(listen_fd2, receiver, header, text);
				}

				string status = "+OK Email Sent\r\n";
				char *ems = new char[status.size() + 1];
				memcpy(ems, status.c_str(), status.size() + 1);
				do_write(cfd, ems, status.size() + 1);

				if (vflag == 1)
				{
					fprintf(stderr, "[%d] Web_to_Front: %s", cfd, status.c_str());
				}
			}
			// If we get the command: "MOVE_EMAIL~USERNAME~FOLDER~HEADER"
//			else if (0 == lower.find("move_email"))
//			{
//				// Gets info by splitting command
//				stringstream stream(current_line);
//				string seg, username, folder, header;
//
//				int i = 0;
//				while (getline(stream, seg, '~'))
//				{
//					if (i == 1)
//					{
//						username = seg;
//					}
//					else if (i == 2)
//					{
//						folder = seg;
//					}
//					else if (i == 3)
//					{
//						header = seg;
//					}
//					i += 1;
//				}
//
//				// Check if sender has an account
//				string get_current_folder = "GET(" + username + ", " + current_folder + ")";
//				string cur_fold;
//
//				string new_folder = current_folder + "//" + folder;
//				string get_new_folder = "GET(" + username + ", " + new_folder + ")";
//				string new_fold;
//
//				new_fold = header + "\r\n" + new_fold;
//				string put_new = "PUT(" + username + ", " + new_folder + ", " + new_fold + ")";
//
//				// Gets email headers by splitting command
//				stringstream stream2(cur_fold);
//				string seg2;
//				string updated_folder = "";
//
//				i = 0;
//				while (getline(stream2, seg, '\n'))
//				{
//					if (seg2 != header)
//					{
//						updated_folder = updated_folder + seg2;
//					}
//				}
//
//				string put_updated = "PUT(" + username + ", " + current_folder + ", " + updated_folder + ")";
//
//			}
			// If the line starts with QUIT, prints out exit messages,
			// closes socket, and exits thread
			else if (0 == lower.find("quit"))
			{
				if (vflag == 1)
				{
					fprintf(stderr, "[%d] Frontend: %s", cfd, current_line.c_str());
				}

				string quit_storage = "quit\r\n";
				char *quity = new char[quit_storage.size() + 1];
				memcpy(quity, quit_storage.c_str(), quit_storage.size()+1);
				do_write(listen_fd2, quity, quit_storage.size() + 1);

				string quit_line = ("+OK Goodbye!\r\n");
				char *quit = new char[quit_line.size() + 1];
				memcpy(quit, quit_line.c_str(), quit_line.size() + 1);
				do_write(cfd, quit, quit_line.size() + 1);

				if (vflag == 1)
				{
					fprintf(stderr, "[%d] Web_to_Front: +OK Goodbye!\r\n", cfd);
					fprintf(stderr, "[%d] Connection closed\r\n", cfd);
				}

				close(cfd);
				pthread_exit(NULL);
			}
			else
			{
				if (vflag == 1)
				{
					fprintf(stderr, "[%d] Frontend: %s\r\n", cfd, current_line.c_str());
				}

				string new_line = "-ERR: Unknown command\r\n";
				char *line = new char[new_line.size() + 1];
				memcpy(line, new_line.c_str(), new_line.size() + 1);
				do_write(cfd, line, new_line.size() + 1);

				if (vflag == 1)
				{
					fprintf(stderr, "[%d] Web_to_Front: %s", cfd, new_line.c_str());
				}
			}

			cout << endl;
	}
}

// When Control+C is pressed, print out shut down message for each open
// socket, close all sockets, and join all threads
void event_handler(int e)
{
	string new_line = "-ERR Server shutting down\r\n";
	char *line = new char[new_line.size() + 1];
	memcpy(line, new_line.c_str(), new_line.size() + 1);

	for (int c = 0; c < numThreads; ++c)
	{
		do_write(comm_fd.at(c), line, new_line.size() + 1);
		close(comm_fd.at(c));
		pthread_cancel(threads.at(c));
		pthread_join(threads.at(c), NULL);
	}

	exit(0);
}

// MAIN FUNCTION
int main(int argc, char *argv[])
{
	int listen_fd = socket(PF_INET, SOCK_STREAM, 0);
	// Default port is 6000
	int port = 6000;

	// Flags for command line parameters
	int pflag = 0;
	int c;

	// Listens for Control+C
	signal(SIGINT, event_handler);

	// Checks for any optional command line parameters
	while ((c = getopt (argc, argv, "p:v")) != -1)
	{
		// If -p is present, get the port number
		if (c == 'p')
		{
			pflag = 1;
			port = atoi(optarg);
		}
		else if (c == 'v')
		{
			vflag = 1;
		}
	}

	// Builds structure needed to use sockets (from Lecture 6 slides)
	struct sockaddr_in servaddr;
	bzero(&servaddr, sizeof(servaddr));
  	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htons(INADDR_ANY);
	servaddr.sin_port = htons(port);
	bind(listen_fd, (struct sockaddr*) &servaddr, sizeof(servaddr));

	// Connects to server, and listens for new connection
	listen(listen_fd, 10);


	while (true)
	{
		// Initializes a new socket
		struct sockaddr_in clientaddr;
		socklen_t clientaddrlen = sizeof(clientaddr);
		comm_fd.push_back(accept(listen_fd, (struct sockaddr*) &clientaddr, &clientaddrlen));

		if (vflag == 1)
		{
		  	  // Prints out the new connection
		  	  fprintf(stderr, "[%d] New connection\r\n", comm_fd[numThreads]);
		}
		
		// Creates a new thread that interacts with the client
		pthread_t thread;
		threads.push_back(thread);

		pthread_create(&threads.at(numThreads), NULL, worker, &comm_fd[numThreads]);
		numThreads++;
	}

	// After all threads are done, joins them back to main thread
	for (int c = 0; c < numThreads; ++c)
	{
		pthread_join(threads.at(numThreads), NULL);
	}
}
