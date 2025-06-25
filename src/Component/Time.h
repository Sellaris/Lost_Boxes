#include <ctime>
#ifndef DAB_TIME_H
#define DAB_TIME_H// DAB_TIME_H
class Time{
  private: time_t _begin,_clock;
  public: void begin(),start(),stop(),reset();time_t get() const;
};
#endif 
