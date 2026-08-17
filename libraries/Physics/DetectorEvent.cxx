#include <DetectorEvent.h>

#include <cstdio>
#include <utility>

ClassImp(DetectorEvent)

// ============== DetectorEvent ==============
// Purpose: Store one built fragment group and its EMMA anode reference time.
// Inputs: Time-ordered fragments and reference timestamp in ns.
// Outputs: Initialized detector event.
DetectorEvent::DetectorEvent(std::vector<Fragment> fragments, long timestampNs)
  : fFragments(std::move(fragments)), fTimestampNs(timestampNs) { }

// ============== Print ==============
// Purpose: Print the reference timestamp and fragment count.
// Inputs: ROOT option string.
// Outputs: Detector event summary written to stdout.
void DetectorEvent::Print(Option_t *opt) const {
  printf("DetectorEvent\n");
  printf("\tTimestampNs: %ld\n", fTimestampNs);
  printf("\tFragments:   %lu\n", fFragments.size());
}
