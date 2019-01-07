
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    FILE* fic;
    char ligne[81];
    short nb_lignes_lues, nb_val_lues;
    int xmax = 21;
    int ymax = 22;
    int WindF = 1;
    char WindD[15];
    int *rochers = NULL;
    int nombreRochers = 0;
    
    fic = fopen("ocean.txt", "rt");
    
    printf("Combien il y a de rochers ?");
    scanf("%d", &nombreRochers);
    if (nombreRochers > 0)
    {
        rochers = malloc(2*nombreRochers * sizeof(int));
        if (rochers == NULL)
        {
            exit(0);
        }
    }
    

    if (fic==NULL)
    {
        puts("Probleme d'ouverture du fichier texte");
        exit(0);
    }
    
    while (fgets (ligne, 81, fic) != NULL)
    {
        nb_lignes_lues++;
        if (nb_lignes_lues <=1)
        {
            nb_val_lues = sscanf (ligne, "%d%d%d%s", &xmax, &ymax, &WindF, WindD);
            if (nb_val_lues!=4)
                printf ("PROBLEME Ligne %2hd\n", nb_lignes_lues);
            else
                printf("(Donnees extraites Ligne %2hd) %d %d %d %s\n", nb_lignes_lues,xmax, ymax, WindF, WindD);
        }
        else
        {
            sscanf(ligne, "%d", &rochers[nb_lignes_lues-2]);
        }
    }
        
    //printf("%d\n", xmax);
    //printf("%d\n", rochers[0]);
    //printf("%d\n", rochers[1]);
    //printf("%d\n", rochers[2]);
    //printf("%d\n", rochers[3]);
    printf("%d\n", rochers[4]);
    printf("%d\n", rochers[5]);
    free(rochers);
    fclose(fic);
    return 0;
}
