/*
 * Copyright 2011 (C) Alexandre Kalendarev 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */


#include "scgi.hpp"
#include <memory>
#include <iostream>
#include <algorithm>
#include <string>
#include <fstream>
#include <sys/types.h>
//   #include <cctype>


#define MAXITEMS 10

using namespace std;

struct item {
	string  name;
	int		order;
	int		id;
};

long long now_microseconds(void) {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (long long) tv.tv_sec * 1000000 + (long long) tv.tv_usec;
}


/**
* the autocomplete callback handler
* 
*/
class Handler1: public IScgiHandler {

	void run(map< string,string > * parms, char * buffOut) {

		// if method is POST 
		if ( getParam("REQUEST_METHOD",parms) != "POST" ) 
			return;
		
		// return to WEB-client the POST data
		long long start_time = now_microseconds();

		map<string,item> * cityes = reinterpret_cast<map<string,item> * >(userData);
		map<string,item>::iterator it, it_low, it_up;

		u_char * postData = (u_char*)getParam("POST_DATA",parms).c_str();
		int len =  strlen( reinterpret_cast<char*>(postData));
		
		for(int i=0; i < len;i++) {				

			u_char c = *(postData +i);
			
			/* fix  tolower Russian (cyrilic)  UTF-8 */
			if ( c >= 0x90 && c<= 0x9F) { // А-П
				*(postData +i) = c+32;
			}

			if ( c >= 0xA0 && c<= 0xAF) {  // Р-Я
				*(postData +i) = c+224;
			}
		}
					
		const char * fword = const_cast<char*>(reinterpret_cast<char *>(postData));			

		it_low=cityes->lower_bound (fword);  // itlow points to b

		if (it_low==cityes->end()) {
			sprintf(buffOut, "{\"cityes\" : []}");			
			addHeader("Content-type: text/json");			
			return;
		}	
		
		map <int,item> result;		
		for (it = it_low; it != cityes->end(); it++) {
			if ( (*it).first.substr(0,len) != fword) break;
			result.insert(pair<int,item>((*it).second.order,(*it).second));
		}						

		map <int,item>::iterator it_res;		
		
		string out;
		int i = 0;
		char data[256];
		
		for(it_res = result.begin(); it_res != result.end(); it_res++ ) {
			char zpt =',';
			if (!i++) 
				zpt = ' ';
			if (i > MAXITEMS)
					break;
			sprintf(data ,"%c{ \"name\": \"%s\", \"id\":%d}", zpt,(*it_res).second.name.c_str(),(*it_res).second.id);	
			out = out + data ;
		}

//			cout << out << endl << "len=" << out.size() << endl;
		int all_time = now_microseconds()-start_time;		
		sprintf(buffOut, "{\"cityes\" : [%s],\"time\":%d}" , out.c_str(),all_time);
		
		addHeader("Content-type: text/json");

		return;	
		
	}
};

int loadData( char * filename, map<string,item> * cityes) {

	ifstream cityFile;

	cityFile.open(filename);

	if (!cityFile) {
	  cerr << "can't open city file \n"; 
		return 1;
	}
	
	char data[256];

	int i = 0;	
	while (cityFile.getline(data,256)) {
		char * tok = strtok(data,"\"");
		char * p = data+(strlen(tok)+3);
		char * tok2 = strtok(p,"\",");

		char * p2 = p+(strlen(tok2)+3);		

		char * tok3 = strtok(p2,",");
		char * p3 = p2+(strlen(tok3)+1);		

		item Item;
		Item.name = string(p+1);
		Item.order =  atoi(p2);		
		Item.id =  atoi(p3);		
				
		cityes->insert(pair<string,item>( string(tok), Item));
		++i;
//		cout << (p+1) << " " << Item.order<< " " << Item.id << endl;
	}

//	cout << "readed items=" << i << endl;
	cityFile.close();
	
	return 0;
} 

int main(int argc, char **argv)
{

	if (argc < 2) {
		cout << "Usage:\n\t"<< argv[0] << " [filename] "<< endl;
		return 0;
	}
	
	map<string,item> cityes;
	//load data
	loadData(argv[1], &cityes );	

	// declare scgi server
	scgiServer scgi;

	if (scgi.checkPid()) {
		cerr << "the pid file exist or daemon already started\n" << endl;
		return 1;
	}

	//If You started server as daemon You must to use demonize() method 
	pid_t pid;
	if ( false && (pid = scgi.demonize()) < 1) {
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
	
	auto_ptr<Handler1>h1(new Handler1());
	h1->setUserData( reinterpret_cast<void *>(&cityes));
	// add specific handlers 
	scgi.addHandler("/cityes", reinterpret_cast<IScgiHandler *>(h1.get()));	

	scgi.run();

}

