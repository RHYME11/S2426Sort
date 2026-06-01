#ifndef __EVENT_H__
#define __EVENT_H__

#include <vector>
#include <Fragment.h>

class Event {

  public:
    Event();
  
  virtual ~Event();
  

  

  private:
    std::vector<Fragment> frags;

};
