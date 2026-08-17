#ifndef __DETECTOREVENT_H__
#define __DETECTOREVENT_H__

#include <vector>

#include <Fragment.h>
#include <TObject.h>

class DetectorEvent {
  public:
    DetectorEvent() { }
    DetectorEvent(std::vector<Fragment> fragments, long timestampNs);
    DetectorEvent(const DetectorEvent&) = default;
    DetectorEvent(DetectorEvent&&) noexcept = default;
    DetectorEvent& operator=(const DetectorEvent&) = default;
    DetectorEvent& operator=(DetectorEvent&&) noexcept = default;
    virtual ~DetectorEvent() { }

    void Print(Option_t *opt="") const;

    const std::vector<Fragment>& Fragments() const { return fFragments; }
    long TimestampNs() const { return fTimestampNs; }

  private:
    std::vector<Fragment> fFragments;
    long fTimestampNs{-1};

  ClassDef(DetectorEvent,2);
};

#endif
