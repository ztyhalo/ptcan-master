/****************************************************
 *************进程间通讯数据库**********************
 *Version: 1.1
 *History: 2017.7.3
 *         2017.7.6添加回调类
 *         2017.7.11 将1个文件拆分为多个文件
 ****************************************************/

#ifndef __SEMSHARE_H__
#define __SEMSHARE_H__
#include "sharemem.h"
// linux共享内存加信号 不继承线程类
template < class T >
class Sem_Share_Data : public ShareDataT< T >
{
public:
    int   semid;
    uint  bufsize;
    uint *rd_buf_p;
    uint *wr_buf_p;
    key_t sem_key;

public:
    Sem_Share_Data()
    {
        semid    = 0;
        sem_key  = 19860610;
        bufsize  = 0;
        rd_buf_p = NULL;
        wr_buf_p = NULL;
    }
    Sem_Share_Data(uint sz, key_t semkey = 19860610, key_t sharekey = 20130410)
    {
        creat_sem_data(sz, semkey, sharekey);
    }

    int creat_sem_data(uint sz, key_t semkey = 19860610, key_t sharekey = 20130410);
    int write_send_data(T val);
    int read_send_data(T &val);
    int wait_thread_sem(void);
};

template < class T >
int Sem_Share_Data< T >::creat_sem_data(uint sz, key_t semkey, key_t sharekey)
{
    int   err  = 0;
    uint *midp = NULL;
    printf("share key is %d\n", sharekey);
    err     = this->creat_data(sz + sizeof(uint) * 2, sharekey);
    bufsize = sz / sizeof(T);
    if(err == 0)
    {
        midp      = (uint *)(this->data + bufsize);
        rd_buf_p  = midp;
        wr_buf_p  = midp + 1;
        *rd_buf_p = 0;
        *wr_buf_p = 0;
        semid     = create_sem(semkey, 0);
        err       = semid > 0 ? 0 : -2;
    }
    return err;
}

template < class T >
int Sem_Share_Data< T >::write_send_data(T val)
{
    uint mid = *wr_buf_p;
    if(bufsize == 1 && mid == 0)
        this->set_data(mid, val);
    else if(bufsize == 1)
        printf("write off \n");
    else
        this->set_data(mid, val);
    mid++;
    if(bufsize > 1)
    {
        mid %= bufsize;
        if(mid == *rd_buf_p)
        {
            printf("write off %d %d\n", mid, *rd_buf_p);
            return -1;
        }
    }

    *wr_buf_p = mid;
    sem_v(semid);
    return 0;
}

template < class T >
int Sem_Share_Data< T >::read_send_data(T &val)
{
    uint mid = *rd_buf_p;

    if(bufsize == 1)
        mid = 0;
    this->get_data(mid, val);

    if(bufsize == 1)
    {
        *wr_buf_p = 0;
    }
    else
    {
        mid++;
        mid %= bufsize;

        *rd_buf_p = mid;
    }
    return 0;
}
template < class T >
int Sem_Share_Data< T >::wait_thread_sem(void)
{
    if(sem_p(semid) == 0)
        return 0;
    else
        return -1;
}

// linux共享内存加信号 继承线程类
template < class T, class FAT >
class Sem_Pth_Data : public Sem_Share_Data< T >, public Call_B_T< T, FAT >
{
public:
    void run();
};

template < class T, class FAT >
void Sem_Pth_Data< T, FAT >::run(void)
{
    while(1)
    {
        if(sem_p(this->semid) == 0)
        {
            T val;
            if(read_send_data(val) == 0)
            {
                if(this->z_callbak != NULL)    //执行操作
                {
                    this->z_callbak(this->father, val);
                }
            }
        }
    }
}
// qt共享内存加linux信号 不继承线程类
template < class T >
class Sem_Qt_Data : public QTShareDataT< T >
{
public:
    int       semid;
    uint      bufsize;
    uint16_t *rd_buf_p;
    uint16_t *wr_buf_p;
    uint16_t *count_p;
    uint16_t  mark;
    uint32_t  zjs;
    key_t     sem_key;

public:
    Sem_Qt_Data()
    {
        semid    = 0;
        sem_key  = 19860610;
        bufsize  = 0;
        rd_buf_p = NULL;
        wr_buf_p = NULL;
        count_p  = NULL;
        zjs      = 0;
    }
    Sem_Qt_Data(uint sz, key_t semkey = 19860610, QString sharekey = "lhshare")
    {
        creat_sem_data(sz, semkey, sharekey);
    }
    ~Sem_Qt_Data()
    {
        zprintf3("destory Sem_Qt_Data!\n");
        //        del_sem(semid);
    }

    int creat_sem_data(uint sz, key_t semkey = 19860610, QString sharekey = "lhshare", int al = 8);
    int write_send_data(T val);
    int read_send_data(T &val);
    int wait_thread_sem(void);
};

template < class T >
int Sem_Qt_Data< T >::creat_sem_data(uint sz, key_t semkey, QString sharekey, int al)
{
    int       err     = 0;
    uint16_t *midp    = NULL;
    T        *midaddr = NULL;
    zprintf3("key: sem key = %d\n", semkey);
    int aline = ((sizeof(uint16_t) * 3) % al) ? 1 : 0;
    aline += ((sizeof(uint16_t) * 3) / al);
    zprintf3("report io share creat: creat qt share size = < %d > \n", sz + aline * al);
    midaddr = this->creat_data(sz + aline * al, sharekey);
    bufsize = sz / sizeof(T);
    if(midaddr != NULL)
    {
        midp      = (uint16_t *)(this->data + bufsize);
        wr_buf_p  = midp;
        rd_buf_p  = midp + 1;
        count_p   = midp + 2;
        *rd_buf_p = 0;
        *wr_buf_p = 0;
        *count_p  = 0;
        mark      = 0;
        zjs       = 0;
        semid     = create_sem(semkey, 0);
        err       = semid > 0 ? 0 : -2;
    }
    else
        err = -1;
    return err;
}

template < class T >
int Sem_Qt_Data< T >::write_send_data(T val)
{
    this->lock_qtshare();
    uint16_t mid   = *wr_buf_p;
    uint16_t count = *count_p;
    if(bufsize == 1 && mid == 0)
        this->noblock_set_data(mid, val);
    else if(bufsize == 1)
        zprintf1("write off \n");
    else
        this->noblock_set_data(mid, val);
    mid++;
    if(bufsize > 1)
    {
        mid %= bufsize;
        if(mid == *rd_buf_p)
        {
            zprintf1("write off %d %d\n", mid, *rd_buf_p);
            this->unlock_qtshare();
            return -1;
        }
    }
    if(count < 65535)
        count++;
    //    printf("count++\n");

    *wr_buf_p = mid;
    *count_p  = count;
    this->unlock_qtshare();
    sem_v(semid);
    return 0;
}

template < class T >
int Sem_Qt_Data< T >::read_send_data(T &val)
{
    this->lock_qtshare();
    uint16_t mid   = *rd_buf_p;
    uint16_t count = *count_p;
    int      i     = 0;
    int      zs    = get_sem_count(semid);
    zjs++;
    if(count > (zs + 1))
    {
        i = count - zs;
    }
    else
    {
        this->unlock_qtshare();
        return -1;
    }
    else i = 1;
    {
        if(count >= 256)
        {
            zprintf1("read cout err %d rd %d wr %d sem %d!!!\n", count,
                     *rd_buf_p, *wr_buf_p, get_sem_count(semid));
        }
        if(*rd_buf_p == *wr_buf_p)
        {
            if(mark == 0)
            {
                int vcout = get_sem_count(semid);
                zprintf1("read err sem cout %d!!!\n", vcout);
                zprintf1("read ==err rd %d num %d!!!\n", *rd_buf_p, count);
            }
            mark = 1;
            this->unlock_qtshare();
            return -1;
        }
        if(bufsize == 1)
            mid = 0;
        this->noblock_get_data(mid, val);

        if(bufsize == 1)
        {
            *wr_buf_p = 0;
        }
        else
        {
            mid++;
            mid %= bufsize;

            *rd_buf_p = mid;
        }
        //   count = 177;
        if(i < 65530)
            count--;
        *count_p = count;
    }
    i--;
    this->unlock_qtshare();
    return i;
}
template < class T >
int Sem_Qt_Data< T >::wait_thread_sem(void)
{
    if(sem_p(semid) == 0)
        return 0;
    else
        return -1;
}

// qt共享内存加linux信号 继承线程类
template < class T, class FAT >
class Sem_QtPth_Data : public Sem_Qt_Data< T >, public Call_B_T< T, FAT >
{
public:
    ~Sem_QtPth_Data()
    {
        zprintf3("destory Sem_QtPth_Data!\n");
    }

    void run();
};

template < class T, class FAT >
void Sem_QtPth_Data< T, FAT >::run(void)
{
    zprintf3("qt sem run!\n\n");
    //    int res;
    //   res = pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,   NULL);   //设置立即取消
    //   if (res != 0)
    //   {
    //       perror("Thread pthread_setcancelstate failed");
    //       exit(EXIT_FAILURE);
    //   }
    uint32_t valmak = 0;
    int      err    = 0;
    while(1)
    {
        if(sem_p(this->semid, 2) == 0)
        {
            valmak++;
            zprintf1("sem cout %d num %d!\n", get_sem_count(this->semid), valmak);
            T val;
            while(1)
            {
                err = this->read_send_data(val);
                zprintf1("sem cout %d num %d error %d????????????????????????????????????????????!\n", get_sem_count(this->semid), valmak, err);

                if(err >= 0)
                {
                    if(this->z_callbak != NULL)    //执行操作
                    {
                        this->z_callbak(this->father, val);
                    }
                    if(err == 0)
                        break;
                }
                else
                    break;
            }
        }
        else
        {
            pthread_testcancel();
        }
    }
    pthread_exit(NULL);
}

#endif /*__SEMSHARE_H__*/
