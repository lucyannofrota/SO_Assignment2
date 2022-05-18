#include <stdio.h>
#include <stdlib.h>

#include <float.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include <semaphore.h>


#include <sys/wait.h>

double* mptr;

sem_t *sem_r, *sem_w;

int fd;

void free_resources(void){
    munmap(mptr,1*sizeof(double));
    sem_close(sem_r);
    sem_close(sem_w);
    sem_unlink("serv_c_sem_r");
    sem_unlink("serv_c_sem_w");
    close(fd);
}

void catch_interrupts(int val){
    printf("\nSignal received. Terminating the program\n");
    free_resources();
    exit(0);
}

int main(int arc, char **argv){
    printf("Problema 2| Server\n");

    // Configurando Sinais

    // Core
    signal(SIGBUS, catch_interrupts);
    signal(SIGABRT, catch_interrupts);
    signal(SIGFPE, catch_interrupts);
    signal(SIGILL, catch_interrupts);
    signal(SIGIOT, catch_interrupts);
    signal(SIGSEGV, catch_interrupts);
    signal(SIGQUIT, catch_interrupts);
    signal(SIGSYS, catch_interrupts);
    signal(SIGXFSZ, catch_interrupts);
    signal(SIGTRAP, catch_interrupts);
    signal(SIGXCPU, catch_interrupts);

    // Term
    signal(SIGALRM, catch_interrupts);
    signal(SIGHUP, catch_interrupts);
    signal(SIGINT, catch_interrupts);
    signal(SIGIO, catch_interrupts);
    signal(SIGKILL, catch_interrupts);          
    signal(SIGPIPE, catch_interrupts);
    signal(SIGPOLL, catch_interrupts);
    signal(SIGPROF, catch_interrupts);
    signal(SIGPWR, catch_interrupts);
    signal(SIGSTKFLT, catch_interrupts);
    signal(SIGTERM, catch_interrupts);
    signal(SIGUSR1, catch_interrupts);
    signal(SIGUSR2, catch_interrupts);
    signal(SIGVTALRM, catch_interrupts);

    // Stop
    signal(SIGTTIN, catch_interrupts);
    signal(SIGTTOU, catch_interrupts);
    signal(SIGSTOP, catch_interrupts);

    // Inicializando a memoria compartilhada
    fd = shm_open("s_mem",O_CREAT | O_RDWR | O_TRUNC, 0666);
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

    fd = open("input.bin", O_CREAT | O_RDWR | O_TRUNC, 0666);

    do{
        sem_wait(sem_r); // Aguarda pelo semaforo de leitura
        if(mptr[0] != DBL_MIN){
            printf("[Memoria partilhada] -> Server: %.2f\n",mptr[0]);
            write(fd, mptr, 1 * sizeof(double));
            len++;
        }
        else break;
        sem_post(sem_w); // Liberta o semaforo de escrita
    }while(mptr[0] != DBL_MIN);

    double list[len];

    // Lendo o file "input.bin"
    lseek(fd, (off_t)0, SEEK_SET);
    read(fd, list, len * sizeof(double));

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

    printf("\n\nDone!\n");

    return 0;
}
