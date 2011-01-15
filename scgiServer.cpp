#include "scgi.hpp"
#include "parser.cpp"

using namespace std;

void on_connect(int fd, short event, void *arg) 
	{
		sockaddr_in client_addr;
		socklen_t  len = sizeof(client_addr);
		// Accept incoming connection
		int sock = accept(fd, reinterpret_cast<sockaddr*>(&client_addr), &len);
		if (sock < 1) { 
			std::cerr << "accept error" << std::endl;
			return;
		}

		char * data =  (char*) malloc(BUFFSIZE);
		if (!data) {
			free(data);
			close(sock);
			cerr << "malloc error" << endl;
			return;
		}

		int data_size = read(sock, data, BUFFSIZE);
		if (data_size <= 0) {
			std::cout << "error read data " << strerror(errno) << std::cout;
			free(data);
			close(sock);
			return;	
		}

		map<string,string> parms;
		Parser parser;
		parser.run(data, &parms);

		map<string,string>::iterator ip;
		
//		for(ip = parms.begin(); ip != parms.end(); ip++) {
//			cout << (*ip).first << "\t\t" << (*ip).second << endl;
//		}

		char handler_data[BUFFSIZE];	
		bzero(handler_data,BUFFSIZE);	
		char out_data[BUFFSIZE];		
		
		map<string,IScgiHandler *>::iterator it;
		map<string,IScgiHandler *> * pHandlers = reinterpret_cast< map<string,IScgiHandler *> * >(arg);
						 		 
		it = pHandlers->find( parms["DOCUMENT_URI"] );
		
		int statusCode = 200; 
		char statusMsg[10];
		bzero(statusMsg,10);
		*handler_data = '\0';
		int contentLenght = 0;
		
		char headersOutBuff[1024];
		headersOutBuff[0] = '\0';
		if (it == pHandlers->end()) {
			statusCode = 404;
			strcpy(statusMsg,"Not Found");
		} else {
			strcpy(statusMsg,"Ok");
			IScgiHandler * handler = (*it).second;
			handler->run(&parms, handler_data);	
			contentLenght = strlen(handler_data);
			handler->getHeaders(headersOutBuff);	
		}
		
		sprintf(out_data,"Status: %d %s\r\nContent-lenght: %d\r\nPowered: libscgi\r\n%s\r\n%s", statusCode, statusMsg, contentLenght, headersOutBuff,handler_data);

		write(sock, out_data, strlen((char*)out_data) );	

		free(data);
		close(sock);
	}

	bool scgiServer::addHandler(char * key, IScgiHandler * handler) {
		if ( scgiServer::handlers.find(string(key)) != scgiServer::handlers.end())
			return 1;
			
		scgiServer::handlers.insert(pair<string,IScgiHandler *>(string(key),handler));
		return 0;
	};
		
	int scgiServer::init(char * ip_addr, u_short port) {
	
		scgiServer::pidfile="scgi_server.pid";
		// Create server socket
		scgiServer::server_sock = socket(AF_INET, SOCK_STREAM, 0);
		if (server_sock == -1) {
			std::cerr << "Failed to create socket" << strerror(errno) << std::endl;
			return 1;
		}	
		
		bzero((void*)&sa,sizeof(sa));
		int         on      = 1;		
		scgiServer::sa.sin_family       = AF_INET;
		scgiServer::sa.sin_port         = htons(port);
		scgiServer::sa.sin_addr.s_addr  = inet_addr(ip_addr);

		// Set option SO_REUSEADDR to reuse same host:port in a short time
		if (setsockopt(scgiServer::server_sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1) {
			cerr << "Failed to set option SO_REUSEADDR" << std::endl;
			return 1;
		}

		int err;
		if ( (err = bind( scgiServer::server_sock, (struct sockaddr*) &(scgiServer::sa), sizeof(scgiServer::sa)))  < 0 )
		{
			close(scgiServer::server_sock);
			cerr << "bind error: " << strerror( errno) <<  std::endl;
			return  1;
		}

		if (listen(scgiServer::server_sock, 10) == -1) {
			close(scgiServer::server_sock); 		
			std::cerr << "Failed to make server listen: " << strerror(errno) << std::endl;
			return 1;
		}
				
		return 0;
	};
		
	int scgiServer::run() {

		struct event ev;
		event_base * base = event_init();
		if (!base) {
			std::cerr << "Failed to create new event base" << std::endl;
			return 1;
		}		
		
		event_set(&ev, scgiServer::server_sock, EV_READ | EV_PERSIST, on_connect, &(scgiServer::handlers));
		event_add(&ev, NULL);
		
		event_dispatch();

		event_base_free(base);

    return 0;
	};

	int scgiServer::demonize() {
	
		int  fd;
		
		switch (fork()) {
		case -1:
			perror("demonize fork failed");
			return -1;

		case 0:
			break;

		default:
			return 0;
		}

		int pid = getpid();

		if (setsid() == -1) {
			perror("setsid() failed");        
			return -1;
		}

		umask(0);

		for (int i = 0; i < 1024; i++)
			close(i);

		fd = open("/dev/null", O_RDWR);
		if (fd == -1) {
			perror("open(\"/dev/null\") failed");        
			return -1;
		}

		if (dup2(fd, STDIN_FILENO) == -1) {
			perror("dup2(STDIN) failed");        
			return -1;
		}

		if (dup2(fd, STDOUT_FILENO) == -1) {
			perror("dup2(STDOUT) failed");        		
			return -1;
		}

		if (dup2(fd, STDERR_FILENO) == -1) {
			perror("dup2(STDERR) failed");
			return -1;
		}

		if (fd > STDERR_FILENO) {
			if (close(fd) == -1) {
				perror("close(fd) failed");        		
				return -1;
			}
		}

		return pid;		
	
};		

bool scgiServer::checkPid() {
	struct stat st;
	if (!stat(scgiServer::pidfile.c_str(),&st)) {
		return true;		
	}
	
	if (errno==ENOENT)
		return false;
		
	return true;				
};

int scgiServer::savePid( pid_t pid )
{
	int fd = open(scgiServer::pidfile.c_str(),  O_WRONLY|O_CREAT);

	if (!fd) {
	  cerr << "can't create pid file " << scgiServer::pidfile << " " << strerror(errno)<<endl;
		return -1;
	}

	write(fd, (void *)pid, sizeof(pid));
	close(fd);
	return 0;
};

string IScgiHandler::getParam(string paramName, map< string,string > * parms) {
	map< string,string >::iterator it = parms->find(paramName); 
	
	if ( it != parms->end()) 
		return (*it).second;
				
	return "";	
};

void IScgiHandler::getHeaders(char * headersOutBuff) {
		char * p = headersOutBuff;
		if (headers.size()) {
			list<string>::iterator it;
			for (it=headers.begin();it!=headers.end();it++) {
				strcpy(p,(*it).c_str());
				int size = (*it).size();
				p = p + size;
				*(p++) = '\r';
				*(p++) = '\n';				
			}
		}
		*p='\0';

};

void  IScgiHandler::addHeader(string header) {
	headers.push_back(header);
};

