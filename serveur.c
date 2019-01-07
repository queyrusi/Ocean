#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "structures.h"
#include "fonctions_serveur.h"
#include "serveur.h"
#include "donnees.h"

// Global variables
int ssocket = 0;
int csocket = 0;
int server_sockfd = 0, client_sockfd = 0;
ClientList *root, *now;


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

char *position = "192N_023E";  // obsolète
/**
 * fonction quitter_sock
 * ------------------
 * 
 */

void quitter_sock(int sig) {// On cherche ici a fermer toutes les sockets lorsqu'on va shutdown le systeme
    ClientList *tmp;
    while (root != NULL) {// Tant qu'on a pas fermer la racine on continue
        printf("\nFermeture Socket: %d\n", root->data);
        close(root->data); // On ferme la socket sur laquelle on se situe puis on remonte les liens
        tmp = root;
        root = root->link;
        free(tmp);
    }
    printf("Au revoir\n");// On a tout fermer, meme le serveur lui meme
    exit(EXIT_SUCCESS);
}

/**
 * fonction send_to_all_clients pour le chat et l'envoi de l'ocean toutes les minutes
 * ------------------
 * 
 */
void send_to_all_clients(ClientList *Client_courant, char tmp_buffer[]) {// Permet d'envoyer un message à tous les clients d'un coup
    ClientList *tmp = root->link;
    while (tmp != NULL) {// Tant qu'il reste des liens à explorer
        if (Client_courant->data != tmp->data) { // On envoi à tous les clients sauf à lui-meme
            printf("Envoyer a la socket %d: \"%s\" \n", tmp->data, tmp_buffer);
            send(tmp->data, tmp_buffer, LENGTH_SEND, 0);
        }
        tmp = tmp->link;
    }
}

/**
 * fonction gestion_vent pour changer la direction et la force du vent
 * ------------------
 * 
 */
void modif_vent(){
	char choix_vent[3];
    choix_vent[0] = 0; // on initialise le vent à 0
    char message_console[LENGTH_MSG] = {};
    while(1){
		if (fgets(message_console, 32, stdin) != NULL && strcmp(left(message_console, 4),"vent") ==0 ) {
	                choix_vent[0]= message_console[4];
	                choix_vent[1]= message_console[5];
	                printf("Félicitation, vous venez de changer la force et la direction du vent, elle est de :%s\n", choix_vent);
	                //fflush(stdin);
	    	}
	    else{
	        	printf("Malheureusement, la forme pour changer le vent n'est pas la bonne\n");
	        	//fflush(stdin);
	        	
	    	}
	}
}

/**
 * fonction client_handler pour gerer l'ensemble des connections qui rentrent dans le système
 * 
 */

void client_handler(void *p_client) { //gestionnaire de client, permet au serveur de gerer la liste de client
    int control_false = 0; // permet de controller si il n'y a pas d'erreur, passe a 1 si c'est le cas
    char code_bateau_entrant[LENGTH_NAME] = {};
    char recv_buffer[LENGTH_MSG] = {};
    char send_buffer[LENGTH_SEND] = {};
    ClientList *Client_courant = (ClientList *)p_client;

    char *position = "192N_023E";  // obsolète, il faut gerer ça dans la stucture client

    // Nomage des clients qui rentrent pour les iidentifier dans le serveur
    
    if (recv(Client_courant->data, code_bateau_entrant, LENGTH_NAME, 0) <= 0 || est_bateau(code_bateau_entrant) == 0 ) {
        printf("%s Erreur code bateau \n", Client_courant->ip);//si on ne recoit pas le bon code bateau, erreur
        control_false = 1;
    } else {
    	char * nom_bat = nom_bateau(code_bateau_entrant);
        strncpy(Client_courant->name, nom_bat, LENGTH_NAME); // On copie le nom que le client a rentre dans une chaine de char usable
        free(nom_bat);
        printf("Le bateau %s(%s)(%d) a rejoint l'ocean'.\n", Client_courant->name, Client_courant->ip, Client_courant->data);
        sprintf(send_buffer, "Le bateau%s(%s) a rejoint l'ocean", Client_courant->name, Client_courant->ip);// On met dans le buffer et on envoi a tout le monde que le nouveau bateau a rejoint l'ocean
        send_to_all_clients(Client_courant, send_buffer); // on envoi à tout le monde que un client vient de rejoindre la salle
    }

    // Conversation entre les clients 
    while (1) {
        if (control_false) {
            break;
        }
    	
        	
        
        int receive = recv(Client_courant->data, recv_buffer, LENGTH_MSG, 0);
        if (receive > 0) {
	    if (strcmp(recv_buffer, "P") == 0 ){
	    	printf(" Le bateau %s(%s)(%d) a demandé sa position'.\n", Client_courant->name, Client_courant->ip, Client_courant->data);
			/* on copie la position dans le buffer et on l'envoit */
			sprintf(send_buffer, "%s, vous êtes à la position %s", Client_courant->name, position);
			//strncpy(buffer, position, 11);
			send(Client_courant->data, send_buffer, strlen(send_buffer), 0);
	    	/* on copie la position dans le buffer et on l'envoit */
	    	sprintf(send_buffer, "%s(%s) est à la position %s", Client_courant->name, Client_courant->ip, position);
            }
        else if (strlen(recv_buffer) == 0) {
                continue;
            }
            sprintf(send_buffer, "%s：%s from %s", Client_courant->name, recv_buffer, Client_courant->ip);

        } else if (receive == 0 || strcmp(recv_buffer, "exit") == 0) {
            printf("%s(%s)(%d) part du chat\n", Client_courant->name, Client_courant->ip, Client_courant->data);
            sprintf(send_buffer, "%s(%s) part du chat", Client_courant->name, Client_courant->ip);
            control_false = 1;

	} else {
            printf("Erreur: -1\n");
            control_false = 1;
        }
        send_to_all_clients(Client_courant, send_buffer);
    }

    // Pour enlever un noeud
    close(Client_courant->data);
    if (Client_courant == now) { //on enleve un noeud situé au bord
        now = Client_courant->prev;
        now->link = NULL;
    } else { // on enleve un noeud situé au millieu
        Client_courant->prev->link = Client_courant->link;
        Client_courant->link->prev = Client_courant->prev;
    }
    free(Client_courant);
}

int main(int argc, char * argv[])
{
    signal(SIGINT, quitter_sock); // permet d'interrompre la communication via le clavier et la fct quitter_sock
    int port = 8881;
	char nom[30];
    // Creation de la socket
    ssocket = socket(AF_INET , SOCK_STREAM , 0);
    if (ssocket == -1) {
        printf("Erreur creation socket");
        exit(EXIT_FAILURE);
    }
	
	/* Paramétrage du contexte d'adressage serveur et client*/
	/* Socket information */
    struct sockaddr_in serveur_info, client_info;
    int s_addrlen = sizeof(serveur_info);
    int c_addrlen = sizeof(client_info);
    memset(&serveur_info, 0, s_addrlen);
    memset(&client_info, 0, c_addrlen);
	serveur_info.sin_family=PF_INET;
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

	// Affichage de l'adresse IP du serveur
    getsockname(ssocket, (struct sockaddr*) &serveur_info, (socklen_t*) &s_addrlen);
    printf("Demarrage serveur sur: %s:%d\n", inet_ntoa(serveur_info.sin_addr), ntohs(serveur_info.sin_port));

    // On initialise un nouveau noeud pour les potentiels clients
    root = newNode(ssocket, inet_ntoa(serveur_info.sin_addr));
    now = root;

    while (1) {
        csocket = accept(ssocket, (struct sockaddr*) &client_info, (socklen_t*) &c_addrlen);

        // Affiche l'adresse IP du client
        getpeername(csocket, (struct sockaddr*) &client_info, (socklen_t*) &c_addrlen);
        printf("Le Client d'adresse %s:%d est entré\n", inet_ntoa(client_info.sin_addr), ntohs(client_info.sin_port));
        printf("Le vent a été initialisé à 0, pour le modifier, rentrez : ventNew_forceNew_dir\n");
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



        // On ajoute le nouveau client à la liste des clients
        ClientList *c = newNode(csocket, inet_ntoa(client_info.sin_addr));
        c->prev = now;
        now->link = c;
        now = c;

        // A mettre dans le handler
		//recv(csocket, buffer, 1024, 0);
		//printf("Cette connexion est lié au code bateau suivant : %s\n", buffer);

		//caracteriser_navire(navire1, buffer);

		//inscrire_navire(navire1, &mon_annuaire);

		//mon_annuaire.navire1 = *navire1;
		//affiche_annuaire(mon_annuaire);

		/**
		 * -------------------------------------------------
		 * Le navire est à présent consigné dans l'annuaire.
		 * -------------------------------------------------
		 */
        pthread_t gestion_client_menu;
        if (pthread_create(&gestion_client_menu, NULL, (void *)client_handler, (void *)c) != 0) {
            perror("Pthread erreur!\n");
            exit(EXIT_FAILURE);
        }
        pthread_t gestion_vent;
        if (pthread_create(&gestion_vent, NULL, (void * )modif_vent, (void *)c) != 0) {
            perror("Pthread erreur!\n");
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}
