#ifndef __EVENTPROCESS_H__
#define __EVENTPROCESS_H__


#include <vector>
#include <queue>
#include <mutex>
#include <thread>

#include <Fragment.h>
#include <Tigress.h>
#include <Emma.h>

struct DetectorEvent {
  long timestamp{0};

  std::unique_ptr<Tigress> tigress;
  std::unique_ptr<Emma>    emma;
  //std::unique_Ptr<Tip>     tip;

  bool Empty() const {
    return !tigress; //&& !emma && !tip;
  }
};

class EventProcess {
  public:
    virtual ~EventProcess(); 
    static EventProcess *Get();

    void push(DetectorEvent event);
    bool pop(DetectorEvent &event);  
    //bool peek(Fragment*& out);

    void loop(); // monitor the queue and decide when to do useful things.

    void Stop() { fStop = true; }

    bool     Running() const { return !fStop.load(); }
    uint32_t Pushed() const { return fPushed.load(); }
    uint32_t Popped() const { return fPopped.load(); }

    uint32_t Size()    const { std::lock_guard lk(fMutex); return fQueue.size(); }

  private:
    EventProcess();

  private:
    static EventProcess *fEventProcess;

    mutable std::mutex fMutex;
    std::queue<DetectorEvent> fQueue;

    std::atomic<uint32_t> fPushed{0};
    std::atomic<uint32_t> fPopped{0};

    std::atomic_bool fStop{false};
    std::thread fWorker;
};




#endif
