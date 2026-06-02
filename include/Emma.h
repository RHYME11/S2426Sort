#ifndef __EMMA_H__
#define __EMMA_H__

#include <map>
#include <vector>

#include <Event.h>
#include <Fragment.h>
#include <Rtypes.h>

class Emma {
  public:
    Emma();
    virtual ~Emma();

    // ============== Set ==============
    // purpose: Copy EMMA fragments from an event.
    // inputs: source event
    // outputs: none
    void Set(const Event& event);

    // ============== Clear ==============
    // purpose: Clear stored EMMA fragments.
    // inputs: none
    // outputs: none
    void Clear();

    // ============== Print ==============
    // purpose: Print an EMMA summary and PGAC X position.
    // inputs: none
    // outputs: none
    void Print() const;

    // ============== PGACX ==============
    // purpose: Return stored PGAC X position.
    // inputs: none
    // outputs: PGAC X position, or -1 when inputs are incomplete
    double PGACX() const { return fPGACX; }

    // ============== CalculatePGACX ==============
    // purpose: Calculate PGAC X position from stored left, right, and anode charges.
    // inputs: none
    // outputs: PGAC X position, or -1 when inputs are incomplete
    double CalculatePGACX() const;
    
    void SetPGACX(double val) { fPGACX = val; }

    const std::vector<Fragment>& Si() const { return fSi; }
    const std::map<int, std::vector<Fragment> >& ICs() const { return fICs; }
    const std::vector<Fragment>& Anodes() const { return fAnodes; }
    const std::vector<Fragment>& Left() const { return fLeft; }
    const std::vector<Fragment>& Right() const { return fRight; }
    const std::vector<Fragment>& PGACTop() const { return fPGACTop; }
    const std::vector<Fragment>& PGACBot() const { return fPGACBot; }

  private:
    std::vector<Fragment> fSi;
    std::map<int, std::vector<Fragment> > fICs;
    std::vector<Fragment> fAnodes;
    std::vector<Fragment> fLeft;
    std::vector<Fragment> fRight;
    std::vector<Fragment> fPGACTop;
    std::vector<Fragment> fPGACBot;
    double fPGACX{-1};

  ClassDef(Emma, 2);
};

#endif
