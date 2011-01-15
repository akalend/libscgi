#include "scgi.hpp"
#include <memory>

using namespace std;

/**
* the user callback handler
* 
*/
class Handler1: public IScgiHandler {

	void run(map< string,string > * parms, char * buffUot) {
		string parm = getParam("REQUEST_METHOD",parms);
		// if method is POST 
		if ( parm == "POST" ) {
			// return to WEB-client the POST data
			strcpy(buffUot, getParam("POST_DATA",parms).c_str());
			return;	
		}
		// or return to WEB-client the QUERY_STRING parameter
		strcpy(buffUot, reinterpret_cast<char*>(userData));		  
	}
};

class Handler2: IScgiHandler {

	void run(map< string,string > * parms, char * buffUot) {
		
		strcpy(buffUot,"54321");
	}
};


int main(int argc, char **argv)
{

	// declare scgi server
	scgiServer scgi;

	if (scgi.checkPid()) {
		cerr << "the pid file exist or daemon already started\n" << endl;
		return 1;
	}

	//If You started server as daemon You must to use demonize() method 
	pid_t pid;
	if ((pid = scgi.demonize()) < 1) {
		if (pid == -1) {
			cerr << "demonize error\n";
			return 1;
		}	
		return 0;
	}

	if (scgi.savePid(pid) == -1) {
		cerr << "server stopped\n";
		return 1;	
	}
	
	// initialize server, bind and listen socket
	if (scgi.init("127.0.0.1", 8080) ) {
		cerr << "server stopped\n";
		return 1;
	}
	
	char * userData = "user data";
	
	auto_ptr<Handler1>h1(new Handler1());
	h1->setUserData( reinterpret_cast<void *>(userData));
	// add specific handlers 
	scgi.addHandler("/post", reinterpret_cast<IScgiHandler *>(h1.get()));	
	scgi.addHandler("/xxx",  reinterpret_cast<IScgiHandler *>(new Handler2()));

	scgi.run();

}

