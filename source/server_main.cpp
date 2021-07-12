//////////////////////////////////////////////////////////////
// server_main.cpp											//
// Basic socket programming example of server application	//
// Last updated 10/10/2019									//
// Originally developed by Ed Walker						//
// Adapted by Kent Jones and Scott Griffith					//
//////////////////////////////////////////////////////////////
//******************************************************
//*													   *
//*   NEEDS TO HAVE ACCESS TO WINDOWS SOCKET LIBRARY!  *
//*   												   *
//*   Modify your ./.vscode/tasks.json				   *
//*   In the "build project" task include:			   *
//*   "-lwsock32" as an argument					   *
//*   												   *
//*   This should be directly after the line:		   *
//*    "\"${workspaceFolder}\"/source/*.cpp", 		   *
//*													   *
//******************************************************

#include <iostream>		  //terminal output
#include <string>		  //Creation of messages
#include "Socket.h"		  //Build winSock wrapper, shared definition by both client and server
#include <fstream>
#include <sstream>
#include <vector>
#include <stdio.h>
#include <time.h>
#include <cstdlib>
#include <ctime>
using namespace std;

//Communication Port to connect to/from
const int MYPORT = 50001;

// Basic request lines
string ok = "HTTP/1.1 200 OK\r\n";
string bad_req = "HTTP/1.1 400 Bad Request\r\n\r\n";

//Utility Function for clean exit of program
// @input: message, display to the terminal to prompt for exit
// Closes socket and exits program
int done(const string message) {
	Socket::Cleanup();
	cout << message;
	cin.get();
	exit(0);
}

// Gets UTC time for header
string getTime() {
	time_t now = time(0);
	tm* UTC_time = gmtime(&now);
	string gmt = asctime(UTC_time);
	return gmt;
}

// Reads file from external .txt file and returns string copy
// of specified file. (Modified from stack overflow entry)
string readFile(const string &fileName) {
	// Opens file
	ifstream my_file(fileName.c_str(), ios::in | ios::binary | ios::ate);

	// Finds file size and then directs cursor to the beginning of the file
	ifstream::pos_type fileSize = my_file.tellg();
	my_file.seekg(0, ios::beg);

	// Reads the characters from the file into a vector
	vector<char> bytes(fileSize);
	my_file.read(bytes.data(), fileSize);

	// Returns the string containing the complete file
	return string(bytes.data(), fileSize);
}

// Gets size of the file being sent
int getSize(const string& str) {
	// Counts the chars in file and stores size of file in count
	int count = 0;
	for (int i = 0; str[i]; i++) {
		count++;
	}
	return count;
}

// Builds most basic HTTP header for proper functionality
string formHeader(const string& str) {
	string connectType = "Connection: close\r\n\r\n";

	string header = ok;
	header.append(connectType);
	return header;
}

// Checks the received message for proper request form, sends GET requests to formHeader function
string checkMsg(const string& str) {
	string space = " ";
	size_t foundSpace = str.find(space);
	if (foundSpace != string::npos) {
		// creates a string of characters found before the first space
		string req_type = str.substr(0, foundSpace);
		if (req_type == "GET") {
			return formHeader(str);
		}
		else {
			return bad_req;
		}
	}
	else {
		return bad_req;
	}
	
}

// THE SERVER: serves clients
int main() {
	//Initilize socket, make sure network stack is available
	if (!Socket::Init()) {
		cerr << "Fail to initialize WinSock!\n";
		return -1;
	}

	// Step 1: Create a TCP socket
	Socket server("tcp");

	// Step 2: Bind socket to a port (MYPORT)	
	if (!server.sock_bind("", MYPORT)) {
		string str;
		str = "Could not bind to port " + to_string(MYPORT);
		done(str);
	}

	// Step 3: Ask my socket to "listen"
	if (!server.sock_listen(1)) 
	{
		done("Could not get socket to listen");
	}

	cout << "Server is now listening on port " << MYPORT << endl;

	// Read files into program
	string theHTML = readFile("assets/index.html");
	string theJPG = readFile("assets/WhitworthLogo.jpg");
	string theICO = readFile("assets/favicon.ico");
	
	while(1) {

		// Step 4:	Wait to accept a connection from a client.  
		//			The variable conn is the "connected" socket.  This is the socket that is actually connected to the server.
		//			The socket variable server can be used to accept another connection if you want.
		Socket conn = server.sock_accept();
		cout << "Connected\n" << endl;
	
		// Step 5: Receive/send a message from/to the client.
		string msg = conn.msg_recv();
		cout << "Received message: " << msg << endl;

		// Defines the search to deterrmine the file type being sent
		string icon = "favicon.ico";
		string jpg = "WhitworthLogo.jpg";
		string spaceSlash = " / ";
		size_t foundICO = msg.find(icon);
		size_t foundJPG = msg.find(jpg);
		size_t found = msg.find(spaceSlash);
		string resp = checkMsg(msg);

		// Checks if it requests HTML
		if (found != string::npos) {
			conn.msg_send(resp+theHTML);
		}

		// Checks if client requests icon
		if (foundICO != string::npos) {
			conn.msg_send(resp+theICO);
		}

		// Checks if client requests jpg
		if (foundJPG != string::npos) {
			conn.msg_send(resp+theJPG);
		}
		
		cout << "Response sent" << endl;

	}

	done("Press enter to exit");
	return 0;

}


