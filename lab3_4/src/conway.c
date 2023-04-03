#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

#define R 10
#define K 10
#define BROJ_PRIORITETA 28

/*
int mreza[][K] = {
                      {1, 0, 1, 0, 1, 1, 1, 0, 0, 1}, 
                      {0, 1, 1, 1, 1, 0, 1, 0, 0, 0}, 
                      {1, 1, 0, 1, 0, 1, 0, 0, 1, 0},
                      {1, 0, 1, 0, 1, 1, 1, 0, 1, 0}, 
                      {0, 1, 0, 1, 0, 1, 0, 1, 0, 1}, 
                      {1, 1, 0, 1, 0, 1, 0, 0, 1, 0},
                      {1, 0, 0, 1, 0, 1, 1, 1, 1, 1}, 
                      {0, 1, 0, 1, 0, 1, 0, 1, 1, 0},
                      {1, 1, 1, 1, 0, 1, 1, 0, 0, 0},
                      {1, 1, 0, 1, 0, 1, 0, 0, 1, 0}
                  };*/

int mreza[][K] = {
                      {0, 1, 0, 0, 0, 0, 0, 0, 0, 0}, 
                      {0, 1, 0, 0, 0, 0, 0, 0, 0, 0}, 
                      {0, 1, 0, 0, 0, 0, 0, 0, 0, 0},
                      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
                      {0, 0, 0, 0, 0, 0, 1, 1, 0, 0}, 
                      {0, 0, 0, 0, 0, 0, 1, 0, 0, 0},
                      {0, 0, 1, 0, 0, 0, 0, 0, 0, 0}, 
                      {0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
                      {0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
                      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
                  };



int mreza[R][K];
int nova_mreza[R][K] = {0};

/* struktura koja cuva indekse i prioritet svake celije odnosno niti koja joj odgovara */
struct indeks_prior{
	int i;
	int j;
	int pr;
}indeks_prior;


sem_t semaphore[BROJ_PRIORITETA];


/* ispis mreze */
void print_mrezu(int mreza[R][K]){
	for(int i = 0; i < R; i++){
    	for(int j = 0; j < K; j++){
      	if(mreza[i][j] == 1){
        	printf("* ");
      	} else {
        	printf("  ");
      	}
    }
    printf("\n");
  }
}


/* funkcija koja na osnovu vrijednosti celije i na osnovu broja zivih i mrtvih komsija racuna novu vrijednost celije, 
 		prema pravilima igre zivota */
int prezivljavanje(int celija, int komsije[], int dim_komsije){
  int i;
  int zive_komsije = 0;
  for(i = 0; i < dim_komsije; i++){
    if(komsije[i]){
      zive_komsije++;
    }
  }

  if(celija){
    if(zive_komsije == 2 || zive_komsije == 3){
      return 1;
    }
  }

  if(!celija){
    if(zive_komsije == 3){
      return 1;
    }
  }

  return 0;
}

/* funkcija niti standardnog potpisa*/
void* evolucija(void* arg){
	
	struct indeks_prior a = *(struct indeks_prior *)arg;	
	
	// sve niti osim one koja je postovana od strane prethodne niti cekaju
	// na samom pocetku samo nit prioriteta nula moze uci, jer je ona postovana u main funkciji
	sem_wait(&semaphore[a.pr]);

	int n = 0;
	int celija = mreza[a.i][a.j];
	int komsije[R*K-1] = {0};
	for(int k = a.i-1; k <= a.i+1; k++){
    	for(int z = a.j-1; z <= a.j+1; z++){
            if(!(k == a.i && z == a.j)){    					// eliminise samu celiju kao vlastitog komsiju
                if((k >= 0 && k < R) && (z >= 0 && z < K)){
                      komsije[n++] = mreza[k][z];			// u niz komsije smjestaju se sve komsije celije odgovarajuce niti
                }
            }
        }
    }

    // funkcija prezivljavanje na osnovu celije i njenih komsija vraca novo stanje celije u smjesta ga u novu mrezu koja se postepeno puni 
    nova_mreza[a.i][a.j] = prezivljavanje(celija, komsije, n);

    // na osnovu unaprijed poznatog odnosa prioriteta u zavisnosti od toga koja nit kojeg prioriteta je trenutno u izvrsavanju
    // postovace se odgovarajuci broj niti koji ce se dalje izvrsavati

		if(a.pr == 0 || a.pr == 25 || a.pr == 26){
			sem_post(&semaphore[a.pr + 1]);

		} else if(a.pr == 1 || a.pr == 2 || a.pr == 23 || a.pr == 24) {
			sem_post(&semaphore[a.pr + 1]);
			sem_post(&semaphore[a.pr + 1]);
			
		} else if(a.pr == 3 || a.pr == 4 || a.pr == 21 || a.pr == 22) {
			sem_post(&semaphore[a.pr + 1]);
			sem_post(&semaphore[a.pr + 1]);
			sem_post(&semaphore[a.pr + 1]);

		} else if(a.pr == 5 || a.pr == 6 || a.pr == 19 || a.pr == 20) {
			sem_post(&semaphore[a.pr + 1]);
			sem_post(&semaphore[a.pr + 1]);
			sem_post(&semaphore[a.pr + 1]);
			sem_post(&semaphore[a.pr + 1]);

		} else {
			sem_post(&semaphore[a.pr + 1]);
			sem_post(&semaphore[a.pr + 1]);
			sem_post(&semaphore[a.pr + 1]);
			sem_post(&semaphore[a.pr + 1]);
			sem_post(&semaphore[a.pr + 1]);
		}

		free(arg);

	}

/* funkcija koja racuna matricu prioriteta na osnovu mreze celija */
	// kao argument se proslijedi prazna matrica prioriteta a zatim se popunjava
void izracunaj_matricu_prioriteta(int matrica_prioriteta[R][K]){
    
    int pr;

    for(int i = 0; i < R; i++){
      pr = i * 2;
      for(int j = 0; j < K; j++){
        matrica_prioriteta[i][j] = pr++;
      }
    }
}

/* funkcija pomocu koje se zive i mrtve celije mreze kroz komandnu liniju za inicijalizaciju igre zivota */ 
void unesi_mrezu(){
	printf("Unesi inicijalni raspored mreze: \n");
	for(int i = 0; i < R; i++){
		for(int j = 0; j < K; j++){
			printf("pozicija [%d][%d]: ", i, j);
			int elem;
			do{
				scanf("%d", &elem);
				if(elem == 1 || elem == 0){
					break;
				}
				printf("Unesi ponovo poziciju [%d][%d]: ", i, j);
			} while(1);
			mreza[i][j] = elem;
		}
	}
}

/*void analiziraj_matricu_prioriteta(int matrica_prioriteta[R][K]){

	int najveci_prioritet = 0;

	for(int i = 0; i < R; i++){
		for(int j = 0; j < K; j++){
				if(matrica_prioriteta[i][j] > najveci_prioritet){
					najveci_prioritet = matrica_prioriteta[i][j];
				}
								
		}				
	}
	
	broj_prioriteta = najveci_prioritet + 1;
}*/



int main(int argc, char* argv[]){

	/* kao argument komandne linije funkciji main se proslijedjuje jedan parametar*/
  if(argc < 2){
 		printf("Nije proslijedjen ulazni parametar!\n");
    return 1;
  }

  int trajanje_spavanja;
  trajanje_spavanja= atoi(argv[1]);					// kroz parametre kom. linije prosledjuje se vrijeme spavanja glavne niti
    
  //unesi_mrezu();

 	system("clear");									
	print_mrezu(mreza);
	sleep(trajanje_spavanja);
	
	/* za svaki element mreze formira se po jedna nit, tj ovde referenca na nit za sada */
	pthread_t th[R][K];
	

	/* racunanje matrice prioriteta */
	int matrica_prioriteta[R][K];
  izracunaj_matricu_prioriteta(matrica_prioriteta);

  //analiziraj_matricu_prioriteta(matrica_prioriteta);

  /* beskonacna while petlja koja omoguca da program radi sve dok ne bude rucno prekinut */
	while(1){

		for(int i = 0; i < BROJ_PRIORITETA; i++){
			sem_init(&semaphore[i], 0, 0);
		}
		sem_post(&semaphore[0]);				// postujemo nulti semafor, tj prvu nit koja ima prioritet nula
		
		/* kreiranje niti i prosledjivanje parametra u vidu strukture struct indeks_prior koja sadrzi indekse niti i njen prioriteet */
		for(int i = 0; i < R; i++){
			for(int j = 0; j < K; j++){
				struct indeks_prior *a = malloc(sizeof(struct indeks_prior));
				a->i = i;
				a->j = j;
				a->pr = matrica_prioriteta[i][j];
				if(pthread_create(&th[i][j], NULL, &evolucija, a) != 0){
					perror("failed to create\n");
					return 1;
				}
			}
		}

		for(int i = 0; i < R; i++){ 
			for(int j = 0; j < K; j++){
				if(pthread_join(th[i][j], NULL) != 0){
					perror("failed to join\n");
					return 2;
				}
			}
		}

		for(int i = 0; i < R; i++){
	        for(int j = 0; j < K; j++){
	          mreza[i][j] = nova_mreza[i][j];
	        }
	    }

	    system("clear");
	    print_mrezu(mreza);
	    fflush(stdout);
	    sleep(trajanje_spavanja);
	}

	for(int i = 0; i < BROJ_PRIORITETA; i++){
		sem_destroy(&semaphore[i]);
	}

	return 0;
}

