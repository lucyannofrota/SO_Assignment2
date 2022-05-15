#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include <pthread.h>
#include <time.h>
#include <stdlib.h>

#include <sys/msg.h>

#include <errno.h>


void* consumer_thread(void* arg){

    pthread_exit(NULL);
}

void* calc_thread(void* arg){

    int *args = (int*) arg;

    printf("Os valores recebidos pelo thread são: %i, %i\n",args[0],args[1]);

    int *ret = (int*) malloc(2*sizeof(int));


    ret[0] = args[0] + args[1];
    ret[1] = args[0] * args[1];

    pthread_exit(ret);
}


int MAIN_PID;

void catch_sigint(int val){
    kill(MAIN_PID,SIGINT);
    exit(0);
}

int main(int arc, char **argv){
    printf("Problema 3\n");

    MAIN_PID = getpid();

    // Configurando Sinais
    signal(SIGINT, catch_sigint);

    // Inicializando vetor de argumentos para o thread
    int arg1[2];

    printf("Introduza um numero inteiro (1/2): ");
    scanf("%d",&arg1[0]);
    printf("Introduza um numero inteiro (2/2): ");
    scanf("%d",&arg1[1]);

    printf("Valores introduzidos: %i, %i\n",arg1[0],arg1[1]);


    // Criando Handlers dos threads
    pthread_t thr1;

    // Criando Threads
    pthread_create(&thr1,NULL,calc_thread, &arg1);

    // Inicializando ponteiro para receber a resposta do thread
    int *ret;

    // Aguardando o termino do thread
    pthread_join(thr1,&ret);

    printf("Soma: %i\nProduto: %i\n",ret[0],ret[1]);

    // Libertando a memoria alocada pelo thread
    free(ret);
    
    printf("Done!\n");

    return 0;
}
