#ifndef __EVENT_H__
#define __EVENT_H__

#include <cstddef>
#include <map>
#include <vector>

#include <Fragment.h>
#include <Rtypes.h>

class Event {
  public:
    Event();
    explicit Event(const std::vector<Fragment>& fragments);
    virtual ~Event();

    // ============== Set ==============
    // purpose: Copy event fragments and rebuild detector-category indices.
    // inputs: vector of fragments
    // outputs: none
    void Set(const std::vector<Fragment>& fragments);

    // ============== Copy ==============
    // purpose: Copy every stored field from an input Event into a new Event object.
    // inputs: source Event
    // outputs: copied Event
    static Event Copy(const Event& event);

    // ============== Print ==============
    // purpose: Print a concise event summary.
    // inputs: none
    // outputs: none
    void Print() const;

    // ============== Clear ==============
    // purpose: Clear stored fragments, category indices, and event status.
    // inputs: none
    // outputs: none
    void Clear();

    bool IsGood() const { return fGood; }
    size_t Size() const { return fFragments.size(); }

    const std::vector<Fragment>& Fragments() const { return fFragments; }
    const Fragment& FragmentAt(size_t index) const { return fFragments.at(index); }
    
    const std::vector<size_t>& Cores() const { return fCores; }
    const std::vector<size_t>& Segments() const { return fSegments; }
    const std::vector<size_t>& Bgos() const { return fBgos; }

    const std::vector<size_t>& Si() const { return fSi; }
    const std::map<int, std::vector<size_t> >& ICs() const { return fICs; }
    const std::vector<size_t>& Anodes() const { return fAnodes; }
    const std::vector<size_t>& Left() const { return fLeft; }
    const std::vector<size_t>& Right() const { return fRight; }
    const std::vector<size_t>& Top() const { return fTop; }
    const std::vector<size_t>& Bot() const { return fBot; }

  private:
    std::vector<Fragment> fFragments;

    std::vector<size_t> fCores;
    std::vector<size_t> fSegments;
    std::vector<size_t> fBgos;

    std::vector<size_t> fSi;
    std::map<int, std::vector<size_t> > fICs;

    std::vector<size_t> fAnodes;
    std::vector<size_t> fLeft;
    std::vector<size_t> fRight;
    std::vector<size_t> fTop;
    std::vector<size_t> fBot;

    bool fGood{false};

  ClassDef(Event,2);
};

#endif
