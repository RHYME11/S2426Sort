#ifndef __TIGRESS_H__
#define __TIGRESS_H__

#include <vector>

#include <Event.h>
#include <Rtypes.h>
#include <TigressHit.h>

class Tigress {
  public:
    Tigress();
    virtual ~Tigress();

    // ============== Set ==============
    // purpose: Build TIGRESS hits from event core, segment, and BGO fragments.
    // inputs: source event
    // outputs: none
    void Set(const Event& event);

    // ============== Clear ==============
    // purpose: Clear stored TIGRESS hits.
    // inputs: none
    // outputs: none
    void Clear();

    // ============== Print ==============
    // purpose: Print a TIGRESS summary and all stored hits.
    // inputs: none
    // outputs: none
    void Print() const;

    size_t Size() const { return fHits.size(); }
    const std::vector<TigressHit>& Hits() const { return fHits; }

  private:
    std::vector<TigressHit> fHits;

  ClassDef(Tigress, 1);
};

#endif
