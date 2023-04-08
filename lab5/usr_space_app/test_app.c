#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>


/* Definitions of ioctl commands */
#define UPISI_STANJE _IOW('a', 'a', struct celija_komsije*)
#define PROCITAJ_STANJE _IOR('a', 'b', int*)

#define R 3
#define K 3

/* Initial game of life */
int mreza[R][K] = {
	{0, 1, 0}, 
	{0, 1, 0},
	{0, 1, 0}
};

/* Struct where data about cells from user space is stored, and then passed to kernel space using ioctl */
struct celija_komsije {
	int celija;
	int komsije[R*K-1];
	int dim_komsije;
};


/* Function that prints cells */
void print_mrezu(int mreza[R][K]){
  int i, j;
  for(i = 0; i < R; i++){
      for(j = 0; j < K; j++){
        if(mreza[i][j] == 1){
          printf("* ");
        } else {
          printf("  ");
        }
    }
    printf("\n");
  }
}


int main(){

	int fd;
	fd = open("/dev/cnw_device", O_RDWR);
	if(fd < 0){
		printf("Fajl rukovaoca nije uspjesno otvoren\n");
		return -1;
	}

    printf("Inicijalni raspored celija igre zivota:\n");
	print_mrezu(mreza);
    sleep(2);

    while(1){

        int nova_mreza[R][K] = {0};

        int i, j, k, z;
        for(i = 0; i < R; i++){
            for(j = 0; j < K; j++){

                int n = 0;
                struct celija_komsije ck; 
                ck.celija = mreza[i][j]; 
                for(int n = 0; n < R; n++){
                	ck.komsije[n] = 0;
                }    
      
                for(k = i-1; k <= i+1; k++){
                    for(z = j-1; z <= j+1; z++){
                        if(!(k == i && z == j)){                            // eliminites cell as its own neighbout
                            if((k >= 0 && k < R) && (z >= 0 && z < K)){
                                ck.komsije[n++] = mreza[k][z];              // adds cells neighbour into array that is later to be analyzed
                            }
                        }
                    }
                }
                ck.dim_komsije = n;

                /* Trying to result to read when there is nothing written in driver */
                int novo_stanje = 0;
                //ioctl(fd, PROCITAJ_STANJE, (int *) &novo_stanje);
                if(novo_stanje == -1){
                    printf("Pokusaj citanja kada nista nije upisano u drajver!\n");
                    return -1;
                }

                printf("Upis stanja u drajver/n");
                /* IOCTL call that sends data to kernel space*/
                ioctl(fd, UPISI_STANJE, (struct celija_komsije*)&ck);

                printf("Citanje novog stanja koje je drajver izracunao\n");
                novo_stanje = 0;
                /* IOCTL call that receives data from kernel space*/
                ioctl(fd, PROCITAJ_STANJE, (int *) &novo_stanje);
               	nova_mreza[i][j] = novo_stanje;
            }
        }

        for(i = 0; i < R; i++){
            for(j = 0; j < K; j++){
                mreza[i][j] = nova_mreza[i][j];
            }
        }

         system("clear");
         print_mrezu(mreza);
         sleep(2);
         fflush(stdout);
    }

    close(fd);

	return 0;
}
