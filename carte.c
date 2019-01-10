#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* init_map();
char*ajouter_bateau(char *map, char *position);
int x=0;
int y=0;

int main(int argc, char * argv[]) {
	char *map=init_map();
	map=ajouter_bateau(map,"026N_040E");
	return 0;
}

char *init_map() {
FILE * fic;
	fic = fopen("ocean.txt", "rt");
	char ligne[81];

	int xi = 0;
	int yi=0;

	int compteur =0;

	//char map[100][100];
	

	while (compteur!=2){
		fgets(ligne,100,fic);
		if (compteur ==0) {
			x=atoi(ligne)+2;
		}
		else {
			y=atoi(ligne)+2;
		}
		compteur += 1;
	}

	char* map = malloc(x*y*sizeof(char));
	for (xi=0 ; xi<x ;xi++){
		for (yi=0 ; yi<y ; yi++) {
			map[xi+yi*x]='-';
		}
	}
	while (fgets(ligne,100,fic) != NULL) {
		if (compteur%2==0) {
			xi=x-atoi(ligne)+1;
		}
		else {
			yi=atoi(ligne)+1;
			map[xi+yi*x] = 'R';
		}
	compteur += 1;
	
	}
	for (xi=0 ; xi<x ;xi++){	
		for (yi=0 ; yi<y ; yi++) {
			if(xi==0||yi==0||xi==x-1||yi==y-1){
				printf("*");
			}
			else{
				printf("%c", map[xi+yi*x]);
			}
		}
		printf("\n");
	}
	return map;
}

char *ajouter_bateau(char *map, char *position) {
	int i;
	char *nord;
	int nordint;
	char *est;
	int estint;
	char*map2=map;
	"""026N_132E""";
	nord[0]= position[0];
	nord[1]= position[1];
	nord[2]= position[2];
	nordint=x-atoi(nord);
	est[0]= position[0];
	est[1]= position[1];
	est[2]= position[2];
	estint=atoi(nord);
	map2[100-26+80*100]='B';
	int xi;
	int yi;
	for (xi=0 ; xi<x ;xi++){	
		for (yi=0 ; yi<y ; yi++) {
			if(xi==0||yi==0||xi==x-1||yi==y-1){
				printf("*");
			}
			else{
				printf("%c", map2[xi+yi*x]);
			}
		}
		printf("\n");
	}
	return map2;
}

// est_place_libre(*position[10], listeClients (annuaires)) 
// 1 si ok ; 0 sinon
// distingue si 1er navire connecté : annuaire vide





