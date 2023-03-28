#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include<sys/ioctl.h>
 
#define DIMENSION 3

typedef struct cells{
        int cells_matrix[DIMENSION][DIMENSION];
}CELLS;

#define WR_VALUE _IOW('a','a',CELLS*)
#define RD_VALUE _IOR('a','b',int*)

void print_matrix(CELLS cells){
        int i,j;
        for(i=0;i<DIMENSION;i++){
                for(j=0;j<DIMENSION;j++){
                        printf("%d ", cells.cells_matrix[i][j]);
                }
                printf("\n");
        }
}
 
int main()
{
        int fd;
        int new_state = 0;
        CELLS cells = {
                .cells_matrix = {{1,0,1}, {0,0,0}, {1,0,0}}
        };
        
        fd = open("/dev/etx_device", O_RDWR);
        if(fd < 0) {
                printf("Cannot open device file...\n");
                return 0;
        }

        // Uncomment for error check 
        /*int ret = ioctl(fd, RD_VALUE, (int*) &new_state); 
        if(ret < 0){
                printf("Wrong return value from driver!\n");
                return 0;
        }*/
 
        printf("Current state:\n");
        print_matrix(cells);
        ioctl(fd, WR_VALUE, (CELLS*) &cells); 
 
        printf("\nNew state:\n");
        ioctl(fd, RD_VALUE, (int*) &new_state);
        cells.cells_matrix[DIMENSION/2][DIMENSION/2] = new_state;
        print_matrix(cells);
 
        close(fd);
}