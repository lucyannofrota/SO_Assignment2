#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/un.h>

#include <pthread.h>

// int processo4(int *childStatus){
//     // Fazendo Fork
//     int myPID = fork();
//     if(myPID == -1) perror("Failed to fork\n");
//     if(myPID == 0){ // Selecionando processo filho
//         printf("Processo 4 |PID: %i\n",getpid());
//         // Pretende-se redirecionar o input para o file: "file.txt" 
//         close(0); // Fechar porta de leitura
//         int fdf = open("file.txt",O_RDONLY,0666); // Abrir/criar o ficheiro em mode de escrita
//         dup2(fdf,0); // Redirecionar porta de leitura
//         execl("/bin/less","less",NULL);
//         exit(1);
//     }
//     else{ // Processo pai
//         return myPID; 
//     }
// }

void* my_thread(void* arg){
    int *data = ((int*) arg);
    // *((int*) arg) = *((int*) arg) + 1;
    printf("Data: %i\n",data[0]);
    // int dt[3] = {0,0,0};
    data[0] += 1;
    data[1] += 1;
    data[2] += 1;
    pthread_exit(data);
    // return NULL;
}

int main(int arc, char **argv){
    printf("Problema 1\n");

    int data1[3] = {18,1,25};
    void *retval;
    // int data2[3] = {3,3,3};

    // Criando Handlers dos threads
    pthread_t thr1, thr2;

    // Criando Threads
    pthread_create(&thr1,NULL,my_thread, data1);
    pthread_join(thr1,(void**)&retval);
    int *dat = ((int*) retval);
    pthread_create(&thr2,NULL,my_thread, retval);

    // Esperando Threads
    pthread_join(thr2,NULL);





    // printf("Processo Pai inicializando processos |PID: %i\n",getpid());


    // // Executando o ultimo processo
    // int childStatus4; processo4(&childStatus4); // Chamando Processo 4
    // wait(&childStatus4);
    printf("Done!\n");

    return 0;
}
