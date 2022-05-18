#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/msg.h>

#include <pthread.h>

void* calc_thread(void* arg){
    // Inicializando ponteiro de retorno
    int *ret = malloc(2*sizeof(int));

    // Ponteiro para receber o argumento de entrada
    int *args = (int*) arg;

    printf("Os valores recebidos pelo thread são: %i, %i\n",args[0],args[1]);

    ret[0] = args[0] + args[1];
    ret[1] = args[0] * args[1];

    pthread_exit(ret);
}

// Inicializando ponteiro para receber a resposta do thread
int *ret;

void free_resources(void){
    free(ret);
}

void catch_interrupts(int val){
    printf("\nSignal received. Terminating the program\n");
    free_resources();
    exit(0);
}

int main(int arc, char **argv){
    printf("Problema 3\n");

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


    // Aguardando o termino do thread
    pthread_join(thr1,(void**)&ret);

    printf("Soma: %i\nProduto: %i\n",ret[0],ret[1]);

    // Libertando a memoria alocada pelo thread
    free(ret);
    
    printf("Done!\n");

    return 0;
}
