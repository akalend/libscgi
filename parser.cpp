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

using namespace std;

class Parser {
	public:

		int run(char * buff, map< string,string > * parms) {
			char * p = buff;
			char * buff_end = buff+BUFFSIZE;
			char name[32];
			char value[256];
			
			while(*(++p)!=':' && p != buff_end);
			if (p == buff_end) return -1;
			
			strncpy(value,buff, p-buff);			
			value[p-buff]='\0';	
			int len = atoi(value);					
//			cout<< "len=" << len << endl;
			
			if (*(p+len+1) != ',') {
				printf("error format c=%c ", *(p+len+1));
				printf("%s\n", (p+len+1));
				return -1;
			}
			
			// parse params
			char * q = name;
			++p;
			int n = 0;
			int l = 0;
			bool isName = true;
			*q = *p;
			while(++n < len) {
				if (*p == '\0' && isName) {
					q = name;
					name[l]='\0';
//					printf("%s\n", q);	
					q=value;		
					l=0; 
					isName = false;
				} else if (*p == '\0' && !isName) {
					q = value;
					value[l]='\0';
					parms->insert(pair<string,string>(string(name),string(value)));
					//printf("%s %s\n",name,value);			
					l=0;
					q = name;
					isName = true;
				} else {
					*q = *p;
					++q;
					++l;
				}
					++p;
			}

			// parse POST
			map<string,string>::iterator it;
			it = parms->find("CONTENT_LENGTH");
			if (it == parms->end()) {
				cout << "cannot CONTENT_LENGTH parameter\n";
				return -1;
			}
			
			const char * len_str = const_cast<char *>((*it).second.c_str());
			len = atoi(len_str);
			n=0;
			p = p+2;
			strncpy(value,p,len);
			*(value+len) = '\0';

			parms->insert(pair<string,string>("POST_DATA",value));
			
			return 0;
		}
};