/****************************************************
 *************进程间通讯数据库**********************
 *Version: 1.1
 *History: 2017.7.3
 *         2017.7.6添加回调类
****************************************************/


#ifndef PRO_DATA_H
#define PRO_DATA_H
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <vector>
#include <QVector>
#include <QString>
#include <map>
#include <stdio.h>
#include "syssem.h"
#include <QSharedMemory>
#include <QDebug>
#include <sys/msg.h>
#include <pthread.h>
#include <semaphore.h>
#include "e_poll.h"
#include "netprint.h"

using namespace std;

#define LINUX_MSG_MAX   8192


#define timeprf(...) do { time_t now;\
                        struct tm timenow;\
                        char stbuf[32];\
    time(&now);\
    localtime_r(&now, &timenow);\
    asctime_r(&timenow,stbuf);\
    stbuf[strlen(stbuf)-1] = '\0';\
    printf("%s ", stbuf);\
    printf( __VA_ARGS__);}while(0)

template <class T>
class creatdata
{
public:
    T *         data;
    T *         addp;
    int         size;
    uchar       creatmark;
public:
    creatdata(T* addr= NULL, int sz = 0){
        addp = NULL;
        data = addr;
        size = sz;
        creatmark = 0;
    }
    ~creatdata(){

        if(data != NULL && creatmark == 1){
            delete [] data;
            data = NULL;
            qDebug("creatdata delete");
        }
    }

    void data_init(T * add = NULL, int siz = 0);
    virtual int creat_data(int size);
    virtual void set_data(uint add, T  val);
    virtual T  get_data(uint add);
    virtual int  get_data(uint add, T & val);
};

template <class T>
void creatdata<T>::data_init(T* add, int siz)
{
    data = add;
    size = siz;
}

template <class T>
int creatdata<T>::creat_data(int size)
{
   data = new T[size];
   if(data != NULL)
       return 0;
   return -1;
}

template <class T>
void creatdata<T>::set_data(uint add, T  val)
{
    if(add >= size/sizeof(T))
    {
        printf("set data off\n");
        return;
    }

    memcpy(data+add, &val, sizeof(T));
}

template <class T>
T  creatdata<T>::get_data(uint add)
{
    if(add >= size/sizeof(T))
    {
        printf("get data off\n");
        return *data;
    }
    return *(data+add);
}

template <class T>
int  creatdata<T>::get_data(uint add, T & val)
{
    if(add >= size/sizeof(T))
    {
        printf("get data off\n");
        return -1;
    }

    val = *(data+add);
    return 0;
}


template <class T1, class T2>
class creathead:public creatdata<T2>
{
public:
    T1 *       head;
    char *     dataadd;
public:
    creathead(){
        head = NULL;
        dataadd = NULL;
    }
    ~creathead(){
        printf("creathead\n" );
        if(dataadd != NULL && this->creatmark == 1)
        {
            delete [] dataadd;
        }
        this->data = NULL;
    }

    int creat_data(int size);
    int creat_data(int size, int headsize = 0);
};

template <class T1, class T2>
int creathead<T1,T2>::creat_data(int size)
{
     dataadd = new char[size];
     this->data = (T2 *)dataadd;
     this->creatmark = 1;
     return 0;
}

template <class T1, class T2>
int creathead<T1,T2>::creat_data(int size, int headsize)
{

    dataadd = new char[size];
    this->data = (T2 *)(dataadd+headsize);
    this->head = (T1 *) dataadd;
    this->creatmark = 1;
    this->size = size;
    return 0;
}

//线程回调类
template <class DTYPE, class F>
class Call_B_T:public Pth_Class
{
public:
    F * father;
public:
    ~Call_B_T(){
         zprintf3("destory Call_B_T!\n");
    }

    int (*z_callbak)(F * pro, DTYPE val);

    int set_z_callback(int  (*callback)(F * pro, DTYPE), F * arg);
    int z_pthread_init(int  (*callback)(F * pro, DTYPE), F * arg);
};

template <class DTYPE, class F>
int  Call_B_T<DTYPE,F>::set_z_callback(int  (*callback)(F * pro, DTYPE), F * arg)
{
    if(callback != NULL)
    {
        z_callbak = callback;
        father = arg;
        return 0;
    }
    return -1;
}

template <class DTYPE, class F>
int  Call_B_T<DTYPE,F>::z_pthread_init(int  (*callback)(F * pro, DTYPE), F * arg)
{
   set_z_callback(callback, arg);
   start();
    return 0;
}
//数据buf操作类
template<class DTYPE, int N = 2>
class Z_Buf_T
{
public:
    DTYPE                buf[N];
    uint                 sem_wr;
    uint                 sem_rd;
    sem_t                mgsem;
    pthread_mutex_t      sem_mut;

public:
    Z_Buf_T()
    {
        sem_wr = 0;
        sem_rd = 0;
        memset(buf, 0x00, sizeof(buf));
        sem_init(&mgsem, 0, 0);
        pthread_mutex_init(&sem_mut, NULL);
    }
    ~Z_Buf_T(){
        sem_destroy(&mgsem);
        pthread_mutex_destroy(&sem_mut);
    }

    void buf_write_data(DTYPE val);
    DTYPE* buf_wr_data(DTYPE val);
    void buf_write_data(DTYPE * val);
    int  buf_read_data(DTYPE & val);
    int  wait_buf_sem(void);

};

template<class DTYPE, int N>
void Z_Buf_T<DTYPE,N>::buf_write_data(DTYPE val)
{
    pthread_mutex_lock(&sem_mut);
    buf[sem_wr] = val;
    sem_wr++;
    sem_wr %= N;

//    timeprf("write datanum %d\n",datanum);
    if(sem_wr == sem_rd)
    {
        zprintf1("sembuf_t over\n");
    }
    pthread_mutex_unlock(&sem_mut);
    sem_post(&mgsem);

}

template<class DTYPE, int N>
DTYPE* Z_Buf_T<DTYPE,N>::buf_wr_data(DTYPE val)
{
    DTYPE* ret = NULL;
    pthread_mutex_lock(&sem_mut);
    buf[sem_wr] = val;
    ret = &buf[sem_wr];
    sem_wr++;
    sem_wr %= N;

//    timeprf("write datanum %d\n",datanum);
    if(sem_wr == sem_rd)
    {
        zprintf1("sembuf_t over\n");
    }
    pthread_mutex_unlock(&sem_mut);
    return  ret;

}

template<class DTYPE, int N>
void Z_Buf_T<DTYPE,N>::buf_write_data(DTYPE * val)
{
    pthread_mutex_lock(&sem_mut);
    buf[sem_wr] = *val;
    sem_wr++;
    sem_wr %= N;

//    timeprf("write datanum\n");
    if(sem_wr == sem_rd)
    {
        zprintf1("sembuf_t over\n");
    }
    pthread_mutex_unlock(&sem_mut);
    sem_post(&mgsem);

}

template<class DTYPE, int N>
int Z_Buf_T<DTYPE,N>::buf_read_data(DTYPE &val)
{
//    timeprf("read datanum %d\n",datanum);
    if(sem_rd == sem_wr) return -1;
    val = buf[sem_rd];
    sem_rd++;
    sem_rd %= N;
    return 0;
}
template<class DTYPE, int N>
int Z_Buf_T<DTYPE,N>::wait_buf_sem(void)
{
    sem_wait(&mgsem);
    return 0;
}

//带线程回调的buf操作类
template<class DTYPE, int N = 2, class F=void>
class Pth_Buf_T:public Z_Buf_T<DTYPE,N>,public Call_B_T<DTYPE, F>
{
public:
    void run();
};

template <class DTYPE,int N, class F>
void  Pth_Buf_T<DTYPE,N,F>::run(void)
{
    while(1)
    {
        if(this->wait_buf_sem() == 0)
        {
            DTYPE val;
            if(this->buf_read_data(val) == 0)
            {
                 if(this->z_callbak != NULL)           //执行操作
                 {
                     this->z_callbak(this->father , val);
                 }
            }

        }
    }
}

#endif /*PRO_DATA_H*/
