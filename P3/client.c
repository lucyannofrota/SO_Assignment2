#include <stdio.h>
#include <stdlib.h>

#include <float.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include <semaphore.h>

// #include "my_tools.h"

// #include "my_buffer.h"

FILE* file = NULL;

int read_line(FILE** file, double* value){
    if(*file == NULL) *file = fopen("input.asc", "r");
    char f_number[strlen("-0.")-FLT_MIN_10_EXP+1]; // Buffer com o tamanho maximo que uma variavel float pode assumir em ascii
    if(fgets(f_number,sizeof f_number,*file) != NULL){
        *value = atof(f_number);
        printf("Vl: %.2f\n",*value);
        return 1;
    }
    return 0;
}

// int read_line(double* value){
//     if(file == NULL) file = fopen("input.asc", "r");
//     char f_number[strlen("-0.")-FLT_MIN_10_EXP+1]; // Buffer com o tamanho maximo que uma variavel float pode assumir em ascii
//     if(fgets(f_number,sizeof f_number,file) != NULL){
//         *value = atof(f_number);
//         return 1;
//     }
//     return 0;
// }

// void catch_sigint(int val){
//     // kill(MAIN_PID,SIGINT);
//     free_resources();
//     exit(0);
// }


void free_resources(void){
    fclose(file);
}

int main(int arc, char **argv){
    printf("Problema 2| Client\n");
    
    // Configurando Sinais
    // signal(SIGINT, catch_sigint);

    // Fazendo aquisição dos dados de "input.asc"
    // read_input(&buffer);
    // print_buffer(&buffer);

    // Inicializando memoria compartilhada
    double* mptr;
    int fd = shm_open("s_mem",O_RDWR, 0666);
    if(fd == -1){
        perror("open() error.");
    }
    else{
        mptr = mmap(NULL,1*sizeof(double),PROT_WRITE|PROT_READ,MAP_SHARED,fd,0);
        if(mptr == MAP_FAILED){
            perror("mmap() error.");
        }
        close(fd);
    }

    // Inicializando semaforo
    sem_t *sem_r = sem_open("serv_c_sem_r",O_RDWR, 0666);
    sem_t *sem_w = sem_open("serv_c_sem_w",O_RDWR, 0666);
    if(sem_r == SEM_FAILED || sem_w == SEM_FAILED) perror("sem_open() error.");
    // int sval;
    // sem_getvalue(sem, &sval);
    // sem_wait(sem);

    int count = 0;
    double num_line;
    while(read_line(&file,&num_line)){
        // sem_wait(sem_r);
        // printf("Server %.2f\n",(double)mptr[0]);
        // sem_post(sem_w);

        sem_wait(sem_w);
        mptr[0] = num_line;
        printf("Client %.2f\n",(double)mptr[0]);
        sem_post(sem_r);
        // sleep(1);
        count++;
    }

    // Mandando o valor terminador
    sem_wait(sem_w);
    mptr[0] = DBL_MIN;
    printf("Client DBL_MIN %.2f\n",(float)mptr[0]);
    sem_post(sem_r);

    // free(buffer.ptr);
    // free_buffer(&buffer);
    free_resources();

    printf("\n\nDone!\n");

    return 0;
}
