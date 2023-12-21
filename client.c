#include "mytest.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define ARRAY_SIZE 8 * 8
#define NUM_PROCS 4
#define CHUNK_SIZE 8
#define NUM_PIPES 12

int data[4][2 * 8];
int rcv_data[64];

int client1_dat[64];
int client2_dat[64];
int client3_dat[64];
int client4_dat[64];

int pipes[NUM_PIPES][2]; // 3개씩 client에 분배
/*12 13 14 21 23 24 31 32 34 41 42 43*/

void distribute() {
    int buf[8 * 8];
    FILE* file;
    int fd[4];
    int i, j;
    char filename[20];
    
    sprintf(filename, "data.dat");
    file = fopen(filename, "r");
    
    for (i = 0;i < 8 * 8;i++) {
        fscanf(file, "%d", &buf[i]);
    }
    
    for (i = 0; i < 4; ++i) {
        sprintf(filename, "P%d분산데이터.dat", i + 1);
        fd[i] = open(filename, O_CREAT | O_WRONLY | O_APPEND, 0666);
    }
    
    for (i = 0;i < 2 * 8;i++) {
        for (j = 0;j < 4;j++) {
            data[j][i] = buf[i * 4 + j];
        }
    }
    
    for (i = 0;i < 4;i++) {
        write(fd[i], data[i], sizeof(int) * 2 * 8);
    }

    
}

void WRclient_data() {
    int i;
    /*P1*/
    for(i = 0;i < 3; i++) {
        close(pipes[i][0]);
        write(pipes[i][1], data[0], sizeof(int) * 16);
        close(pipes[i][1]);
    }
    printf("client1 write clear\n");
    /*P2*/
    for(i = 3;i < 6; i++) {
        close(pipes[i][0]);
        write(pipes[i][1], data[1], sizeof(int) * 16);
        close(pipes[i][1]);
    }
    printf("client2 write clear\n");
    /*P3*/
    for(i = 6;i < 9; i++) {
        close(pipes[i][0]);
        write(pipes[i][1], data[2], sizeof(int) * 16);
        close(pipes[i][1]);
    }
    printf("client3 write clear\n");
    /*P4*/
    for(i = 9;i < 12; i++) {
        close(pipes[i][0]);
        write(pipes[i][1], data[3], sizeof(int) * 16);
        close(pipes[i][1]);
    }
    printf("client4 write clear\n");
}

void RDclient_dat(int cnt) { 
    int n, a, b, i, j;
    int fd[4];
    char filename[20];

    /*P1 - 3, 6, 9*/
    for(a = 3;a <= 9; a += 3) {
        close(pipes[a][1]);
        read(pipes[a][0], client1_dat, sizeof(int) * 64);
        close(pipes[a][0]);
        for(b = 0; b < 16; b++) {
            rcv_data[client1_dat[b] % 64] = client1_dat[b];
        }
    }
    printf("Read성공 client1\n");

    /*P2 - 0, 7, 10*/ 
    close(pipes[0][1]);
    read(pipes[0][0], client2_dat, sizeof(int) * 64);
    close(pipes[0][0]);
    for(b = 0; b < 16; b++) {
        rcv_data[client2_dat[b] % 64] = client2_dat[b];
    }

    close(pipes[7][1]);
    read(pipes[7][0], client2_dat, sizeof(int) * 64);
    close(pipes[7][0]);
    for(b = 0; b < 16; b++) {
        rcv_data[client2_dat[b] % 64] = client2_dat[b];
    }

    close(pipes[10][1]);
    read(pipes[10][0], client2_dat, sizeof(int) * 64);
    close(pipes[10][0]);
    for(b = 0; b < 16; b++) {
        rcv_data[client2_dat[b] % 64] = client2_dat[b];
    }
    printf("Read성공 client2\n");

    /*P3 - 1, 4, 11*/
    close(pipes[1][1]);
    read(pipes[1][0], client3_dat, sizeof(int) * 64);
    close(pipes[1][0]);
    for(b = 0; b < 16; b++) {
        rcv_data[client3_dat[b] % 64] = client3_dat[b];
    }

    close(pipes[4][1]);
    read(pipes[4][0], client3_dat, sizeof(int) * 64);
    close(pipes[4][0]);
    for(b = 0; b < 16; b++) {
        rcv_data[client3_dat[b] % 64] = client3_dat[b];
    }

    close(pipes[11][1]);
    read(pipes[11][0], client3_dat, sizeof(int) * 64);
    close(pipes[11][0]);
    for(b = 0; b < 16; b++) {
        rcv_data[client3_dat[b] % 64] = client3_dat[b];
    }
    printf("Read성공 client3\n");

    /*P4*/
    for(a = 2;a <= 8; a += 3) {
        close(pipes[a][1]);
        read(pipes[a][0], client4_dat, sizeof(int) * 64);
        close(pipes[a][0]);
        for(b = 0; b < 16; b++) {
            rcv_data[client4_dat[b] % 64] = client4_dat[b];
        }
    }
    printf("Read성공 client4\n");

    for(i = 0;i < 4; i++) {
        sprintf(filename, "Node#%d.dat", i + 1);
        fd[i] = open(filename, O_CREAT | O_WRONLY | O_APPEND, 0666);
    }

   
    for(j = 0; j < 64; j++) {
        if((rcv_data[j] / 8) % 4 == 0) 
            write(fd[0], &rcv_data[j], sizeof(int)); 
        else if(rcv_data[j] / 8 % 4 == 1)
            write(fd[1], &rcv_data[j], sizeof(int)); 
        else if(rcv_data[j] / 8 % 4 == 2)
            write(fd[2], &rcv_data[j], sizeof(int)); 
        else if(rcv_data[j] / 8 % 4 == 3)
            write(fd[3], &rcv_data[j], sizeof(int));
    }
}

void mkpipe() {
    /*파이프 생성*/
    int i;
    for (i = 0; i < NUM_PIPES; ++i) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }
}

int main() {
    int cnt, n, i, j;
    pid_t pid;
    char name[20];
    int data[65];
    int fd[4];

    mkpipe();

    distribute(); // 데이터 분산

    /*데이터 쓰기 반복*/
    for (cnt = 0; cnt < NUM_PROCS; ++cnt) {
        switch (pid = fork()) {
        case -1:
            perror("fork");
            exit(EXIT_FAILURE);
            break;

        case 0: // Child Process - 쓰기
            WRclient_data();
            exit(0);
            break;
        default:
            wait(NULL);
            RDclient_dat(cnt + 1);
            exit(1);
            break;
        }
    }   
    for(i=0;i<4;i++){
        wait(NULL);
    }
    return 0;
}
