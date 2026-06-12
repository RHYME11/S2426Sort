#ifndef __DETECTORPROCESS_H__
#define __DETECTORPROCESS_H__


#include <vector>
#include <queue>
#include <mutex>
#include <thread>

#include <EventProcess.h>

#include <Fragment.h>

class DetectorProcess {
  public:
    virtual ~DetectorProcess(); 
    static DetectorProcess *Get();

    void loop(); // monitor the queue and decide when to do useful things.

    void Stop() { fStop = true; }

    bool     Running() const { return !fStop.load(); }
    uint32_t Pushed() const { return fPushed.load(); }
    uint32_t Popped() const { return fPopped.load(); }

  private:
    DetectorProcess();

  private:
    static DetectorProcess *fDetectorProcess;

    std::mutex fMutex;

    std::atomic<uint32_t> fPushed{0};
    std::atomic<uint32_t> fPopped{0};

    std::atomic_bool fStop{false};
    std::thread fWorker;
};




#endif
