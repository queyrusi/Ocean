#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#ifndef LIST
#define LIST

/**
 * fonction print_msg 
 * ------------------
 * 
 */
void print_msg(char *talker, char * chat);

/**
 * fonction quitter_sock
 * ------------------
 * 
 */


void quitter_sock(int sig);

/**
 * fonction send_to_all_clients pour le chat et l'envoi de l'ocean toutes les minutes
 * ------------------
 * 
 */
void send_to_all_clients(ClientList *Client_courant, char tmp_buffer[]);


/**
 * fonction gestion_vent pour changer la direction et la force du vent
 * ------------------
 * 
 */
void modif_vent();


/**
 * fonction client_handler pour gerer l'ensemble des connections qui rentrent dans le système
 * 
 */

void client_handler(void *p_client);

#endif 
