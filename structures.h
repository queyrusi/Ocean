#define MAXNOM 30

/**
 * structure bateau
 * ----------------
 */
typedef struct bateau
{
        char nom[MAXNOM];
        int identifiant;
        int * pidentifiant;
        /*beaucoup plus pratique pour l'envoi du buffer initial */
        char vitesse[2];  
        /* ajout de la même vitesse en int pour les "vraies" 
        manipulations */
        int vitesse_int;
        char direction[3];
        char position[10];
        int x;
        int y;
} Bateau;
 
 
/**
 * structure annuaire
 * ------------------
 */
typedef struct annuaire
{
	char identifiants_pris[4];
    struct bateau navire1;
    struct bateau navire2;
    struct bateau navire3;
} Annuaire;

