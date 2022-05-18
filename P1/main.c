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

int pid_list[N]; // Array utilizada para fazer track dos PIDs dos processos filhos. Essa lista tem importancia crucial para finalizar os processos corretamente caso ocorra um erro
int pstatus_list[N];

int msq1, msq2;

struct mymsg {
    long   mtype;      /* Message type. */
    int   mtext[1];    /* Payload. */
};

void free_resources(void){
    // Fechando as filas
    msgctl(msq1,IPC_RMID,NULL);
    msgctl(msq2,IPC_RMID,NULL);

    unlink("tkn");
}

void catch_interrupts(int val){
    printf("\nSignal received. Terminating the program\n");
    int i;
    for(i = 0; i < N; i++){ // Envia 'SIGTERM' para todos os processo filhos
        kill(pid_list[i],SIGTERM);
    }
    free_resources(); // Liberta os recursos utilizados
    exit(0); // Finaliza esse processo
}

void setup_queue(int msq,int n_terms){
    // Inicialização da queue


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
    // childStatus - Ponteiro utilizado para que o processo pai tenha acesso ao status do processo filho
    // myfunc      - Ponteiro para uma função que recebe um ponteiro void como argumento e retona void. Esse parametro é utilizado para dar funcionalidade ao processo filho lançado
    // arg         - Ponteiro que indica o argumento de entrada da função apontada por 'myfunc'

    printf("Processo Realizando Fork|PID %i\n",getpid());
    // Fazendo Fork
    int myPID = fork();
    if(myPID == -1) perror("Failed to fork\n");
    if(myPID == 0){ // Selecionando processo filho
        printf("Processo Filho Iniciado|PID %i\n",getpid());
        myfunc(arg); // Executanto a função pretendida pelo processo. Essa função será 'consumer()' ou 'producer()'
        exit(1);
    }
    else{ // Processo pai
        return myPID; 
    }
}

int get_rand(void){
    // Função para gerar numero aleatorio dentro do intervalo desejado
    return rand()%((N_MAX+1)-N_MIN) + N_MIN;
}

int has_duplicates(int *ptr, int len, int new){
    // Função que verifica se o já existe o valor 'new' no array apontado por 'ptr'
    int i;
    for(i = 0; i < len; i++){
        if(ptr[i] == new) return 1;
    }
    return 0;
}

void print_buffer(int *ptr,int len){
    int i;
    for (i = 0; i < len; i++)
    {
        printf("%i | %i\n", i+1,ptr[i]);
    }
}

void write_to_file(int *buff,int ref,int len){
    char name[10];
    sprintf(name,"Key_%i",ref);
    int stdout_ = dup(1);
    close(1); // Fechar porta de escrita
    int fdf = open(name,O_WRONLY|O_CREAT,0666); // Abrir/criar o ficheiro em mode de escrita
    dup2(fdf,1); // Redirecionar porta de escrita
    print_buffer(buff, len);
    close(fdf); // Fechar file
    dup2(stdout_,1); // Retornar porta de escrita ao stdout
}

void consumer(void* arg){

    int arg_ref = 0, arg_num;

    if(arg != NULL){
        arg_ref = ((int *)arg)[0];
        arg_num = ((int *)arg)[1];
    }
    else{
        perror("Missing Arguments.");
    }

    // O argumento recebido que é armazenado na variavel 'arg_ref' é utilizado para gerar o ficheiro de output 'Key_{arg_ref}'

    struct mymsg msg1, msg2;

    msg2.mtype = 1;

    int numbers = 1;

    int buff[arg_num];

    do{
        if(msgrcv(msq1,&msg1,sizeof(struct mymsg),0,0) == -1){ // Nessa instrução é feita uma tentativa de receber uma mensagem presente na queue 'msq1'
            perror("Error sending message\n");
        }
        else{
            if(has_duplicates(buff,arg_num,msg1.mtext[0])){ // Caso seja bem sucedido em receber a mensagem é feita a verificação se o valor recebido está duplicado
                printf("Consumer| Valor Duplicado\n");
                msg2.mtext[0] = 1;
                if(msgsnd(msq2,&msg2,sizeof(struct mymsg),0) != 0){ // Envio de mensagem para queue 'msq2' com o valor 1, que significa que é para gerar um novo valor
                    perror("Error sending message\n");
                }
            }
            else{
                buff[numbers-1] = msg1.mtext[0];
                numbers++;

                if(numbers > arg_num){
                    msg2.mtext[0] = 0;
                    if(msgsnd(msq2,&msg2,sizeof(struct mymsg),0) != 0){ // Envio de mensagem para queue 'msq2' com o valor 0, que significa que esse processo já recebeu todos os valores necessarios
                        perror("Error sending message\n");
                    }
                    break; // Quebra o loop caso já tenha recebido todos os valores necessarios
                }
                else{
                    msg2.mtext[0] = 1;
                    if(msgsnd(msq2,&msg2,sizeof(struct mymsg),0) != 0){ // Envio de mensagem para queue 'msq2' com o valor 1, que significa que é para gerar um novo valor
                        perror("Error sending message\n");
                    }
                }
            }
        }
    }while(1);

    write_to_file(buff,arg_ref,arg_num);

    printf("Consumer| Processo Concluido!\n");

    exit(0);
}

void producer(void* arg){

    int arg_M = 0, arg_N = 0;

    if(arg != NULL){
        arg_M = ((int*) arg)[0];
        arg_N = ((int*) arg)[1];
    }
    else{
        perror("Missing Arguments.");
    }

    struct mymsg msg1, msg2;

    msg2.mtype = 1;
    msg2.mtext[0] = 1;

    int i;

    printf("Producer| [N_number,M,N] = [%i,%i]\n",arg_M,arg_N);

    // Enviando M valores para a queue 'msq2'
    for(i = 0; i < arg_M; i++){
        // Send to queue
        if(msgsnd(msq2,&msg2,sizeof(struct mymsg),0) != 0){
            perror("Error sending message\n");
        }
        struct msqid_ds config;
        if(msgctl(msq2,IPC_STAT,&config) != 0){
        
            perror("Failed to get info\n");
        }
    }

    printf("%i mensagens eviadas para msq2\n",M);

    int count = 0;

    while(1){
        if(msgrcv(msq2,&msg2,sizeof(struct mymsg),0,0) == -1){ // Leitura de valor da queue 'msq2'
            perror("Error sending message\n");
        }
        else{
            if(msg2.mtext[0] == 0) count++; // Caso o valor lido seja '0', indica que algum processo consumidor concluiu o seu trabalho
            else{ // Caso o valor lido seja '1', indica que deve-se enviar um novo valor para a queue 'msq1'
                msg1.mtext[0] = get_rand();
                if(msgsnd(msq1,&msg1,sizeof(struct mymsg),0) != 0){
                    perror("Error sending message\n");
                }
            }
        }
        if(count == arg_N) break; // Caso N processos tenham terminado, esse processo deve terminar em seguida
    }

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
    int args_producer[] = {M,N};
    pid_list[0] = launch_process(&(pstatus_list[0]),producer,args_producer); 

    // Criando Processos Consumidores
    int i;
    int args_consumer[] = {0, N_NUMBERS};
    for(i = 0; i < N; i++){
        args_consumer[0] = i;
        pid_list[i + 1] = launch_process(&(pstatus_list[i + 1]), consumer, args_consumer);
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