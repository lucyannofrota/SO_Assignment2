#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
// #include <fcntl.h>
#include <sys/types.h>
// #include <sys/un.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include <pthread.h>
#include <time.h>
#include <stdlib.h>

#include <sys/msg.h>

#include <errno.h>


#define N_MAX 49
#define N_MIN 1


int msq1, msq2;
// key_t key1, key2;

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
        printf("Queue Size: %i\n",(int)config.msg_qbytes);
        if(msgctl(msq,IPC_SET,&config) != 0){
            perror("Failed to setup msq\n");
        }
    }
}

// float sumf(float a,float b){
//     return a+b;
// }

// float exec_func(float (*myfunc)(float,float)){
//     return myfunc(3.5,1.5);
// }

// int launch_process(int *childStatus){
//     // Fazendo Fork
//     int myPID = fork();
//     if(myPID == -1) perror("Failed to fork\n");
//     if(myPID == 0){ // Selecionando processo filho
        
//         exit(1);
//     }
//     else{ // Processo pai
//         return myPID; 
//     }
// }

int get_rand(void){
    return rand()%((N_MAX+1)-N_MIN) + N_MIN;
}

void* consumer_thread(void* arg){
    struct mymsg msg2;

    msg2.mtype = 1;
    msg2.mtext[0] = 1;

    pthread_exit(NULL);
}

void* producer_thread(void* arg){
    srand(7);
    // srand(time(NULL));
    int M = ((int*) arg)[0];
    int N = ((int*) arg)[1];

    struct mymsg msg2;

    msg2.mtype = 1;
    msg2.mtext[0] = 1;

    int i;

    // Enviando as mesagens para msq2
    for(i = 0; i < M; i++){//get_rand();
        // msg2.mtext[0] = 'c';
        // printf("Num: %i\n",msg2.mtext[0]);
        // Send to queue
        if(msgsnd(msq2,&msg2,sizeof(struct mymsg),IPC_NOWAIT) != 0){
            perror("Error sending message\n");
        }
        struct msqid_ds config;
        if(msgctl(msq2,IPC_STAT,&config) != 0){
            perror("Failed to get info\n");
        }
        else{
            printf("Queue: [%i]\n",(int)config.msg_qnum);
        }
    }

    printf("%i mensagens eviadas para msq2\n",M);



    

    // int data = arg;
    // *((int*) arg) = *((int*) arg) + 1;
    // printf("Data: %i\n",data[0]);
    // int dt[3] = {0,0,0};
    // data[0] += 1;
    // data[1] += 1;
    // data[2] += 1;

    pthread_exit(NULL);
    // printf("asdas: %i\n",data);
    // return NULL;
}


// void* launch_thread(void* arg){
//     int *data = ((int*) arg);
//     // *((int*) arg) = *((int*) arg) + 1;
//     printf("Data: %i\n",data[0]);
//     // int dt[3] = {0,0,0};
//     data[0] += 1;
//     data[1] += 1;
//     data[2] += 1;
//     pthread_exit(data);
//     // return NULL;
// }

// void launch_process(void){

// }

int MAIN_PID;

void catch_sigint(int val){
    kill(MAIN_PID,SIGINT);
    exit(0);
}

int main(int arc, char **argv){
    printf("Problema 1\n");

    MAIN_PID = getpid();

    int N = 4;
    int M = 40;

    // Configurando Sinais
    signal(SIGINT, catch_sigint);

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
    setup_queue(msq1,N);
    setup_queue(msq2,M);

    // int msgctl(int msqid, int cmd, struct msqid_ds *buf);



    // key1 = ftok("progfile", 65);


    // int data1[3] = {18,1,25};
    // void *retval;
    // int data2[3] = {3,3,3};

    // Criando Handlers dos threads
    pthread_t thr1;

    // float (*fun_ptr)(float,float) = sumf;



    // float ret = exec_func(fun_ptr);

    // printf("Sum: %f\n",ret);


    int arg1 = {M,N};

    // Criando Threads
    pthread_create(&thr1,NULL,producer_thread, &arg1);
    pthread_join(thr1,NULL);
    // pthread_join(thr1,(void**)&retval);
    // int *dat = ((int*) retval);
    // pthread_create(&thr2,NULL,my_thread, retval);

    // Esperando Threads
    // pthread_join(thr2,NULL);





    // printf("Processo Pai inicializando processos |PID: %i\n",getpid());


    // // Executando o ultimo processo
    // int childStatus4; processo4(&childStatus4); // Chamando Processo 4
    // wait(&childStatus4);
    
    // Fechando as filas
    msgctl(msq1,IPC_RMID,NULL);
    msgctl(msq2,IPC_RMID,NULL);

    printf("Done!\n");

    return 0;
}
