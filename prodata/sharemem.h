/****************************************************
 *************进程间通讯数据库**********************
 *Version: 1.1
 *History: 2017.7.3
 *         2017.7.6添加回调类
 *         2017.7.11 将1个文件拆分为多个文件
****************************************************/

#ifndef __SHAREMEM_H__
#define __SHAREMEM_H__
#include "pro_data.h"

//linux共享内存 不继承线程类
template <class T>
class ShareDataT:public creatdata<T>
{
public:
    key_t  shm_id;
    T               *             readp;
public:
    ShareDataT(){
        shm_id = 20130408;
        readp = NULL;
    }
    ~ShareDataT(){
        printf("share data\n");
        if(this->data != NULL && this->creatmark == 1)
            shmctl(shm_id, IPC_RMID, NULL);
        this->data = NULL;
    }

    int creat_data(int size);
    int creat_data(int size, key_t id);
    int read_creat_data(key_t id = 20130408 ,int size = 0);
};

template <class T>
int ShareDataT<T>::creat_data(int siz)
{
    int shmid ;
    void *shmptr ;
    printf("shm id is %d\n", shm_id);
    if((shmid = shmget(shm_id , siz, SHM_R|SHM_W) ) < 0 )
    {
        if((shmid = shmget(shm_id , siz, SHM_R|SHM_W|IPC_CREAT) ) < 0 ){

            return -1;
        }
        this->creatmark = 1;
    }
    shmptr = shmat(shmid, 0, 0 );

    if (shmptr == (void*) -1 ){
        return -2;
    }

    this->data = (T * )shmptr;
    this->size = siz;
    readp = this->data;

    return 0;
}

template <class T>
int ShareDataT<T>::creat_data(int size, key_t id)
{
    shm_id = id;
    return creat_data(size);
}

template <class T>
int ShareDataT<T>::read_creat_data(key_t id ,int siz)
{
    int shmid ;
    void *shmptr ;
     shm_id = id;
    if((shmid = shmget(shm_id , siz, SHM_R|SHM_W) ) < 0 ){

        return -1;
    }
    shmptr = shmat(shmid, 0, 0 );

    if (shmptr == (void*) -1 ){
        return -2;
    }
    this->data = (T * )shmptr;
    this->size = siz;
    this->readp = this->data;
    return 0;

}
//QT共享内存 不继承线程类
template <class T>
class QT_Share_MemT
{
private:
    QSharedMemory lhshare;
public:
    QString     shm_key;
    T   *         data;
public:
    QT_Share_MemT(){
        shm_key = "lhshare";
        lhshare.setKey(shm_key);
    }
    ~QT_Share_MemT(){

        if(lhshare.isAttached())
        {
            lhshare.detach();
        }
         zprintf3("destory QT_Share_MemT!\n");
    }

    int creat_data(int size);
    T* creat_data(int size, QString keyid);
    void lock_qtshare(void);
    void unlock_qtshare(void);
    void set_data(T * addr, T  val);
};

template <class T>
void QT_Share_MemT<T>::lock_qtshare(void)
{
    lhshare.lock();
}
template <class T>
void QT_Share_MemT<T>::unlock_qtshare(void)
{
    lhshare.unlock();
}

template <class T>
int QT_Share_MemT<T>::creat_data(int siz)
{

    lhshare.setKey(shm_key);
    if(lhshare.isAttached())
    {
        zprintf3("qt share have attach\n");
        lhshare.detach();
    }

    if(!lhshare.create(siz))
    {
        zprintf1("qt1111 share error\n");

        if(lhshare.error() == 4)       //已经存在
        {
            if(!lhshare.attach())
            {
                zprintf1("can't attatch qt share!\n");
                return -2;
            }
        }
        else
         return -1;
    }
    zprintf1("qt share create ok \r\n");
    this->data = (T * )lhshare.data();

    return 0;
}

template <class T>
T* QT_Share_MemT<T>::creat_data(int size, QString keyid)
{
    shm_key = keyid;
    if(creat_data(size) == 0)
        return this->data;
    return NULL;
}

template <class T>
void QT_Share_MemT<T>::set_data(T * addr, T  val)
{
    if(addr == NULL) return ;
    lhshare.lock();
    *addr = val;
    lhshare.unlock();
}

//QT共享内存 不继承线程类
template <class T>
class QTShareDataT:public creatdata<T>
{
private:
    QSharedMemory lhshare;
public:
    QString  shm_key;
public:
    QTShareDataT(){
        shm_key = "lhshare";
        lhshare.setKey(shm_key);
    }
    ~QTShareDataT(){
        if(this->data != NULL && this->creatmark == 1){
            lhshare.detach();
            this->data = NULL;
            this->creatmark = 0;
        }
        zprintf1("destory QTShareDataT!\n");
    }

    int creat_data(int size);
    T* creat_data(int size, QString keyid);
    int read_creat_data(int size , QString keyid = "lhshare");
    void set_data(uint add, T  val);
    void set_data(T * addr, T  val);
    T  get_data(uint add);
    T  get_data(T* addr);
    int  get_data(uint add, T & val);
    int  get_data(T* addr, T & val);
    void noblock_set_data(uint add, T  val);
    T  noblock_get_data(uint add);
    int  noblock_get_data(uint add, T & val);
    void lock_qtshare(void);
    void unlock_qtshare(void);
    int  noblock_get_data_clear(uint add, T & val);
};

template <class T>
void QTShareDataT<T>::lock_qtshare(void)
{
    lhshare.lock();
}
template <class T>
void QTShareDataT<T>::unlock_qtshare(void)
{
    lhshare.unlock();
}

template <class T>
int QTShareDataT<T>::creat_data(int siz)
{
    lhshare.setKey(shm_key);
    if(lhshare.isAttached())
    {
        zprintf3("qt share have attach\n");
        lhshare.detach();
    }

    if(!lhshare.create(siz))
    {
       zprintf1("qt share creat error, error num: %d\n", lhshare.error());

        if ( lhshare.error( ) == QSharedMemory::AlreadyExists )       //已经存在
        {
            zprintf1(" qt create share return 4\r\n");
            if ( !lhshare.attach( ) )
            {
                zprintf1("can't attatch qt share\n");
                return -2;
            }
            zprintf1("attatch qt share\n");
            //change by zc 2022-5-18
            //share if error 4, you should attach it ,then detach and recreate it.
            lhshare.detach();
            zprintf1("qt share recreate\r\n");
            lhshare.create(siz);
        }
        else
        {
            return -1;
        }
    }
    this->creatmark = 1;

    this->data = (T * )lhshare.data();
    this->size = siz;
    this->addp = this->data;

    return 0;
}

template <class T>
T* QTShareDataT<T>::creat_data(int size, QString keyid)
{
    shm_key = keyid;
    if(creat_data(size) == 0)
        return this->data;
    return NULL;
}

template <class T>
int QTShareDataT<T>::read_creat_data(int siz , QString keyid)
{
    shm_key = keyid;

    lhshare.setKey(shm_key);

    if(!lhshare.attach())
    {
        printf("can't attatch qt share\n");
        return -1;
    }
    this->data = (T * )lhshare.data();
    this->size = siz;
    this->addp = this->data;
    return 0;
}

template <class T>
void QTShareDataT<T>::set_data(uint add, T  val)
{
    if(add >= this->size/sizeof(T))
    {
        printf("set data off\n");
        return;
    }
    lhshare.lock();
    memcpy(this->data+add, &val, sizeof(T));
    lhshare.unlock();
}

template <class T>
void QTShareDataT<T>::set_data(T * addr, T  val)
{
    if((addr - this->data) >= this->size/sizeof(T))
    {
        printf("set data off\n");
        return;
    }
    lhshare.lock();
    *addr = val;
    lhshare.unlock();
}


template <class T>
T  QTShareDataT<T>::get_data(uint add)
{
    if(add >= this->size/sizeof(T))
    {
        printf("get data off\n");
        return *this->data;
    }
    T mid;
    lhshare.lock();
    mid = *(this->data+add);
    lhshare.unlock();
    return mid;
}

template <class T>
T  QTShareDataT<T>::get_data(T * addr)
{
    if((addr - this->data) >= this->size/sizeof(T))
    {
        printf("get data off\n");
        return *this->data;
    }
    T mid;
    lhshare.lock();
    mid = *(addr);
    lhshare.unlock();
    return mid;
}

template <class T>
int  QTShareDataT<T>::get_data(uint add, T & val)
{
    if(add >= this->size/sizeof(T))
    {
        printf("get data off\n");
        return -1;
    }

    lhshare.lock();
    val = *(this->data+add);
    lhshare.unlock();
    return 0;
}
template <class T>
int  QTShareDataT<T>::get_data(T* addr, T & val)
{
    if((addr - this->data) >= this->size/sizeof(T))
    {
        printf("get data off\n");
        return -1;
    }

    lhshare.lock();
    val = *(addr);
    lhshare.unlock();
    return 0;
}

template <class T>
void QTShareDataT<T>::noblock_set_data(uint add, T  val)
{
    if(add >= this->size/sizeof(T))
    {
        printf("set data off\n");
        return;
    }
    memcpy(this->data+add, &val, sizeof(T));
}

template <class T>
T  QTShareDataT<T>::noblock_get_data(uint add)
{
    if(add >= this->size/sizeof(T))
    {
        printf("get data off\n");
        return *this->data;
    }
    T mid;
    mid = *(this->data+add);
    return mid;
}

template <class T>
int  QTShareDataT<T>::noblock_get_data(uint add, T & val)
{
    if(add >= this->size/sizeof(T))
    {
        printf("get data off\n");
        return -1;
    }
    val = *(this->data+add);
    return 0;
}

template <class T>
int  QTShareDataT<T>::noblock_get_data_clear(uint add, T & val)
{
    if(add >= this->size/sizeof(T))
    {
        printf("get data off\n");
        return -1;
    }
    val = *(this->data+add);
    memset(this->data+add, 0, sizeof(T));
    return 0;
}





#endif /*__SHAREMEM_H__*/
