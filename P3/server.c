#include <stdio.h>
#include <stdlib.h>

#include <float.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include <semaphore.h>

#include <math.h>

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

// void read_bin(double* list, int len){
//     int fd = open("input.bin",O_RDONLY,0666);
//     write(fd,list,len*sizeof(double));
// }

// void write_asc(double* list)

// void append_bin(double* value){
//     // int fd = open("input.bin",O_CREAT|O_WRONLY|O_TRUNC,0666);
//     // write(fd,table,MAXBUF*sizeof(int));
//     // close(fd);
// }

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
    // shm_open
    int fd = shm_open("s_mem",O_CREAT | O_RDWR | O_TRUNC, 0666);
    if(fd == -1){
        perror("open() error.");
    }
    else{
        ftruncate(fd,1*sizeof(double));
        // fallocate(fd,0,0,1*sizeof(double));
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

    // int sval;

    // mptr[0] = -5.3;

    // Servidor fica a espera que o cliente mande o primeiro valor
    // do{
    //     sem_getvalue(sem, &sval);
    //     printf("Waiting for client\n",sval);
    // }while(sval == 2);


    // Servidor fica a espera que o cliente mande o primeiro valor
    // do{
    //     sem_getvalue(sem, &sval);
    //     printf("Waiting for client\n",sval);
    //     sleep(1);
    // }while(sval == 2);

    // Inicializando buffer
    // struct my_buffer buffer;
    int len = 0;
    do{
        // sem_getvalue(sem, &sval);
        // if(sval == 1){ // Servidor fica a espera que o cliente mande o primeiro valor
        //     printf("Waiting for client\n",sval);
        //     sem_post(sem);
        //     continue;    
        // }
        sem_wait(sem_r);
        if(mptr[0] != DBL_MIN){
            write_bin(mptr,0);
            len++;
        }
            // add_to_buffer(&buffer,mptr[0]);
            // printf("Server %.2f\n",(double)mptr[0]);
        sem_post(sem_w);
    }while(mptr[0] != DBL_MIN);

    double list[len];

    // read_bin(list,len);

    // Lendo o file "input.bin"
    fd = open("input.bin",O_RDONLY,0666);
    read(fd,list,len*sizeof(double));

    // Escrevendo o file "output.asc"
    fd = open("output.asc",O_CREAT | O_WRONLY | O_TRUNC,0666);
    int i;
    char c_buffer[strlen("-0.")-FLT_MIN_10_EXP+1];
    int n_len = 0;
    float mult = 4.0;
    for(i = 0; i < len; i++){
        // n_len = snprintf(NULL,0,"%f\n",list[i]);
        n_len = snprintf(NULL,0,"%.2f\n",list[i]*mult);
        snprintf(c_buffer,(n_len+1),"%.2f\n",list[i]*mult);
        printf("Valor: %s",c_buffer);
        // sprintf(c_buffer,"%.2f\0\n",list[i]);
        write(fd,c_buffer,(n_len)*sizeof(char));
        // printf("%i | %f\n",i,(float)list[i]);
    }
    close(fd);

    // Lendo o file "output.asc"
    // fd = open("output.asc",O_RDONLY,0666);

    // int count = 0;
    // double num_line;
    // while(read_line(&file,&num_line)){
    //     printf("Client %.2f\n",(double)mptr[0]);
    //     sem_post(sem_r);
    //     // sleep(1);
    //     count++;
    // }

    // ftoa()

    // sem_wait()
    // if(sem_init(&sem, 1, 0) != 0) perror("sem_init() error.");

    // Print Buffer
    // print_buffer(&buffer);


    // Libertando recursos
    free_resources();
    // sem_close("serv_c_sem");

    printf("\n\nDone!\n");

    return 0;
}
