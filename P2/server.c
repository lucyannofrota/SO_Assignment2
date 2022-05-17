#include <stdio.h>
#include <stdlib.h>

#include <float.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include <semaphore.h>

#include <signal.h>

double* mptr;

sem_t *sem_r, *sem_w;

void free_resources(void){
    munmap(mptr,1*sizeof(double));
    sem_close(sem_r);
    sem_close(sem_w);
    sem_unlink("serv_c_sem_r");
    sem_unlink("serv_c_sem_w");
}

void write_bin(double* value, int mode){
    static int fd = 0;
    if(mode == 1) close(fd);
    else{
        if(fd == 0) fd = open("input.bin",O_CREAT|O_WRONLY|O_TRUNC,0666);
        write(fd,value,1*sizeof(double));
    }
}

void catch_sigint(int val){
    // kill(MAIN_PID,SIGINT);
    printf("\nCustom ctrl c\n");
    free_resources();
    exit(0);
}

int main(int arc, char **argv){
    printf("Problema 2| Server\n");

    // Configurando Sinais
    signal(SIGINT, catch_sigint);

    // Inicializando a memoria compartilhada
    int fd = shm_open("s_mem",O_CREAT | O_RDWR | O_TRUNC, 0666);
    if(fd == -1){
        perror("open() error.");
    }
    else{
        ftruncate(fd,1*sizeof(double));
        mptr = mmap(NULL,1*sizeof(double),PROT_WRITE|PROT_READ,MAP_SHARED,fd,0);
        if(mptr == MAP_FAILED){
            perror("mmap() error.");
        }
        close(fd);
    }
    

    // Inicializando os semaforos
    sem_r = sem_open("serv_c_sem_r",O_CREAT, 0666, 0);
    sem_w = sem_open("serv_c_sem_w",O_CREAT, 0666, 1);
    if(sem_r == SEM_FAILED || sem_w == SEM_FAILED) perror("sem_open() error.");

    
    int len = 0;
    do{
        sem_wait(sem_r); // Aguarda pelo semaforo de leitura
        if(mptr[0] != DBL_MIN){
            printf("[Memoria partilhada] -> Server: %.2f\n",mptr[0]);
            write_bin(mptr,0);
            len++;
        }
        sem_post(sem_w); // Liberta o semaforo de escrita
    }while(mptr[0] != DBL_MIN);

    double list[len];

    // Lendo o file "input.bin"
    fd = open("input.bin",O_RDONLY,0666);
    read(fd,list,len*sizeof(double));

    // Escrevendo o file "output.asc"
    fd = open("output.asc",O_CREAT | O_WRONLY | O_TRUNC,0666);
    int i;
    char c_buffer[strlen("-0.")-FLT_MIN_10_EXP+1]; // Buffer com o tamanho maximo que uma variavel float pode assumir em ascii
    int n_len = 0;
    float mult = 4.0;
    for(i = 0; i < len; i++){
        n_len = snprintf(NULL,0,"%.2f\n",list[i]*mult); // Determinando o tamnanho do numero em ascii
        snprintf(c_buffer,(n_len+1),"%.2f\n",list[i]*mult); // Buffer com o tamanho maximo que uma variavel float pode assumir em ascii
        printf("(4x) Value: %s",c_buffer);
        write(fd,c_buffer,(n_len)*sizeof(char));
    }
    close(fd);

    // Libertando recursos
    free_resources();
    // sem_close("serv_c_sem");

    printf("\n\nDone!\n");

    return 0;
}
