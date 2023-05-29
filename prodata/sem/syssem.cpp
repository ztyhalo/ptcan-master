#include "syssem.h"
#include <errno.h>
int create_sem(key_t  key,int val)
{
    int   semid;
    extern int errno;
    semid = semget(key, 1, 0666);

    if (semid == -1)
    {
        if(errno == ENOENT)
        {
            semid = semget(key, 1,IPC_CREAT| 0666);
            if (semid == -1)
            {
                printf("=====================zc error\r\n");
                printf("errno=%d\n",errno);
                char *mesg = strerror(errno);
                printf("Mesg:%s\n",mesg);
                perror("Ptcan1: Sem Create Error!\n");
                return -1;
            }
            if(init_sem(semid,val) == -1)
            {
                return -1;
            }
        }
        else
        {
            perror("Sem Get Error!\n");
            return -1;
        }
    }
    else
    {
        zprintf1("sem have create\n");
        if(init_sem(semid,val) == -1)
        {
            zprintf1("sem init fail\n");
            return -1;
        }
    }
    return semid;
}

// 将信号量sem_id设置为init_value
int init_sem(int sem_id,int init_value) {
    union semun sem_union;
    sem_union.val=init_value;
    if (semctl(sem_id,0,SETVAL,sem_union)==-1) {
        perror("Sem init");
        return -1;
    }
    return 0;
}

// 删除sem_id信号量
int del_sem(int sem_id) {
    union semun sem_union;
    if (semctl(sem_id,0,IPC_RMID,sem_union)==-1) {
        perror("Sem delete");
        return -1;
    }
    return 0;
}

// 对sem_id执行p操作
int sem_p(int sem_id) {
    struct sembuf sem_buf;
    sem_buf.sem_num=0;//信号量编号
    sem_buf.sem_op=-1;//P操作
    sem_buf.sem_flg=0;//系统退出前未释放信号量，系统自动释放

    if (semop(sem_id,&sem_buf,1)==-1) {
        perror("Sem P operation");
        return -1;
    }
    return 0;
}

// 对sem_id执行p操作
int sem_p(int sem_id, int milliseconds)
{
    struct sembuf sem_buf;
    struct timespec  timeout;
//    int ret;
    sem_buf.sem_num=0;//信号量编号
    sem_buf.sem_op=-1;//P操作
    sem_buf.sem_flg=0;//系统退出前未释放信号量，系统自动释放

    timeout.tv_sec = (milliseconds / 1000);
    timeout.tv_nsec = (milliseconds - timeout.tv_sec*1000L)*1000000L;

    return(semtimedop(sem_id,&sem_buf,1, &timeout));
}

// 对sem_id执行V操作
int sem_v(int sem_id) {
    struct sembuf sem_buf;
    sem_buf.sem_num=0;
    sem_buf.sem_op=1;//V操作
    sem_buf.sem_flg=0;
    if (semop(sem_id,&sem_buf,1)==-1) {
        perror("Sem V operation");
        return -1;
    }
    return 0;
}

int get_sem_count(int sem_id) {
    union semun sem_union;

    return semctl(sem_id,0, GETVAL, sem_union);

}
