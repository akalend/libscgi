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