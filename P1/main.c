#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#include <sys/ipc.h>
#include <sys/msg.h>
#include <pthread.h>

#include <stdlib.h>



#define N_MAX 49
#define N_MIN 1

#define N_NUMBERS 6

#define N 4
#define M 40 + N


int msq1, msq2;

struct mymsg {
    long   mtype;      /* Message type. */
    int   mtext[1];    /* Payload. */
};

void setup_queue(int msq,int n_terms){
    struct msqid_ds config;

    if(msgctl(msq,IPC_STAT,&config) != 0){
        perror("Failed to setup msq\n");
    }
    else{
        config.msg_qbytes = sizeof(struct mymsg)*n_terms;
        printf("Queue Size: %i bytes\n",(int)config.msg_qbytes);
        if(msgctl(msq,IPC_SET,&config) != 0){
            perror("Failed to setup msq\n");
        }
    }
}

int launch_process(int *childStatus,void (*myfunc)(void*), void* arg){
    printf("Processo Realizando Fork|PID %i\n",getpid());
    // Fazendo Fork
    int myPID = fork();
    if(myPID == -1) perror("Failed to fork\n");
    if(myPID == 0){ // Selecionando processo filho
        printf("Processo Filho Iniciado|PID %i\n",getpid());
        myfunc(arg);
        exit(1);
    }
    else{ // Processo pai
        return myPID; 
    }
}

int get_rand(void){
    return rand()%((N_MAX+1)-N_MIN) + N_MIN;
}

int has_duplicates(int *ptr, int new){
    int i;
    for(i = 0; i < N_NUMBERS; i++){
        if(ptr[i] == new) return 1;
    }
    return 0;
}

void print_buffer(int *ptr, int size){
    int i;
    for(i = 0; i < N_NUMBERS; i++){
        printf("%i | %i\n", i+1,ptr[i]);
    }
}

void write_to_file(int *buff,int ref){
    char name[10];
    sprintf(name,"Key_%i",ref);
    int stdout_ = dup(1);
    close(1); // Fechar porta de escrita
    int fdf = open(name,O_WRONLY|O_CREAT,0666); // Abrir/criar o ficheiro em mode de escrita
    dup2(fdf,1); // Redirecionar porta de escrita
    print_buffer(buff,N_NUMBERS);
    close(fdf); // Fechar file
    dup2(stdout_,1); // Retornar porta de escrita ao stdout
}

void consumer(void* arg){

    int ref = 0;

    if(arg != NULL){
        ref = *((int*) arg);
    }
    else{
        perror("Missing Argument.");
    }

    struct mymsg msg1, msg2;

    msg2.mtype = 1;

    int numbers = 1;

    int buff[N_NUMBERS];

    do{
        if(msgrcv(msq1,&msg1,sizeof(struct mymsg),0,0) == -1){
            perror("Error sending message\n");
        }
        else{
            if(has_duplicates(buff,msg1.mtext[0])){
                printf("Consumer| Valor Duplicado\n");
                msg2.mtext[0] = 1;
                if(msgsnd(msq2,&msg2,sizeof(struct mymsg),0) != 0){
                    perror("Error sending message\n");
                }
            }
            else{
                buff[numbers-1] = msg1.mtext[0];
                numbers++;

                if(numbers > N_NUMBERS){
                    msg2.mtext[0] = 0;
                    if(msgsnd(msq2,&msg2,sizeof(struct mymsg),0) != 0){
                        perror("Error sending message\n");
                    }
                    break;
                }
                else{
                    msg2.mtext[0] = 1;
                    if(msgsnd(msq2,&msg2,sizeof(struct mymsg),0) != 0){
                        perror("Error sending message\n");
                    }
                }
            }
        }
        // if(has_duplicates(buff,))
    }while(1);

    printf("Consumer| Processo Concluido!\n");

    // print_buffer(buff,N_NUMBERS);

    write_to_file(buff,ref);

    // pthread_exit(NULL);
    exit(0);
}

void producer(void* arg){
    srand(7);

    struct mymsg msg1, msg2;

    msg2.mtype = 1;
    msg2.mtext[0] = 1;

    int i;

    printf("Producer| [M,N] = [%i,%i]\n",M,N);

    // Enviando as mesagens para msq2
    for(i = 0; i < M; i++){//get_rand();
        // msg2.mtext[0] = 'c';
        // printf("Num: %i\n",msg2.mtext[0]);
        // Send to queue
        if(msgsnd(msq2,&msg2,sizeof(struct mymsg),0) != 0){
            perror("Error sending message\n");
        }
        struct msqid_ds config;
        if(msgctl(msq2,IPC_STAT,&config) != 0){
        
            perror("Failed to get info\n");
        }
        // else{
        //     printf("Queue: [%i]\n",(int)config.msg_qnum);
        // }
    }

    printf("%i mensagens eviadas para msq2\n",M);

    int count = 0;


    while(1){
        if(msgrcv(msq2,&msg2,sizeof(struct mymsg),0,0) == -1){
            perror("Error sending message\n");
        }
        else{
            // printf("Gerando Novo Valor\n");
            if(msg2.mtext[0] == 0) count++;
            else{
                msg1.mtext[0] = get_rand();
                if(msgsnd(msq1,&msg1,sizeof(struct mymsg),0) != 0){
                    perror("Error sending message\n");
                }
            }
        }
        if(count == N) break;
    }

    exit(0);
}

int pid_list[N];
int pstatus_list[N];

void free_resources(void){
    // Fechando as filas
    msgctl(msq1,IPC_RMID,NULL);
    msgctl(msq2,IPC_RMID,NULL);

    unlink("tkn");
}

void catch_interrupts(int val){
    printf("\nSignal received. Terminating the program\n");
    int i;
    for(i = 0; i < N; i++){
        kill(pid_list[i],SIGINT);
    }
    free_resources();
    exit(0);
}

int main(int arc, char **argv){
    printf("Problema 1\n");

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

    int fd_o = open("tkn",O_CREAT | O_WRONLY,0666);
    close(fd_o);

    // Criando as chaves para as filas
    key_t key1 = ftok("tkn", 0);
    key_t key2 = ftok("tkn", 1);

    if(key1 == -1 || key2 == -1){
        perror("Key error\n");
    }

    // Criando as filas de mensagens
    msq1 = msgget(key1, 0666 | IPC_CREAT);
    msq2 = msgget(key2, 0666 | IPC_CREAT);

    // Configurando as filas
    setup_queue(msq1,M);
    setup_queue(msq2,M);

    // Criando Processo Produtor
    pid_list[0] = launch_process(&(pstatus_list[0]),producer,NULL);

    // Criando Processos Consumidores
    int i;
    for(i = 0; i < N; i++){
        pid_list[i+1] = launch_process(&(pstatus_list[i+1]),consumer,&i);   
    }




    

    // Esperando Processos
    for(i = 0; i < N; i++){
        wait(&(pstatus_list[i]));
    }

    // Libertando recursos
    free_resources();

    printf("\n\nProcesso pai concluido!\n");

    return 0;
}


// signal(SIGHUP, catch_interrupts);
// signal(SIGQUIT, catch_interrupts);
// signal(SIGABRT, catch_interrupts);
// signal(SIGSEGV, catch_interrupts);