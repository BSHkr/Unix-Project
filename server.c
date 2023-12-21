#include <sys/msg.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

#define TOTAL 1024*1024
#define SIZE 1024 *4
#define CHUNK 1024

struct msgbuf {
    long mtype;
    int mtext;
};

union semun {
    int val;
    struct semid_ds* buf;
    unsigned short* array;
};

int initsem(key_t semkey) {
    union semun semunarg;
    int status = 0, semid;

    semid = semget(semkey, 1, IPC_CREAT | IPC_EXCL | 0600);
    if (semid == -1) {
        if (errno == EEXIST)
            semid = semget(semkey, 1, 0);
    }
    else {
        semunarg.val = 1;
        status = semctl(semid, 0, SETVAL, semunarg);
    }

    if (semid == -1 || status == -1) {
        perror("initsem");
        return (-1);
    }

    return semid;
}

int semlock(int semid) {
    struct sembuf buf;

    buf.sem_num = 0;
    buf.sem_op = -1;
    buf.sem_flg = SEM_UNDO;
    if (semop(semid, &buf, 1) == -1) {
        perror("semlock failed");
        exit(1);
    }
    return 0;
}

int semunlock(int semid) {
    struct sembuf buf;

    buf.sem_num = 0;
    buf.sem_op = 1;
    buf.sem_flg = SEM_UNDO;
    if (semop(semid, &buf, 1) == -1) {
        perror("semunlock failed");
        exit(1);
    }
    return 0;
}

void distribute() {
    int buf[1024 * 1024];
    int data[4][256 * 1024];
    FILE* file;
    int fd[4];
    int i, j;
    char filename[20];
    sprintf(filename, "data.dat");
    file = fopen(filename, "r");
    for (i = 0;i < 1024 * 1024;i++) {
        fscanf(file, "%d", &buf[i]);
    }
    for (i = 0; i < 4; ++i) {
        sprintf(filename, "p%d.dat", i + 1);
        fd[i] = open(filename, O_CREAT | O_WRONLY | O_APPEND, 0666);
    }
    for (i = 0;i < 256 * 1024;i++) {
        for (j = 0;j < 4;j++) {
            data[j][i] = buf[i * 4 + j];
        }
    }
    for (i = 0;i < 4;i++) {
        write(fd[i], data[i], sizeof(int) * 256 * 1024);
    }
}
int main() {
    int msgid, semid;
    int fd, fd1[4];
    char fname[10];
    int a, i, j, k, r, tmp;
    struct msgbuf msg;
    int buf[SIZE];
    int chunk[256][1024];

    struct timeval stime, etime;
    int time_result;
    gettimeofday(&stime, NULL);

    msgid = msgget(0651, IPC_CREAT | 0644);
    semid = initsem(0651);
    distribute();
    for (r = 0;r < 64;r++) {
        for (a = 0; a < 4; a++) {
            switch (fork()) {
            case -1:
                perror("fork");
                exit(1);
                break;
            case 0:
                semlock(semid);
                sprintf(fname, "p%d.dat", a + 1);
                fd = open(fname, O_RDONLY);
                lseek(fd, r * SIZE * sizeof(int), SEEK_SET);
                read(fd, buf, sizeof(int) * SIZE);
                for (i = 0;i < SIZE;i++) {
                    msg.mtext = buf[i];
                    msg.mtype = (msg.mtext / 1024) % 4 + 1;
                    msgsnd(msgid, &msg, sizeof(int), 0);
                }
                close(fd);
                semunlock(semid);
                exit(0);
                break;
            default:
                break;
            }
        }

        for (a = 0;a < 4;a++) {
            wait(NULL);
        }

        for (a = 0; a < 4; a++) {
            switch (fork()) {
            case -1:
                perror("fork");
                exit(1);
                break;
            case 0:
                semlock(semid);
                for (i = 0;i < SIZE;i++) {
                    msgrcv(msgid, &msg, sizeof(int), a + 1, IPC_NOWAIT);
                    buf[i] = msg.mtext;
                }

                for (i = 0;i < SIZE;i++) {
                    for (j = i;j < SIZE;j++) {
                        if (buf[i] > buf[j]) {
                            tmp = buf[i];
                            buf[i] = buf[j];
                            buf[j] = tmp;
                        }
                    }
                }

                for (i = 0;i < 4;i++) {
                    for (j = 0;j < CHUNK;j++) {
                        chunk[r * 4 + i][j] = buf[i * 1024 + j];
                    }
                }


                sprintf(fname, "op%d.dat", a + 1);
                fd1[a] = open(fname, O_CREAT | O_WRONLY | O_APPEND, 0666);
                for (i = 0;i < 4;i++) {
                    write(fd1[a], chunk[r * 4 + i], sizeof(int) * 1024);
                }
                semunlock(semid);
                exit(1);
                break;
            default:
                break;
            }
        }
    }

    gettimeofday(&etime, NULL);
    time_result = etime.tv_usec - stime.tv_usec;
    printf("Server_oriented_io TIMES == %ld %ld %ld\n", stime.tv_usec * 64, etime.tv_usec * 64, time_result);
}
