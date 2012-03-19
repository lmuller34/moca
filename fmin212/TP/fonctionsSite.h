
#ifndef DEF_FONC_SITE
#define DEF_FONC_SITE

#include <arpa/inet.h>
#include <errno.h>
#include <ifaddrs.h>
#include <math.h>
#include <netdb.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <unistd.h>


#define PORT_SEND 31000
#define PORT_RECV 31001

typedef enum msg_type {
	MESSAGE = 1,
	REQUEST = 2
} msg_type;

typedef struct site {
	in_addr_t broadcastAdd;
	int sdSend;
	int sdRecv;
	int running;
	struct sockaddr_in* neighbours;
	size_t nbNeighbours;
	struct sockaddr_in *neighboursTmp;
} site;

extern site this_site;


char* getIPadress();
int init(int argc, char** argv);
char* itoa(long n);
int backupSocketNeighbours();
int recoverSocketNeighbours(struct sockaddr_in paramsNewNeighbour);

int broadcast(msg_type t, char* msg);
int sendMessage(int siteID, msg_type t, char* msg);
int sendMessageWithAdd(char* add, msg_type t, char* msg);
void standardInput();
int hostsUpdate(struct sockaddr_in netParamsNeighbour);
void requestTreatment();
void printNeighbours();
void getMessageFromString(char* string, msg_type* type, char** message);

#endif


