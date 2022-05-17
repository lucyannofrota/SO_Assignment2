#include <stdio.h>
#include <stdlib.h>

#include <float.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include <semaphore.h>

FILE* file = NULL;

int read_line(FILE** file, double* value){
    if(*file == NULL){
        *file = fopen("input.asc", "r"); // Abre o ficheiro na primeira vez que é chamada
        if(*file == NULL) perror("Missing \"input.asc\"");
    }
    char f_number[strlen("-0.")-FLT_MIN_10_EXP+1]; // Buffer com o tamanho maximo que uma variavel float pode assumir em ascii
    if(fgets(f_number,sizeof f_number,*file) != NULL){ // Le apenas uma linha do ficheiro
        *value = atof(f_number); // Escreve o valor lido no ponteiro
        printf("Valor lido de input.asc: %.2f\n",*value);
        return 1;
    }
    return 0; // Retorna 0 caso chegue ao fim do ficheiro
}


void free_resources(void){
    fclose(file);
}

int main(int arc, char **argv){
    printf("Problema 2| Client\n");
    
    // Configurando Sinais
    // signal(SIGINT, catch_sigint);


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

    double num_line;
    while(read_line(&file,&num_line)){ // Faz a leitra de uma linha
        sem_wait(sem_w); // Aguarda pelo semaforo de escrita
        mptr[0] = num_line; // Escreve na memoria partilhada
        printf("Client -> [Memoria partilhada]: %.2f\n",(double)mptr[0]);
        sem_post(sem_r); // Liberta o semaforo de leitura
    }

    // Envia o valor terminador na memoria partilhada
    sem_wait(sem_w);
    mptr[0] = DBL_MIN;
    printf("Client DBL_MIN\n");
    sem_post(sem_r);

    free_resources(); // Liberta recursos

    printf("\n\nDone!\n");

    return 0;
}
