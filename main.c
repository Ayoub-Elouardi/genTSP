#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>


#define X_MAX 750
#define Y_MAX 750
#define NBR_VILLE 12
#define POPULATION 10
#define NBR_ITERATION 1000
#define PM 0.25    //Proba de mutation

/*-------------------------------------------------------------*/

typedef struct{
    unsigned x, y;
} city;

typedef struct{
    int solution[NBR_VILLE];
    double fitness;
} individual;

/*-------------------------------------------------------------*/

void setup_cities(city *) ; //génère  aléatoirement les villes.
void generate_initial_population(individual *) ; //génère la population initiale , et crée des solutions aléatoires.
double get_distance(city *, int, int) ; //retourne la distance entre deux villes données.
double * evaluate_fitness(city *, individual *) ; //Calcul la valeur fitness pour chaque individus.
int * selection(double *) ;     //Fonction de sélection.
individual * crossover(individual *, int *)   ;  //Fonction de croisement   .
void mutation(individual *) ;  //Fonction de mutation  .
void cycle_crossover(individual *, int, int, int *, int *) ; //Calcule l’algorithme de croisement du cycle et génère deux nouvelles solution.
void swap(int *, int *) ; //Utilisée dans la fonction de mutation afin d'échanger deux villes.
int highest_fitness(individual *) ;  //retourne l’individus avec  la plus grande valeur fitness.
/*-------------------------------------------------------------*/

int main(){

    srand(time(NULL));           //Random seed initialization

    city cities_map[NBR_VILLE]; //Allocates the cities in the maps

    individual * population, * new_gen;

    population = malloc(sizeof(individual) *POPULATION); //Allocates the population, there are "POPULATION_COUNT" individuals

    setup_cities(cities_map);       //Generates (randomically) the cities position

    FILE * fd = fopen("DonnéeTSP.txt", "w");
    if (fd == NULL){
        printf("ERREUR LE DOSSIER NE PEUT PAS ETRE OUVRIR!\n");
        return -1;
    }

    fprintf(fd, "%d,%d\n", X_MAX, Y_MAX);       //We save the grid size...
    fprintf(fd, "%d\n", NBR_VILLE);        //We save the number of cities generated...

    for (int i = 0; i < NBR_VILLE; i++){   //...and the various coordinates
        fprintf(fd, "%d,%d\n", cities_map[i].x, cities_map[i].y);
    }

    generate_initial_population(population);    //Generates casually the initial population, creating random solutions

    int best;
    int * candidates;
    double length;
    double * cdf;

    for (int i = 0; i < NBR_ITERATION; i++){

        cdf = evaluate_fitness(cities_map, population);    //We obtain the cumulative distribution function associated to the current population

        if (i%100 == 0){

            best = highest_fitness(population);     //returne l index de l individu avec la mellieur fitness

            length = 1/population[best].fitness;    //The lenght of the solution is the inverse of the fitness (the fitness is defined as 1/lenght)

            fprintf(fd, "%d %f ", i, length);       //Save the best lenght...

            for (int j = 0; j < NBR_VILLE; j++){
                fprintf(fd, "%d ", population[best].solution[j]);   //... and the solution
            }
            fprintf(fd, "\n");

            printf("Generation actuelle %d | Meilleure resultat: %f\n", i/100, length);
        }

        candidates = selection(cdf);        //candidates is an integer vector containing the indexes of those that are going to reproduce

        new_gen = crossover(population, candidates); //We crossover the population to obtain a new one

        free(population);

        population = new_gen;

        mutation(population);   //We apply the eventual mutation to the population
    }

    fclose(fd);

    return 0;
}

/*-------------------------------------------------------------*/


void swap(int * a, int * b){
    int t = *a;
    *a = *b;
    *b = t;
}

int highest_fitness(individual * population){

    int index = 0;
    double initial = population[0].fitness;
    for (int j = 1; j < POPULATION; j++){
        if (population[j].fitness > initial){
            initial = population[j].fitness;
            index = j;
        }
    }
    return index;
}

void setup_cities(city * cities_map){

    for (int i = 0; i < NBR_VILLE; i++){
        cities_map[i].x = rand() % X_MAX;   //genere des cordonnées pour les villes ente 0 et X max
        cities_map[i].y = rand() % Y_MAX;
    }
}

void generate_initial_population(individual * population){

    int x;
    for (int i = 0; i < POPULATION; i++){
        for (int j = 0; j < NBR_VILLE; j++){
            x = rand() % (j+1);     //numbre entre 0 et j
            if (x != j){
                population[i].solution[j] = population[i].solution[x];
            }
            population[i].solution[x] = j;
        }
    }
}

double get_distance(city * cities_map, int id_1, int id_2){

    int x1 = cities_map[id_1].x;
    int y1 = cities_map[id_1].y;
    int x2 = cities_map[id_2].x;
    int y2 = cities_map[id_2].y;

    double dist = sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2));
    return dist;
}

double * evaluate_fitness(city * cities_map, individual * population){

    int id_1, id_2;
    double total, fitness_sum = 0;
    double * cdf = malloc(sizeof(double) * POPULATION);  //  alloué l'epace pour la fonction cumulative distribué

    for (int i = 0; i < POPULATION; i++){

        total = 0;  //contient la distance total d'une solution

        for (int j = 0; j < NBR_VILLE-1; j++){
            id_1 = population[i].solution[j];
            id_2 = population[i].solution[j+1];
            total += get_distance(cities_map, id_1, id_2);  //calculer la distance total entre les villes

        population[i].fitness = 1/total;        //la fitness est 1/total
        fitness_sum += population[i].fitness;

    cdf[0] = population[0].fitness/fitness_sum;

    for (int i = 1; i < POPULATION; i++){
        cdf[i] = cdf[i-1] + population[i].fitness/fitness_sum;
    }

    return cdf;
}}}

int * selection(double * cdf){

    double extraction;
    int pivot, first, last;
    int * candidates = malloc(sizeof(int) * POPULATION);

    for (int i = 0; i < POPULATION; i++){

        first = 0;
        last = POPULATION;
        extraction = (double)rand()/RAND_MAX;

        while (first != last){  //recherche dichotomique de la premiere element de la CDF qu'il a la plus grand proba que la valeur aleatoir choisi hat has bigger probabthe value randomly generated
            pivot = (first + last)/2;
            if (cdf[pivot] == extraction){
                break;
            }
            if (cdf[pivot] > extraction){
                last = pivot;
            } else {
                first = pivot + 1;
            }
        }
        candidates[i] = first;
    }

    free(cdf);
    return candidates;
}

individual * crossover(individual * population, int * candidates){

    individual * new_gen = malloc(sizeof(individual) * POPULATION);

    for (int i = 0; i < POPULATION; i+=2){
        cycle_crossover(population, candidates[i], candidates[i+1], new_gen[i].solution, new_gen[i+1].solution);
    }

    free(candidates);




    return new_gen;
}

void mutation(individual * population){

    int x, y, p = RAND_MAX * PM;

    for (int i = 0; i < POPULATION; i++){

        if (rand() < p){
            do {
                x = rand() % NBR_VILLE;
                y = rand() % NBR_VILLE;
            } while (x == y);

            swap(&population[i].solution[x], &population[i].solution[y]);
        }
    }
}

void cycle_crossover(individual * population, int id_1, int id_2, int * child_1, int * child_2){

    int parent_1 = id_1, parent_2 = id_2;
    int j = 0;
    int lookup[NBR_VILLE];
    int flags[NBR_VILLE] = {0};
    int visited = 0;
    int first = population[id_1].solution[0], last;

    for (int i = 0; i < NBR_VILLE; i++){
        lookup[population[id_1].solution[i]] = i;
    }

    while (visited < NBR_VILLE){

        child_1[j] = population[parent_1].solution[j];
        child_2[j] = population[parent_2].solution[j];
        last = population[id_2].solution[j];

        flags[j] = 1;
        visited++;
        j = lookup[last];

        if (first == last){
            swap(&parent_1, &parent_2);
            while (flags[j]) j++;
            first = population[id_1].solution[j];
        }
    }
}
