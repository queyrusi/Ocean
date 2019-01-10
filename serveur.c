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
#include <errno.h>
#include <netdb.h>
#include <ctype.h>


#include "structures.h"
#include "fonctions_serveur.c"
#include "serveur.h"
#include "donnees.h"
#include "string.h"

// Global variables
int ssocket = 0;
int csocket = 0;
int server_sockfd = 0, client_sockfd = 0;
ClientList *root, *now;


/**
 * fonction print_msg 
 * ==================
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
 * fonction quitter_sock
 * =====================
 * 
 */

void quitter_sock(int sig) 
{// On cherche ici a fermer toutes les sockets lorsqu'on va shutdown le systeme
    ClientList *tmp;
    while (root != NULL) 
    {// Tant qu'on a pas fermer la racine on continue, erreur de compilation lorsqu'on ferme le serveur, à corriger
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
 * fonction send_to_all_clients 
 * ============================
 *
 * pour le chat et l'envoi de l'ocean toutes les minutes
 * 
 */
void send_to_all_clients(ClientList *Client_courant, char tmp_buffer[]) {// Permet d'envoyer un message à tous les clients d'un coup
    ClientList *tmp = root->link;
    while (tmp != NULL) 
    {// Tant qu'il reste des liens à explorer
        if (Client_courant->data != tmp->data) 
        { // On envoi à tous les clients sauf à lui-meme
            printf("Envoyer a la socket %d: \"%s\" \n", tmp->data, tmp_buffer);
            send(tmp->data, tmp_buffer, LENGTH_SEND, 0);
        }
        tmp = tmp->link;
    }
}

/**
 * fonction gestion_vent 
 * =====================
 *
 * pour changer la direction et la force du vent
 * 
 */
void modif_vent()
{
	char choix_vent[3];
    choix_vent[0] = 0; // on initialise le vent à 0
    char message_console[LENGTH_MSG] = {};
    while(1)
    {
	    char direction[3];
		if (fgets(message_console, 32, stdin) != NULL && strcmp(left(message_console, 4),"vent") ==0 ) 
		{
	        choix_vent[0]= message_console[4];
	        choix_vent[1]= message_console[5];
	        printf("Félicitation, vous venez de changer la force et la direction du vent, elle est de :%s\n", choix_vent);
	        //fflush(stdin);
	    }
	    else
	    {
	    	printf("Malheureusement, la forme pour changer le vent n'est pas la bonne\n");
	    	//fflush(stdin);
	    }
	}
}





/**
 * fonction client_handler
 * =======================
 *
 * pour gerer l'ensemble des connections qui rentrent dans le système
 * 
 */

void client_handler(void *p_client) 
{ //gestionnaire de client, permet au serveur de gerer la liste de client
    int control_false = 0; // permet de controller si il n'y a pas d'erreur, passe a 1 si c'est le cas
    char code_bateau_entrant[LENGTH_NAME] = {};
    char recv_buffer[LENGTH_MSG] = {};
    char send_buffer[LENGTH_SEND] = {};
    ClientList *Client_courant = (ClientList *)p_client;

    char position[10] = {};  // obsolète, il faut gerer ça dans la stucture client
    char direction[3] = {};

    // Nomage des clients qui rentrent pour les iidentifier dans le serveur
    
    if (recv(Client_courant->data, code_bateau_entrant, LENGTH_NAME, 0) <= 0 || est_bateau(code_bateau_entrant) == 0 ) 
    {
        printf("%s Erreur code bateau \n", Client_courant->ip);//si on ne recoit pas le bon code bateau, erreur
        control_false = 1;
    } 
    else 
    {
    	char * nom_bat = nom_bateau(code_bateau_entrant);
    	char * position = init_position(code_bateau_entrant);
    	char * direction = init_direction(code_bateau_entrant);
    	strncpy(Client_courant->position, position, 10);
    	strncpy(Client_courant->direction, direction, 10);
        strncpy(Client_courant->name, nom_bat, LENGTH_NAME); // On copie le nom que le client a rentre dans une chaine de char usable
        free(nom_bat);
        free(position);
        free(direction);
        printf("Le bateau %s(%s)(%d) a rejoint l'ocean à la position %s en direction %s.\n", Client_courant->name, Client_courant->ip, Client_courant->data, Client_courant->position,Client_courant->direction);
        bzero(send_buffer,sizeof(send_buffer));
        sprintf(send_buffer, "Le bateau%s(%s) a rejoint l'ocean à la position %s en direction %s", Client_courant->name, Client_courant->ip, Client_courant->position, Client_courant->direction);// On met dans le buffer et on envoi a tout le monde que le nouveau bateau a rejoint l'ocean
        send_to_all_clients(Client_courant, send_buffer); // on envoi à tout le monde que un client vient de rejoindre la salle
    }

    // Conversation entre les clients 
    while (1) 
    {
        if (control_false) 
        {
            break;
        }
    	
        
        int receive = recv(Client_courant->data, recv_buffer, LENGTH_MSG, 0);
        if (receive > 0) 
	    {
		    if (strcmp(recv_buffer, "P") == 0 )
		    {
		    	printf("Le bateau %s(%s)(%d) a demandé sa position'.\n", Client_courant->name, Client_courant->ip, Client_courant->data);
				/* on copie la position dans le buffer et on l'envoit */
				sprintf(send_buffer, "%s, vous êtes à la position %s", Client_courant->name, Client_courant->position);
				//strncpy(buffer, position, 11);
				send(Client_courant->data, send_buffer, strlen(send_buffer), 0);
		    	/* on copie la position dans le buffer et on l'envoit */
		    	sprintf(send_buffer, "%s(%s) est à la position %s", Client_courant->name, Client_courant->ip, Client_courant->position);
	            }
	        else if (strlen(recv_buffer) == 0) 
	        {
	                continue;
	        }
            sprintf(send_buffer, "%s：%s from %s", Client_courant->name, recv_buffer, Client_courant->ip);

	    } 
        else if (receive == 0 || strcmp(recv_buffer, "exit") == 0) 
        {
            printf("%s(%s)(%d) a quitté l'océan\n", Client_courant->name, Client_courant->ip, Client_courant->data);
            sprintf(send_buffer, "%s(%s) quitte l'océan", Client_courant->name, Client_courant->ip);
            control_false = 1;
		} 
		else 
		{
	        printf("Erreur: -1\n");
	        control_false = 1;
	    }
	        send_to_all_clients(Client_courant, send_buffer);
    }

    // Pour enlever un noeud
    close(Client_courant->data);
    if (Client_courant == now) 
    { //on enleve un noeud situé au bord
        now = Client_courant->prev;
        now->link = NULL;
    } 
    else 
    { // on enleve un noeud situé au millieu
        Client_courant->prev->link = Client_courant->link;
        Client_courant->link->prev = Client_courant->prev;
    }
    //free(Client_courant);
}

/**
 * fonction client2code
 * ====================
 *
 * Retourne un code navire à partir d'un client.
 *
 * author: Simon
 *
 *
 * Paramètre:
 * ----------
 * Client_node * client : client dont on veut obtenir le code navire
 *
 * Retourne:
 * ---------
 * char * code_client : code client (sans le nom) de la forme 231N_003En21
 *
 */
char * client2code(Client_node * client)
{
	char[13] = code_client;
	int now_x = courant->x;
	int now_y = courant->y;
	int now_v = courant->vitesse;
	char direction = courant->direction;

	if (now_y > 99)
	{
		char now_y_str[4];
	}
	else
	{
		char now_y_str[3];
	}
	if (now_x > 99)
	{
		char now_x_str[4];
	}
	else
	{
		char now_x_str[3];
	}	

	sprintf(now_y_str, "%d", now_x);
	strncpy(code_client, now_y_str, sizeof(now_y_str));
	bzero(now_y_str);
	strcat(code_client, "N_");

	sprintf(now_x_str, "%d", now_x);
	strcat(code_client, now_x_str);
	bzero(now_x_str);
	strcat(code_client, 'E');

	strcat(code_client, direction);

	if (now_v > 9)
	{
		if (now_v > 99)
		{
			perror("client_2_code lit une vitesse à trois chiffres");
			exit(EXIT_FAILURE);

		}
		char now_v_str[3];
	}
	else
	{
		char now_v_str[2];
	}

	sprintf(now_v_str, "%d", now_v);
	strcat(code_client, now_v_str);
	bzero(now_v_str);

	return code_client;
}

/**
 * fonction transmettre_map
 * ========================
 * 
 * Transmet à tous les clients un buffer avec les informations de tous les  
 * clients, toutes les minutes
 *
 * author: Simon
 *
 */
void transmettre_map()
{
	int k;
	char[40] buffer_transmis;
	char * commande_map = "$p";
	strncpy(buffer_transmis, commande_map, 2);
	char[13] code_bateau;

	/* il n'y a aucun client dans l'océan */
	if (ClientList == NULL)
	{
		perror("aucune liste client\n");
        exit(EXIT_FAILURE);
	}

	courant = now;  // utile ?

	/* tant que la pile n'est pas vide */
	do
	{
		bzero(code_bateau, sizeof(code_bateau));
		code_bateau = client2code(courant);
		strcat(buffer_transmis, code_bateau);

		/* il reste des navires dans l'océan */
		if (now->prev != NULL)
		{
			/* les codes navire sont séparés par une virgule */
			strcat(buffer_transmis, ',');
			courant = now->prev;
		}
		else /* on a inscrit le dernier code dans le buffer */
		{
			/* caractère de fin */
			strcat(buffer_transmis, '&');
			break;
		}
	}while (now->prev != NULL)

	/* on attend une minute */
	sleep(60);

	/* send_to_all envoit à tout le monde sauf à lui même... */
	send_to_all_clients(now, buffer_transmis);
	send(now, buffer_transmis);
}


int main(int argc, char * argv[])
{
    signal(SIGINT, quitter_sock); // permet d'interrompre la communication via le clavier et la fct quitter_sock
    int port;
	char nom[30];
    // Creation de la socket
    ssocket = socket(AF_INET , SOCK_STREAM , 0);
    if (ssocket == -1) 
    {
        printf("Erreur creation socket");
        exit(EXIT_FAILURE);
    }
	
	/* Paramétrage du contexte d'adressage serveur et client*/
	/* Socket information */
	port = atoi(argv[1]);
    struct sockaddr_in serveur_info, client_info;
    int s_addrlen = sizeof(serveur_info);
    int c_addrlen = sizeof(client_info);
    memset(&serveur_info, 0, s_addrlen);
    memset(&client_info, 0, c_addrlen);
	serveur_info.sin_family=PF_INET;
	serveur_info.sin_port=htons(port);
	serveur_info.sin_addr.s_addr = htonl(INADDR_ANY);

   /* variables d'Ocean : */
	Annuaire mon_annuaire;
	/* initialisation des identifiants libres de l'annuaire */
	strcpy(mon_annuaire.identifiants_pris, "000");
	
	/* s'il manque un argument */
	if (argc!=2)
	{
		fprintf(stderr,"Usage : %s port-number", argv[0]);
		exit(1);
	}


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
	printf("-------------------------\n");
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

    while (1) 
    {
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


        // On ajoute le nouveau client à la liste des clients
        ClientList *c = newNode(csocket, inet_ntoa(client_info.sin_addr));
        c->prev = now;
        now->link = c;
        now = c;

        // A mettre dans le handler
		//recv(csocket, buffer, 1024, 0);
		//printf("Cette connexion est lié au code bateau suivant : %s\n", buffer);

		//affiche_annuaire(mon_annuaire);

        pthread_t gestion_client_menu;
        if (pthread_create(&gestion_client_menu, NULL, (void *)client_handler, (void *)c) != 0) 
        {
            perror("Pthread erreur!\n");
            exit(EXIT_FAILURE);
        }
        pthread_t gestion_vent;
        if (pthread_create(&gestion_vent, NULL, (void * )modif_vent, (void *)c) != 0)
        {
            perror("Pthread erreur!\n");
            exit(EXIT_FAILURE);
        }

        pthread_t transmission_map;
        if (pthread_create(&transmission_map, NULL, (void * )transmettre_map, (void *)c) != 0) 
        {
            perror("Pthread erreur!\n");
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}
