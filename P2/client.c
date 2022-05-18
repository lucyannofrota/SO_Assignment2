#include <stdio.h>
#include <stdlib.h>

#include <float.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include <semaphore.h>


#include <sys/wait.h>




FILE* file = NULL;
int file_status = 0; // file_status é importante para garantir que file não será finalizado 2 vezes.

int read_line(FILE** file, double* value){
    if(*file == NULL){
        *file = fopen("input.asc", "r"); // Abre o ficheiro na primeira vez que é chamada
        if(*file == NULL) perror("Missing \"input.asc\"");
        else file_status = 1;
    }
    char f_number[strlen("-0.")-FLT_MIN_10_EXP+1]; // Buffer com o tamanho maximo que uma variavel float pode assumir em ascii
    if(fgets(f_number,sizeof f_number,*file) != NULL){ // Le apenas uma linha do ficheiro
        *value = atof(f_number); // Escreve o valor lido no ponteiro
        printf("Valor lido de input.asc: %.2f\n",*value);
        return 1;
    }
    else{
        fclose(*file);
        file_status = 0;
        return 0; // Retorna 0 caso chegue ao fim do ficheiro
    }
}


void free_resources(void){
    if (file_status == 1) fclose(file);
}

void catch_interrupts(int val){
    printf("\nSignal received. Terminating the program\n");
    free_resources();
    exit(0);
}

int main(int arc, char **argv){
    printf("Problema 2| Client\n");

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
