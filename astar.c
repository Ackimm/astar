#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>
#include <math.h>

// quelques codes couleur trouvés sur le net
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"


struct position {
    int x;
    int y;
};
typedef struct position position;

struct noeuds {
    struct position position;
    float cout_h;
    float cout_g;
    float cout_f;
    struct noeuds *parent;
    struct noeuds *suivant;
};
typedef struct noeuds noeuds;

struct listeNoeuds {
    int quantite;
    struct noeuds *debutListe;
};
typedef struct listeNoeuds listeNoeuds;


void afficherGraphe(char **graphe, int nbLignes, int nbColonnes) {
  
    for (int i = 0; i < nbLignes; i++) {
        for (int j = 0; j < nbColonnes; j++) {
            if (graphe[i][j] == 'E') {
                printf(ANSI_COLOR_BLUE" %c "ANSI_COLOR_RESET, graphe[i][j]);  // Bleu pour les noeuds explorés
            } else if (graphe[i][j] == 'D') {
                printf(ANSI_COLOR_GREEN" %c "ANSI_COLOR_RESET, graphe[i][j]);  // Vert pour le départ
            } else if (graphe[i][j] == 'A') {
                printf(ANSI_COLOR_RED" %c "ANSI_COLOR_RESET, graphe[i][j]);  // Rouge pour l'arrivée
            } else if (graphe[i][j] == 'o') {
                printf(" %c ", graphe[i][j]);  // Blanc pour les obstacles
            } else if (graphe[i][j] == '*') {
                printf(ANSI_COLOR_YELLOW" %c "ANSI_COLOR_RESET, graphe[i][j]);  // Jaune pour le chemin final
            } else {
                printf(" %c ", graphe[i][j]);
            }
        }
        printf("\n");
    }
}


// Heuristique admissible et avec recherche rapide
float distanceEuclidienne(int x1, int y1, int x2, int y2) {
    return ((float)x1 - (float)x2)*((float)x1 - (float)x2) + ((float)y1 - (float)y2)*((float)y1 - (float)y2);
}


// Heuristique admissible mais avec recherche plus longue
float distanceManhattan(int x1, int y1, int x2, int y2) {
    return (abs((float)x2 - (float)x1) + abs((float)y2 - (float)y1));
}

// Heuristique avec surévaluation de la distance entre le noeud en cours d'anlyse et le noeud d'arrivée
float distanceAvecSurevaluation(int x1, int y1, int x2, int y2) {
    return (abs((float)x2 - (float)x1-353)) + abs((float)(y2*20) - (float)(y1-10));
}

position trouverCoordonnees(char ***graphe, int nbLignes, int nbColonnes, char c) {
    position position_recherchee;
    for (int i = 0; i < nbLignes; i++) {
        for (int j = 0; j < nbColonnes; j++) {
            if ((*graphe)[i][j] == c) {
                position_recherchee.x = j;
                position_recherchee.y = i;
                return position_recherchee;
            }
        }
    }
    return position_recherchee;
}

noeuds* creer_noeud(int x, int y, int cout_g, float cout_h, noeuds* parent) {
    noeuds *noeud = (noeuds*)malloc(sizeof(noeuds));
  
    noeud->position.x = x;
    noeud->position.y = y;
    noeud->cout_g = cout_g;
    noeud->cout_h = cout_h;
    noeud->cout_f = cout_g + cout_h;
    noeud->parent = parent;
    noeud->suivant = NULL;
    return noeud;
}

void initialiser_liste(listeNoeuds *liste) {
        liste->quantite = 0;
        liste->debutListe = NULL;
}

void inserer_noeud(listeNoeuds *liste, noeuds *noeud) { // pour insérer dans la liste fermé (pas besoin de file de priorité)
    noeud->suivant = liste->debutListe;
    liste->debutListe = noeud;
    liste->quantite++;
}


void inserer_noeud_au_bon_emplacement(listeNoeuds *liste, noeuds *noeud) {

    // Si la liste est vide ou le nouveau noeud a un coût f plus petit ou égal à celui de la tête alors je peux ajouter directement en tête de liste
    if (liste->debutListe == NULL || noeud->cout_f <= liste->debutListe->cout_f) {
        noeud->suivant = liste->debutListe;
        liste->debutListe = noeud;
        liste->quantite++;
        return;
    }
    else {

        noeuds *courant = liste->debutListe;
        while (courant->suivant != NULL && courant->suivant->cout_f < noeud->cout_f) { // je parcours la liste à la recherche du bon emplacement
            courant = courant->suivant;
        }

        noeud->suivant = courant->suivant;
        courant->suivant = noeud;
        liste->quantite++;
    }
}

noeuds* extraire_tete_de_liste(listeNoeuds *liste) {
      
    noeuds *meilleur_noeud = liste->debutListe;
    liste->debutListe = meilleur_noeud->suivant;
    liste->quantite--;
    return meilleur_noeud;
}



bool noeud_present(listeNoeuds *liste, int x, int y) {
    noeuds *courant = liste->debutListe;
    while (courant != NULL) {
        if (courant->position.x == x && courant->position.y == y) 
            return true;
        courant = courant->suivant;
    }
    return false;
}

void afficherElementsListe(listeNoeuds liste){
        noeuds *courant = liste.debutListe;
        while (courant != NULL) {
            printf("(%i, %i)", courant->position.x, courant->position.y);
            if (courant->suivant != NULL)
                printf(" -> ");
            courant = courant->suivant;
        }
        printf("\n");   
}




void majGraphe(char ***graphe, int nbLignes, int nbColonnes, noeuds *noeud) {
    
    for (int i = 0; i < nbLignes; i++) {
        for (int j = 0; j < nbColonnes; j++) {
            if (i == noeud->position.y && j == noeud->position.x && (*graphe)[i][j] != 'A' && (*graphe)[i][j] != 'D')
                (*graphe)[i][j] = 'E';
        }
    }
    system("clear");
    afficherGraphe(*graphe, nbLignes, nbColonnes);
    usleep(25000); 
}


// ajoute à une liste chainée les éléments du chemin en partant de l'arrivée trouvé par astar et en remontant parent par parent jusqu'au noeud de départ (ce qui donne la liste dans le bon sens)
listeNoeuds reconstruire_chemin(noeuds *noeud, char ***graphe, int nbLignes, int nbColonnes) {
    
    listeNoeuds listeCheminFinal;
    initialiser_liste(&listeCheminFinal);
    noeuds *temp;
    temp = noeud;

    while (temp != NULL) {
        inserer_noeud(&listeCheminFinal, temp); 
        temp = temp->parent;
    }


    // partie relative à l'affichage grapheique et textuelle de la liste
    noeuds *courant = listeCheminFinal.debutListe;
    
        while (courant != NULL) {
        if ((*graphe)[courant->position.y][courant->position.x] != 'D' && 
            (*graphe)[courant->position.y][courant->position.x] != 'A') {
            (*graphe)[courant->position.y][courant->position.x] = '*';
            system("clear");
            afficherGraphe(*graphe, nbLignes, nbColonnes);
        }

        courant = courant->suivant;
        usleep(25000);
    }
    printf("Chemin trouvé : \n");
    afficherElementsListe(listeCheminFinal);

    return listeCheminFinal;
}
    


listeNoeuds aStar(char ***graphe, position depart, position arrivee, int nbLignes, int nbColonnes, listeNoeuds *cheminFinal) {
    listeNoeuds listeFermee; // liste des éléments déjà analysés et en cours d'analyse
    listeNoeuds listeOuverte; // liste des éléments qu'on va analyser triés de manière croissante selon leur coût f
    initialiser_liste(&listeFermee);
    initialiser_liste(&listeOuverte);

    // initialisation de la boucle principale de l'algorithme
    noeuds *noeud_depart = creer_noeud(depart.x, depart.y, 0, distanceEuclidienne(depart.x, depart.y, arrivee.x, arrivee.y), NULL);
    inserer_noeud_au_bon_emplacement(&listeOuverte, noeud_depart); // ici j'aurais aussi pu utiliser ma fonction insere_noeud plus basique 
                                                                   //mais je trouve ça plus cohérent de garder la même fonction pour l'ensemble des éléments de la liste ouverte

    while (listeOuverte.debutListe != NULL ) { // boucle d'astar active tant qu'il y a des candidats dans la liste ouverte (condition d'arrêt : plus de candidat dans la liste ouverte)
        
        noeuds *courant = extraire_tete_de_liste(&listeOuverte); // on extrait le noeud avec le meilleur coût f (f = g + h) systématiquement placé en tête de la liste ouverte

        inserer_noeud(&listeFermee, courant); // on met ce noeud dans la liste fermée (des noeuds analysés)

        if (courant->position.x == arrivee.x && courant->position.y == arrivee.y) { //condition d'arrêt alternative (quand on trouve l'arrivée)
        return reconstruire_chemin(courant, graphe, nbLignes, nbColonnes); 
        }

        majGraphe(graphe, nbLignes, nbColonnes, courant); // mise à jour et affichage du tableau avec des caractères E pour symbolisé les éléments expolorés présents dans la liste fermée
   
        int directions[8][2] = {{1, 0}, {1, 1}, {0, 1}, {-1, 1}, {-1, 0}, {-1, -1}, {0, -1}, {1, -1}}; 
        for (int i = 0; i < 8; i++) { // on vérifie les huit cases autour du noeud courant et on ajoute dans la liste ouverte les candidats potentiels 
            int voisin_x = courant->position.x + directions[i][0];
            int voisin_y = courant->position.y + directions[i][1];

            if ((voisin_x < 0 || voisin_y < 0 || voisin_x >= nbColonnes || voisin_y >= nbLignes)
                || ((*graphe)[voisin_y][voisin_x] == 'o')
                || (noeud_present(&listeFermee, voisin_x, voisin_y))
                || (noeud_present(&listeOuverte, voisin_x, voisin_y))){
                continue; // passe directement au voisin suivant si le voisin en cours est hors graphee, obstacle ou déjà analysé / en cours d'analyse
            }
   
            float cout_g;
            if (directions[i][0]==directions[i][1])    
                cout_g = courant->cout_g + sqrt(2); // facultatif dans l'idée d'appliquer ceci à un jeu-vidéo, il ne faudrait pas donner trop d'avantage au déplacement en diagonal 
            else
                cout_g = courant->cout_g + 1;

            float cout_h = distanceEuclidienne(voisin_x, voisin_y, (arrivee.x), (arrivee.y));
    
            noeuds *voisin = creer_noeud(voisin_x, voisin_y, cout_g, cout_h, courant);
            inserer_noeud_au_bon_emplacement(&listeOuverte, voisin); // de sorte que la liste ouverte soit une file de priorités où chaque élément est trié par son coût f        
        }  
    }
    
}

void genererGraphe(int choixGraphe, char ***graphe, int *nbLignes, int *nbColonnes) {
    
    char *filename;
    if (choixGraphe == 1)
        filename = "sansObstacle.txt";
    else if (choixGraphe ==2)
        filename = "obstacleSimple.txt";
    else if (choixGraphe ==3)
        filename = "petitLabyrinthe.txt";
    else if (choixGraphe ==4)
        filename = "grandLabyrinthe.txt";
    else if (choixGraphe ==5)
        filename = "oxo.txt";    
    else if (choixGraphe ==6)
        filename = "impossible.txt";  
    FILE *file = fopen(filename, "r");   
    if (!file) {
        fprintf(stderr, "Erreur : impossible d'ouvrir le fichier %s.\n", filename);
        return;
    }

    *nbLignes = 0;
    *nbColonnes = 0;
    int maxColonnes = 0;
    char c;
    int colonneCourante = 0;

    // comptage des lignes et colonnes
    while ((c = fgetc(file)) != EOF) {
        if (c == '\n') {
            *nbLignes += 1;
            if (colonneCourante > maxColonnes)
                maxColonnes = colonneCourante;
            
            colonneCourante = 0;
        } else {
            colonneCourante++;
        }
    }
    
    // ajuste pour la dernière ligne si elle n'a pas de \n
    if (colonneCourante > 0) {
        *nbLignes += 1;
    if (colonneCourante > maxColonnes)
        maxColonnes = colonneCourante;    
    }
    
    *nbColonnes = maxColonnes;
    *graphe = malloc((*nbLignes) * sizeof(char*));

    for (int i = 0; i < *nbLignes; i++) {
        (*graphe)[i] = malloc((*nbColonnes) * sizeof(char));
    }

    
    rewind(file); // revient au début du fichier

    int i = 0, j = 0;
    
    while ((c = fgetc(file)) != EOF) { // lit le fichier caractère par caractère tant qu'il n'arrive pas à la fin du fichier (EndOfFile)
        if (c == '\n') { // saut de ligne, on passe à la ligne suivante et on revient au premier élément de la ligne
            i++;
            j = 0;
        } else if (j < *nbColonnes) {
            (*graphe)[i][j] = c; // on ajoute le caractère à l'emplacement [i][j]
            j++; 
        }
    }

    fclose(file);
}

int main() {
    char **graphe = NULL;
    int nbLignes = 0, nbColonnes = 0;
    int choixGraphe;

    do {
        printf("Choisir le graphe :\n");
        printf("1. Sans obstacle.\n"); 
        printf("2. Obstacle simple.\n"); 
        printf("3. Petit labyrinthe.\n");
        printf("4. Grand labyrinthe.\n");
        printf("5. Bonus : OXO.\n");
        printf("6. Bonus : pas de chemin possible.\n");
        

        if (scanf("%i", &choixGraphe) != 1) {
            while (getchar() != '\n');
            continue;
        }
    } while (choixGraphe < 1 && choixGraphe > 6);

    genererGraphe(choixGraphe, &graphe, &nbLignes, &nbColonnes);

    position depart = trouverCoordonnees(&graphe, nbLignes, nbColonnes, 'D');
    position arrivee = trouverCoordonnees(&graphe, nbLignes, nbColonnes, 'A');

    listeNoeuds cheminFinal; 
    initialiser_liste(&cheminFinal);

    cheminFinal = aStar(&graphe, depart, arrivee, nbLignes, nbColonnes, &cheminFinal);
    if (cheminFinal.debutListe == NULL)
        printf("Pas de chemin trouvé !\n");  
  
    return 0;
}