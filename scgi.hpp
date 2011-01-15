#include <event.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <cstring>
#include <sys/stat.h>
#include <fcntl.h> 
#include <err.h>
#include <errno.h>
#include <iostream>

#include <memory>
#include <map>
#include <string>
#include <list>

#ifndef __SCGILIB__
#define __SCGILIB__

#define BUFFSIZE 2048
using namespace std;

class IScgiHandler {
	protected:
		void * userData;
		list <string> headers;

	 void addHeader(string header);

	public:
		IScgiHandler(){};
	
	 void setUserData(void * userData_) {
		userData = userData_;
	 };	

	 string getParam(string paramName, map< string,string > * parms);
	 virtual ~IScgiHandler(){};
	 virtual void run(map< string,string > * parms, char * buffUot) {};
	 void getHeaders(char * outBuffer);	
};

class scgiServer
{
		int				server_sock;
		sockaddr_in		sa;
		string pidfile;	
		map<string, IScgiHandler * > handlers; 
		
	public:
		bool addHandler(char * key, IScgiHandler * handler);
		int init(char * ip_addr, u_short port);
		int run();
		int demonize();
		bool checkPid();
		int savePid(pid_t pid);
		void setPidfile(string _pidfile){
			pidfile = _pidfile;
		};
		void closeSock() {
			close(server_sock);
		}		
};
#endif