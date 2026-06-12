#ifndef __EVENTBUILDER_H__
#define __EVENTBUILDER_H__

#include <vector>
#include <map>
#include <mutex>
#include <thread>

#include <atomic>

#include <Fragment.h>


struct CompareFragmentPtrs {
    bool operator()(const std::unique_ptr<Fragment>& a, const std::unique_ptr<Fragment>& b) const {
      if(!a.get() && !b.get()) return false;
      if (!a.get()) return true;  
      if (!b.get()) return false; 
      return a.get()->TimestampNs() > b.get()->TimestampNs();
    }
};




class EventBuilder {
  public:
    virtual ~EventBuilder(); 
    static EventBuilder *Get();

    void push(std::unique_ptr<Fragment> frag);
    bool pop(std::vector<std::unique_ptr<Fragment> > &Builtfrags);  
    void loop(); // monitor the queue and decide when to do useful things.

    
    void Stop()  { fStop = true; }
    void Flush() { fFlushing = true; }
    bool Flushing() const { return fFlushing.load(); }


    bool     Running() const { return !fStop.load(); }
    uint32_t Size()    const { std::lock_guard lk(fMutex); return fQueue.size(); }
    uint32_t Pushed()  const {  return fPushed.load(); }
    uint32_t Popped()  const {  return fPopped.load(); }


  private:
    EventBuilder();

  private:
    static EventBuilder *fEventBuilder;

    long fLastTimestampNs{-1};
    long fLatestTimestampNsSeen{0};
    std::atomic_bool fFlushing{false};

    static constexpr long BUILD_WINDOW_NS  = 10000;
    static constexpr long REORDER_SLACK_NS = 500000000;

    mutable std::mutex fMutex;
    std::multimap<long, std::unique_ptr<Fragment>> fQueue;


    std::atomic<uint32_t> fPushed{0};
    std::atomic<uint32_t> fPopped{0};

    std::atomic_bool fStop{false};
    std::thread fWorker;
};


#endif
