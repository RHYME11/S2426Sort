#ifndef __DETECTORPROCESS_H__
#define __DETECTORPROCESS_H__


#include <atomic>
#include <cstdint>
#include <thread>

#include <EventProcess.h>

class DetectorProcess {
  public:
    virtual ~DetectorProcess(); 
    static DetectorProcess *Get();

    void loop(); // monitor the queue and decide when to do useful things.

    void Stop() { fStop = true; }

    bool     Running() const { return !fStop.load(); }
    uint32_t Pushed() const { return fPushed.load(); }

  private:
    DetectorProcess();

  private:
    static DetectorProcess *fDetectorProcess;

    std::atomic<uint32_t> fPushed{0};

    std::atomic_bool fStop{false};
    std::thread fWorker;
};




#endif
