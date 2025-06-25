#include "Time.h"
/**
 * @brief 开始计时，记录起始时间
 */
void Time::reset(){_begin=0;_clock=0;}
void Time::begin(){_begin = time(nullptr);_clock=0;}
void Time::stop(){if(_begin==0) return;_clock+=time(nullptr)-_begin;_begin=0;}
void Time::start(){_begin=time(nullptr);}
time_t Time::get() const{return _begin==0?_clock:time(nullptr)-_begin+_clock;}