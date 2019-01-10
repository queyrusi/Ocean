#define MAXNOM 31

typedef struct ClientNode { // structure du client
    int data;// Numéro de la socket
    struct ClientNode* prev;
    struct ClientNode* link;
    char ip[16];
    char name[31];
    char position[10]; // obsolète
    /* A ajouter au fur et a mesure pour les bateaux*/
    int vitesse;
    char direction[2];
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
