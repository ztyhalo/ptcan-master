﻿#include <cstdlib>
#include "e_poll.h"
#include "timers.h"
#include <iostream>
#include <typeinfo>
using namespace std;
 

void linuxDly(int s, int ms)
{
    if(s <0 || ms < 0) return;

    struct timeval tim;
    tim.tv_sec = s;
    tim.tv_usec = ms*1000;
    select(0,NULL,NULL,NULL, &tim);
    // zprintf1("select over!\n");
}

void linuxDly(int ms)
{
    if(ms < 0) return;
    linuxDly(0,ms);
}

