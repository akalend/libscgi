#include "scgi.hpp"

using namespace std;

class Handler1: IScgiHandler {

	void run(map< string,string > * parms, char * buffUot) {
		
		map< string,string >::iterator it = parms->find("REQUEST_METHOD"); 
		
		if ( (*it).second == string("POST") ) {			
			it = parms->find("POST_DATA");
			if ( it != parms->end()) 
				strcpy(buffUot,(*it).second.c_str());
			return;	
		}
		
		it = parms->find("QUERY_STRING"); 
		if ( it != parms->end()) 
			strcpy(buffUot,(*it).second.c_str());		  
	}
};

class Handler2: IScgiHandler {

	void run(map< string,string > * parms, char * buffUot) {
		strcpy(buffUot,"54321");
	}
};


int main(int argc, char **argv)
{

	scgiServer scgi;

	if (scgi.checkPid()) {
		cerr << "the pid file exist or daemon already started\n" << endl;
		return 1;
	}

	
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
	
	if (scgi.init("127.0.0.1", 8080) ) {
		cerr << "server stopped\n";
		return 1;
	}
	
	scgi.addHandler("/post", reinterpret_cast<IScgiHandler *>(new Handler1()));	
	scgi.addHandler("/xxx",  reinterpret_cast<IScgiHandler *>(new Handler2()));

	
	scgi.run();

}

