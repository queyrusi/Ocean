#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <signal.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <ctype.h>
#include "structures.h"
#include "fonctions_serveur.h"
#include "serveur.h"
#include "donnees.h"




// Global variables
ClientList *root;
ClientList *now;
/**
 * fonction print_msg 
 * ------------------
 * 
 */
void print_msg(char *talker, char * chat) 
{
	fputs(talker, stdout);
	fputs(": ", stdout);
	fputs(chat, stdout);
	fflush(stdout);
}

/**
 * fonction send_to_all_clients
 * ------------------
 * 
 */
void send_to_all_clients(ClientList *np, char tmp_buffer[]) {// Permet d'envoyer un message à tous les clients d'un coup
    ClientList *tmp = root->link;
    while (tmp != NULL) {// Tant qu'il reste des liens à explorer
        if (np->data != tmp->data) { // On envoi à tous les clients sauf à lui-meme
            printf("Envoyer a la socket %d: \"%s\" \n", tmp->data, tmp_buffer);
            send(tmp->data, tmp_buffer, LENGTH_SEND, 0);
        }
        tmp = tmp->link;
    }
}
/**
 * fonction quitter_sock
 * ------------------
 * 
 */

void quitter_sock(int sig) {// On cherche ici a fermer toutes les sockets lorsqu'on va shutdown le systeme
    ClientList *tmp;
    while (root != NULL) {// Tant qu'on a pas fermer la racine on continue
        close(root->data); // On ferme la socket sur laquelle on se situe puis on remonte les liens
        tmp = root;
        root = root->link;
        free(tmp);
    }
    printf("Au revoir\n");// On a tout fermer, meme le serveur lui meme
    exit(EXIT_SUCCESS);
}

/**
 * fonction read_header
 * --------------------
 * 
 * retourne le nom du client qui parle
 * 
 * inscrit à l'emplacement "&loglen" le premier octet de la socket puis à 
 * "username" un nombre "loglen" d'octets de la socket
 * 
 */
void read_header(int sock, char * username) 
{
	int loglen ;
	read(sock, &loglen, 1);
	read(sock, username, loglen);
}

int main(int argc, char * argv[])
{
	signal(SIGINT, quitter_sock); // permet d'interrompre la communication via le clavier et la fct quitter_sock

	int port = 8888;
	char nom[30];
	
	/* Paramétrage du contexte d'adressage serveur et client*/
	/* Socket information */
	int ssocket;
	int csocket;
    struct sockaddr_in serveur_info, client_info;
    int s_addrlen = sizeof(serveur_info);
    int c_addrlen = sizeof(client_info);
    memset(&serveur_info, 0, s_addrlen);
    memset(&client_info, 0, c_addrlen);
	serveur_info.sin_family=AF_INET;
	serveur_info.sin_port=htons(port);
	serveur_info.sin_addr.s_addr = htonl(INADDR_ANY);
	
	/* variables d'Ocean : */
	char *position = "192N_023E";  // obsolète
	Annuaire mon_annuaire;
	/* initialisation des identifiants libres de l'annuaire */
	strcpy(mon_annuaire.identifiants_pris, "000");
	
	/* s'il manque un argument */
	if (argc!=2)
	{
		fprintf(stderr,"Usage : %s port-number", argv[0]);
		exit(1);
	}

	port = atoi(argv[1]);

	/* si la socket ne se crée pas */
	if ((ssocket=socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("socket rendez-vous");
		exit(1);
	}

	/* on récupère le nom de la mchine sur laquelle le serveur est lancé */
	if (gethostname(nom, 30)==-1)
	{
		perror("Qui suis-je ?");
		exit(1);
	}

	/* affichage d'un identifiant et du nom de la machine sur laquelle le 
	   serveur est lancé */
	printf("Machine: %s\n", nom);


	/* si la socket ne se bind pas avec son contexte d'adressage */
	if (bind(ssocket, (struct sockaddr *) &serveur_info, sizeof(serveur_info))==-1)
	{
		perror("bind");
		exit(1);
	}
	/* si la socket ne rentre pas en mode listen */
	if (listen(ssocket,3)==-1)
	{
		perror("listen");
		exit(1);
	}

	/* Paramètres pour la conversation client-serveur */
	socklen_t addr_size;
	char buffer[1024];
	Bateau * navire1 = (Bateau *)malloc(sizeof(Bateau));
	Bateau * navire2 = (Bateau *)malloc(sizeof(Bateau));
	Bateau * navire3 = (Bateau *)malloc(sizeof(Bateau));
	navire1->identifiant = 0;
	navire2->identifiant = 0;
	navire3->identifiant = 0;
	int navire_courant;
	pid_t childpid;
	/* envoi est un symbole pour accuser récéption. Ça pourrait être autre
	   chose évidemment... */
	char* envoi = "\n";
	// Affichage de l'adresse IP du serveur
    getsockname(ssocket, (struct sockaddr*) &serveur_info, (socklen_t*) &s_addrlen);
    printf("Demarrage serveur sur: %s:%d\n", inet_ntoa(serveur_info.sin_addr), ntohs(serveur_info.sin_port));

    // On initialise un nouveau noeud pour les potentiels clients
    root = newNode(ssocket, inet_ntoa(serveur_info.sin_addr));
    now = root;
	/* le while est la clée du multiplexage : il permet de créer autant de 
	   sockets que de clients qui essayent de se connecter */
	while(1)
	{

		csocket = accept(ssocket, (struct sockaddr*) &client_info, (socklen_t*) &c_addrlen);
		if(csocket < 0)
		{
			exit(1);
		}

		// Affiche l'adresse IP du client
        getpeername(csocket, (struct sockaddr*) &client_info, (socklen_t*) &c_addrlen);
        printf("Le client d'adresse %s:%d est entré\n", inet_ntoa(client_info.sin_addr), ntohs(client_info.sin_port));

        // On ajoute le nouveau client à la liste des clients
        ClientList *c = newNode(csocket, inet_ntoa(client_info.sin_addr));
        c->prev = now;
        now->link = c;
        now = c;
		/**
		 * -------------------------------------
		 * La connection est maintenant établie.
		 * -------------------------------------
		 */

		/* on attribue un identifiant 1, 2 ou 3 au nouveau navire selon que cet 
		   identifiant est libre ou non (vaut 0 ou non) */
		if (navire1->identifiant == 0)
		{
			navire1->identifiant = 1;
			navire_courant = 1;
			mon_annuaire.identifiants_pris[0] = '1';
		}
		else if (navire2->identifiant == 0)
		{
			navire2->identifiant = 2;
			navire_courant = 2;
			mon_annuaire.identifiants_pris[1] = '2';
		}
		else 
		{
			navire3->identifiant = 3;
			navire_courant = 3;
			mon_annuaire.identifiants_pris[2] = '3';
		}
		/* NOTE : l'attribution d;un identifiant devra faire l'objet d'une 
		   fonction */

		/**
		 * La variable "buffer" va recevoir le buffer initial envoyé par le 
		 * client : ce buffer contient les informations du client connecté dans 
		 * la forme d'un string nomPositionDirectionVitesse.
		 *
		 * -----------------------------
		 * Exemple : Arago240N_003Ene4&
		 *
		 */
		recv(csocket, buffer, 1024, 0);
		printf("Cette connexion est lié au code bateau suivant : %s\n", buffer);

		caracteriser_navire(navire1, buffer);

		inscrire_navire(navire1, &mon_annuaire);

		//mon_annuaire.navire1 = *navire1;
		affiche_annuaire(mon_annuaire);

		/**
		 * -------------------------------------------------
		 * Le navire est à présent consigné dans l'annuaire.
		 * -------------------------------------------------
		 */

		if((childpid = fork()) == 0)
		{
			close(ssocket);

			/* initialisation du buffer à zéro pour éviter les mauvaises 
 			   surprises... */
			bzero(buffer, sizeof(buffer));

			while(1)
			{
				recv(csocket, buffer, 1024, 0);

				/* si le serveur reçoit la chaîne "$p", on renvoit la position du bateau correspondant*/
				if ((strcmp(buffer, "$p") == 0) )
				{
					printf("Client %s:%d : *je viens de demander ma position*\n", inet_ntoa(client_info.sin_addr),
		 				   ntohs(client_info.sin_port));
					bzero(buffer, sizeof(buffer));
					/* on copie la position dans le buffer et on l'envoit */
					strncpy(buffer, position, 11);
					send(csocket, buffer, strlen(buffer), 0);
					bzero(buffer, sizeof(buffer));
				}
				/* si le serveur reçoit la chaîne "exit" */
				if (strcmp(buffer, "exit") == 0)
				{
					printf("Disconnected from %s:%d\n", 
						   inet_ntoa(client_info.sin_addr), 
						   ntohs(client_info.sin_port));
					fprintf(stderr,"Cote serveur: fin fils\n");
					break;
				}
				else
				{
					printf("Client:%s:%d : %s\n", inet_ntoa(client_info.sin_addr),
		 				   ntohs(client_info.sin_port), buffer);
					bzero(buffer, sizeof(buffer));
					/* on envoit un accusé de réception au client */
					send(csocket, envoi, sizeof(envoi), 0);
					
				}
				if (strcmp(buffer, "exit") == 0) {
	            printf("Fail to create a socket.");
	        	exit(EXIT_FAILURE);
	        	}
        	}
		}
	}
	return 0;
}