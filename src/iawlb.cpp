/*2:*/
#line 35 "./iawlb.w"

/*20:*/
#line 338 "./iawlb.w"

#include <iostream> 
#include <string> 
#include <cstdio> 
#include <cstdarg> 
#include <cstdlib> 
#include <cerrno> 

/*:20*//*21:*/
#line 348 "./iawlb.w"

#include <pthread.h> 

/*:21*//*22:*/
#line 353 "./iawlb.w"

#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h>  
#include <netdb.h>  

#include <unistd.h>  

#include <poll.h>  

/*:22*//*23:*/
#line 367 "./iawlb.w"

#include "SyncedList.hpp"


/*:23*/
#line 36 "./iawlb.w"

/*10:*/
#line 154 "./iawlb.w"

typedef struct thr_info_s
{
pthread_t thr;
size_t idx;
std::string addr;
std::string port;
int status;
std::string msg;
}thr_info;

/*:10*//*12:*/
#line 181 "./iawlb.w"

static void*process_request(void*arg);

/*:12*//*15:*/
#line 224 "./iawlb.w"

SyncedList<int> accepted_sock;


/*:15*//*24:*/
#line 374 "./iawlb.w"

#define MAX_READ_BUFFER 256  
#define ANS_TIMEOUT 31       
#define MAX_NUM_OF_DEST_PORT 32
#define FOREVER for(;;) 



/*:24*//*29:*/
#line 439 "./iawlb.w"

pthread_mutex_t logmu= PTHREAD_MUTEX_INITIALIZER;


/*:29*/
#line 37 "./iawlb.w"

/*11:*/
#line 169 "./iawlb.w"

int new_dispatcher(thr_info*pt)
{
pthread_attr_t attr;
pthread_attr_init(&attr);
int r= pthread_create(&pt->thr,&attr,process_request,pt);
pthread_attr_destroy(&attr);
return r!=0?-1:0;
}

/*:11*//*25:*/
#line 385 "./iawlb.w"

std::string get_addr(const char*s)
{
std::string addr(s);
size_t p= addr.find_first_of(':');
if(p==std::string::npos||p==0){
return"localhost";
}
return addr.substr(0,p-1);
}

/*:25*//*26:*/
#line 398 "./iawlb.w"

std::string get_port(const char*s)
{
std::string port(s);
size_t p= port.find_first_of(':');
if(p==std::string::npos){
return port;
}
return port.substr(p+1);
}

/*:26*//*27:*/
#line 411 "./iawlb.w"

void*get_in_addr(struct sockaddr*sa)
{
if(sa->sa_family==AF_INET){
return&((static_cast<struct sockaddr_in*> (sa))->sin_addr);
}

return&((static_cast<struct sockaddr_in6*> (sa))->sin6_addr);
}


/*:27*//*28:*/
#line 425 "./iawlb.w"

void Log(const char*s,...)
{
pthread_mutex_lock(&logmu);
va_list ap;
va_start(ap,s);
(void)vprintf(s,ap);
va_end(ap);
fflush(stdout);
pthread_mutex_unlock(&logmu);
}

/*:28*/
#line 38 "./iawlb.w"

/*13:*/
#line 186 "./iawlb.w"

static void*process_request(void*arg)
{
thr_info*ti= static_cast<thr_info*> (arg);
int sockcaller;
int sockfd;

size_t id= ti->idx;

FOREVER{
/*14:*/
#line 219 "./iawlb.w"

while(!accepted_sock.pop(&sockcaller));

/*:14*/
#line 196 "./iawlb.w"


Log("%zu: popped sock %d for (%s, %s)\n",id,sockcaller,ti->addr.c_str(),ti->port.c_str());

/*16:*/
#line 230 "./iawlb.w"

struct addrinfo hints= {0};
struct addrinfo*servinfo;
struct addrinfo*pa;

hints.ai_family= AF_UNSPEC;
hints.ai_socktype= SOCK_STREAM;
hints.ai_flags= AI_PASSIVE;

int status= getaddrinfo(ti->addr.c_str(),ti->port.c_str(),&hints,&servinfo);
if(status!=0){
ti->status= status;
ti->msg= gai_strerror(status);
Log("%zu: error: %s (status %d)\n",id,ti->msg.c_str(),ti->status);
pthread_exit(arg);
}
for(pa= servinfo;pa!=NULL;pa= pa->ai_next){
sockfd= socket(pa->ai_family,pa->ai_socktype,pa->ai_protocol);
if(sockfd==-1){
continue;
}
if(connect(sockfd,pa->ai_addr,pa->ai_addrlen)==-1){
Log("%zu: failed connect for socket %d\n",id,sockfd);
close(sockfd);
continue;
}
break;
}
if(pa==NULL){
ti->status= -1;
ti->msg= "failed to connect";
freeaddrinfo(servinfo);
Log("%zu: error: %s (status %d)",id,ti->msg.c_str(),ti->status);
pthread_exit(arg);
}
freeaddrinfo(servinfo);

/*:16*/
#line 200 "./iawlb.w"


Log("%zu: resolved address; sock %d connected to (%s, %s)\n",id,sockfd,ti->addr.c_str(),ti->port.c_str());

/*17:*/
#line 269 "./iawlb.w"

char readbuf[1024*MAX_READ_BUFFER]= "";
ssize_t readstat= recv(sockcaller,readbuf,sizeof(readbuf),0);
if(readstat> 0){
Log("%zu: read %zd from sock %d\n",id,readstat,sockcaller);
ssize_t targetlen= 0;
while(targetlen<readstat){
ssize_t sendstat= send(sockfd,readbuf,readstat,MSG_NOSIGNAL);
targetlen+= sendstat;
}
Log("%zu: sent %zd to sock %d\n",id,targetlen,sockfd);

struct pollfd pfd[1]= {{sockfd,POLLIN,0}};
int pret= poll(pfd,1,ANS_TIMEOUT*1000);
if(pret==0){
Log("%zu: timed out on sock %d\n",id,sockfd);
}else if(pret> 0){
ssize_t anstat= recv(sockfd,readbuf,sizeof(readbuf),0);
if(anstat> 0){
Log("%zu: received answer from sock %d (%zd bytes)\n",id,sockfd,anstat);
pfd[0].fd= sockcaller;
pfd[0].events= POLLOUT;
pfd[0].revents= 0;
int uret= poll(pfd,1,5);
if(uret> 0){
targetlen= 0;
while(targetlen<anstat){
ssize_t sendstat= send(sockcaller,readbuf,anstat,MSG_NOSIGNAL);
targetlen+= sendstat;
}
Log("%zu: sent back answer to sock %d (bytes %zd)\n",id,sockcaller,targetlen);
}
}else{
Log("%zu: error receiving from sock %d (%d)\n",id,sockfd,anstat);
}
}
}

/*:17*//*18:*/
#line 309 "./iawlb.w"

close(sockfd);
close(sockcaller);



/*:18*/
#line 204 "./iawlb.w"


struct timespec ts= {0,50000000};
if(nanosleep(&ts,NULL)<0){

break;
}
}

return arg;
}


/*:13*/
#line 39 "./iawlb.w"

/*3:*/
#line 42 "./iawlb.w"

int main(int argc,char**argv)
{
size_t i;
int backlog= 1;
int thr_num= 0;
thr_info thr[MAX_NUM_OF_DEST_PORT];


/*4:*/
#line 65 "./iawlb.w"

if(argc<3){
std::cerr<<"LISTEN_ON [IP:]PORT [[IP:]PORT]*\n";
return EXIT_FAILURE;
}

/*:4*/
#line 51 "./iawlb.w"

/*5:*/
#line 73 "./iawlb.w"

struct addrinfo hints= {0};
struct addrinfo*sinfo= NULL;
struct addrinfo*sp= NULL;
hints.ai_family= AF_UNSPEC;
hints.ai_socktype= SOCK_STREAM;
hints.ai_flags= AI_PASSIVE;
int rv= getaddrinfo(NULL,argv[1],&hints,&sinfo);
if(rv!=0){
std::cerr<<"getaddrinfo err: "<<gai_strerror(rv)<<"\n";
return EXIT_FAILURE;
}

/*:5*//*6:*/
#line 90 "./iawlb.w"

int serversock;
for(sp= sinfo;sp!=NULL;sp= sp->ai_next){
serversock= socket(sp->ai_family,sp->ai_socktype,sp->ai_protocol);
if(serversock==-1){
perror("server socket");
continue;
}
int yes= 1;
if(setsockopt(serversock,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes))==-1){
perror("setsockopt");
return EXIT_FAILURE;
}
if(bind(serversock,sp->ai_addr,sp->ai_addrlen)==-1){
close(serversock);
perror("bind");
continue;
}
break;
}
if(sp==NULL){
std::cerr<<"cannot listen on "<<argv[1]<<"\n";
return EXIT_FAILURE;
}
freeaddrinfo(sinfo);

/*:6*/
#line 52 "./iawlb.w"

/*7:*/
#line 125 "./iawlb.w"

for(i= 2;i<argc&&(i-2)<(sizeof(thr)/sizeof(thr[0]));++i){
thr[thr_num].addr= get_addr(argv[i]);
thr[thr_num].port= get_port(argv[i]);
thr[thr_num].idx= i-2;
int r= new_dispatcher(&thr[thr_num]);
if(r<0){
std::cerr<<"cannot create dispatcher addr "<<thr[thr_num].addr<<":"<<thr[thr_num].port<<"\n";
return EXIT_FAILURE;
}
++thr_num;
}

/*:7*//*8:*/
#line 141 "./iawlb.w"

backlog+= thr_num;

/*:8*/
#line 53 "./iawlb.w"

/*9:*/
#line 146 "./iawlb.w"

if(listen(serversock,backlog)==-1){
perror("listen");
return EXIT_FAILURE;
}

/*:9*/
#line 54 "./iawlb.w"

/*19:*/
#line 317 "./iawlb.w"

FOREVER{
struct sockaddr_storage caller_addr= {0};
socklen_t addr_size= sizeof(caller_addr);
int ax_sock= accept(serversock,static_cast<struct sockaddr*> (&caller_addr),&addr_size);
if(ax_sock==-1){
perror("accept");
continue;
}
char addr_buf[INET6_ADDRSTRLEN];
inet_ntop(caller_addr.ss_family,
get_in_addr(static_cast<struct sockaddr*> (&caller_addr)),
addr_buf,
sizeof(addr_buf));
Log("request from %s\n",addr_buf);
accepted_sock.enqueue(ax_sock);
}


/*:19*/
#line 55 "./iawlb.w"


return 0;
}

/*:3*/
#line 40 "./iawlb.w"


/*:2*/
