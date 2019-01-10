#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#ifndef LIST
#define LIST

typedef struct ClientNode { // structure du client
    int data;// code bateau
    struct ClientNode* prev;
    struct ClientNode* link;
    char ip[16];
    char name[31];
    char position[10]; // obsolète
    /* A ajouter au fur et a mesure pour les bateaux*/

    //int identifiant;
    //int * pidentifiant;
    /*beaucoup plus pratique pour l'envoi du buffer initial */
    //char vitesse[2];  
    /* ajout de la même vitesse en int pour les "vraies" 
    manipulations */
    int vitesse;
    char direction[2];
    //char position[10];
    int x;
    int y;
} ClientList;

ClientList *newNode(int sockfd, char* ip) { //creation d'un espace pour un potentiel client
    ClientList *np = (ClientList *)malloc( sizeof(ClientList) );
    np->data = sockfd; //code bateau
    np->prev = NULL;
    np->link = NULL;
    strncpy(np->ip, ip, 16);
    strncpy(np->name, "NULL", 5);
    return np;
}


char * substr(char *chaineSource,int pos,int len) { 
  // Retourne la sous-chaine de la chaine chaineSource 
  // depuis la position pos sur la longueur len
 
  char * dest=NULL;                        
  if (len>0) {                  
    /* allocation et mise à zéro */          
    dest = (char *)calloc(len+1, 1);      
    /* vérification de la réussite de l'allocation*/  
    if(NULL != dest) {
        strncat(dest,chaineSource+pos,len);            
    }
  }                                       
  return dest;                            
}
 
char * mid(char *chaineSource, int pos) {
      // Retourne la sous-chaine de chaineSource depuis 
      // la postion pos jusqu'au reste de chaineSource.
      // Si la position pos est supperieur à la longueur de chaineSource, 
      // on retourne chaineSource.
      return (pos>strlen(chaineSource))? chaineSource : substr(chaineSource, pos, strlen(chaineSource));
}
 
char * left(char *chaineSource, int len){
      // Retourne les len caractères de gauche de chaineSource.
      // Si la longueur len est suppérieur ou égale à la longeur de chaineSource,
      // on retourne chaineSource. Si len est 0, on retourne '\0'. 
      return (len>=strlen(chaineSource))? chaineSource : substr(chaineSource, 0, len);
}
 
char * right(char *chaineSource, int len){
      // Retourne les len caractères de droite de chaineSource.
      // Si la longueur len est suppérieur ou égale à la longueur de chaineSource,
      // on retourne chaineSource. Si len est 0, on retourne '\0'.
      return (len>=strlen(chaineSource))? chaineSource : substr(chaineSource, strlen(chaineSource)-len, len);
}

#endif 
