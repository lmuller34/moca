
#include "fonctionsSite.h"

char* getIPaddress()
{
    char ac[80];
    if (gethostname(ac, sizeof(ac)) == -1)
	{
        perror("Error when getting local host name ");
        return NULL;
    }
    printf("Nom de l'hôte : %s.\n", ac);
	
	
	struct ifaddrs *ifaddr, *ifa;
	int family, s;
	char host[NI_MAXHOST];
	char* res = NULL;
	
	if (getifaddrs(&ifaddr) == -1) {
		perror("getifaddrs");
		exit(EXIT_FAILURE);
	}
	
	/* Walk through linked list, maintaining head pointer so we
	 can free list later */
	printf("Adresses IP disponibles :\n");
	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr == NULL)
			continue;
		
		family = ifa->ifa_addr->sa_family;
		
		/* For an AF_INET* interface address, display the address */
		
		if (family == AF_INET)
		{
			/* Display interface name and family (including symbolic
			 form of the latter for the common families) */
			printf("%s", ifa->ifa_name);
			
			s = getnameinfo(ifa->ifa_addr,
							(family == AF_INET) ? sizeof(struct sockaddr_in) :
							sizeof(struct sockaddr_in6),
							host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
			if (s != 0) {
				printf("getnameinfo() failed: %s\n", gai_strerror(s));
				exit(EXIT_FAILURE);
			}
			printf("\t<%s>\n", host);
			if(ifa->ifa_name[0] != 'l' || ifa->ifa_name[1] == 'o')
			{
				res = malloc(sizeof(char)*NI_MAXHOST);
				strcpy(res, host);
				broadcastAdd = (((((struct sockaddr_in*)(ifa->ifa_netmask))->sin_addr.s_addr) ^ (unsigned int)(pow(2,32)-1)) | inet_addr(res));
			}
		}
	}
	
    printf("Adresse choisie : %s\n", res);
	printf("Adresse de broadcast : %s\n", inet_ntoa(*(struct in_addr *)&broadcastAdd));
	freeifaddrs(ifaddr);
	return res;
}


int gstArgs(int argc, char* argv[], int* sdSend, int*sdRecv)
{
	//int i = 1;
	
	if(argc == 2 && strcmp(argv[1], "--help")==0)
	{
		printf("Arguments : ");
		printf("[masque_sous_reseau]\n");
		return -1;
	}
	/*if(argc < 2)
	{
		printf("Ce programme necessite des arguments. Essayez '--help' pour plus de precisions.\n\n");
		return -1;
	}
	
	while(i < argc)
	{
		if(inet_aton(argv[i], &addMask) != 0)
		{
			//printf("Mask : %s\n", inet_ntoa(addMask));
			i++;
		}
		else
		{
			printf("Veuillez entrer des arguments coherents.\n");
			exit(EXIT_FAILURE);
		}
	}*/
	
	*sdSend = socket(AF_INET, SOCK_DGRAM, 0);
	if(*sdSend < 0)
	{
		perror("Erreur d'ouverture de socket ");
		return -1;
	}
	
	*sdRecv = socket(AF_INET, SOCK_DGRAM, 0);
	if(*sdRecv < 0)
	{
		perror("Erreur d'ouverture de socket ");
		return -1;
	}
	
	int enabled = 1;
	setsockopt(*sdSend, SOL_SOCKET, SO_BROADCAST, &enabled, sizeof(enabled));
	setsockopt(*sdRecv, SOL_SOCKET, SO_BROADCAST, &enabled, sizeof(enabled));
	
	char* addr = getIPaddress();
	struct sockaddr_in myNetParams;
	bzero(&myNetParams,sizeof(myNetParams));
	myNetParams.sin_family = AF_INET;
	myNetParams.sin_addr.s_addr = inet_addr(addr);
	myNetParams.sin_port = htons(PORT_SEND);
	//memset(myNetParams->sin_zero,0,8);
	free(addr);
	
	if(bind(*sdSend, (struct sockaddr*)&myNetParams, (socklen_t)sizeof(myNetParams)) == -1)
	{
		perror("Erreur de lien a la boite reseau ");
		return -1;
	}
	
	myNetParams.sin_family = AF_INET;
	myNetParams.sin_addr.s_addr = INADDR_ANY;
	myNetParams.sin_port = htons(PORT_RECV);
	
	if(bind(*sdRecv, (struct sockaddr*)&myNetParams, (socklen_t)sizeof(myNetParams)) == -1)
	{
		perror("Erreur de lien a la boite reseau ");
		return -1;
	}
	printf("Lancement du site…\n");
	
	return 0;
}

char* itoa(long n)
{
	int i = 1, nbChiffres;
	long trans = n;
	while(trans > 10)
	{
		trans /= 10;
		i++;
	}
	nbChiffres = i;
	char* chaine = malloc((nbChiffres+1) * sizeof(char));
	trans = n;
	
	for(i = 0 ; i < nbChiffres ; i++)
	{
		chaine[nbChiffres-1-i] = 48 + trans % 10;
		trans /= 10;
	}
	chaine[nbChiffres] = 0;
	
	return chaine;
}

int backupSocketNeighbors(struct sockaddr_in* neighbors, struct sockaddr_in** neighborsTmp, int nbNeighbors)
{
	int j;
	*neighborsTmp = malloc(sizeof(struct sockaddr_in)*nbNeighbors);
	if(*neighborsTmp == NULL)
	{
		perror("Erreur d'allocation ");
		return -1;
	}
	for(j = 0 ; j < nbNeighbors ; j++)
	{
		(*neighborsTmp)[j] = neighbors[j];
	}
	
	return 0;
}

int recoverSocketNeighbors(struct sockaddr_in** neighbors, struct sockaddr_in** neighborsTmp, int nbNeighbors, struct sockaddr_in paramsNewNeighbor)
{
	int j;
	*neighbors = realloc(*neighbors, sizeof(struct sockaddr_in)*nbNeighbors);
	if(*neighbors == NULL)
	{
		perror("Erreur de reallocation ");
		return -1;
	}
	
	bzero(&((*neighbors)[nbNeighbors-1]),sizeof((*neighbors)[nbNeighbors-1]));
	((*neighbors)[nbNeighbors-1]).sin_family = paramsNewNeighbor.sin_family;
	((*neighbors)[nbNeighbors-1]).sin_port = paramsNewNeighbor.sin_port;
	((*neighbors)[nbNeighbors-1]).sin_addr.s_addr = paramsNewNeighbor.sin_addr.s_addr;
	
	for(j = 0 ; j < nbNeighbors-1 ; j++)
	{
		(*neighbors)[j] = (*neighborsTmp)[j];
	}
	free(*neighborsTmp);
	
	return 0;
}

int broadcast(char* message, int sdSend)
{
	struct sockaddr_in netParamsNeighbor;
	bzero(&netParamsNeighbor,sizeof(netParamsNeighbor));
	netParamsNeighbor.sin_family = AF_INET;
	netParamsNeighbor.sin_port = htons(PORT_RECV);
	netParamsNeighbor.sin_addr.s_addr = broadcastAdd;
	if (sendto(sdSend, message, strlen(message), 0, (struct sockaddr *)&netParamsNeighbor,sizeof(netParamsNeighbor)) == -1)
	{
		perror("sendto broadcast ");
		return -1;
	}
	return 0;
}

int message(char* add, char* message, int sdSend)
{
	struct sockaddr_in netParamsNeighbor;
	bzero(&netParamsNeighbor,sizeof(netParamsNeighbor));
	netParamsNeighbor.sin_family = AF_INET;
	netParamsNeighbor.sin_port = htons(PORT_RECV);
	netParamsNeighbor.sin_addr.s_addr = inet_addr(add);
	if (sendto(sdSend, message, strlen(message), 0, (struct sockaddr *)&netParamsNeighbor,sizeof(netParamsNeighbor)) == -1)
	{
		perror("sendto message ");
		return -1;
	}
	return 0;
}


