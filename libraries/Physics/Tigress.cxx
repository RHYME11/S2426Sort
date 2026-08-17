
#include <Tigress.h>

#include <DetectorEvent.h>
#include <Fragment.h>
#include <TigressGeometry.h>

#include <TMath.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <limits>

ClassImp(TigressChannelHit)
ClassImp(TigressHit)
ClassImp(Tigress)

namespace {

constexpr int ARRAY_COUNT = 64;

// ============== ParseCrystalNumber ==============
// Purpose: Convert a TIGRESS crystal color in a channel name to 0--3.
// Inputs: TIGRESS channel name.
// Outputs: Crystal number, or -1 for an invalid name.
int ParseCrystalNumber(const std::string& name) {
  if(name.size() <= 5) {
    return -1;
  }

  switch(name[5]) {
    case 'B': return 0;
    case 'G': return 1;
    case 'R': return 2;
    case 'W': return 3;
    default: return -1;
  }
}

// ============== ParseDetectorNumber ==============
// Purpose: Extract the detector number from a TIGRESS Ge or shield name.
// Inputs: TIG or TIS channel name.
// Outputs: Detector number 5--16, or -1 for an invalid name.
int ParseDetectorNumber(const std::string& name) {
  const bool validPrefix = name.compare(0, 3, "TIG") == 0 ||
                           name.compare(0, 3, "TIS") == 0;
  if(name.size() < 9 || !validPrefix ||
     name[3] < '0' || name[3] > '9' ||
     name[4] < '0' || name[4] > '9') {
    return -1;
  }

  const int detector = (name[3] - '0') * 10 + (name[4] - '0');
  if(detector < 5 || detector > 16) {
    return -1;
  }
  return detector;
}

// ============== CalculateArrayNumber ==============
// Purpose: Calculate the zero-based TIGRESS array number used by S2426.
// Inputs: TIGRESS channel name.
// Outputs: Array number 16--63, or -1 for an invalid name.
int CalculateArrayNumber(const std::string& name) {
  const int detector = ParseDetectorNumber(name);
  const int crystal = ParseCrystalNumber(name);
  if(detector < 0 || crystal < 0) {
    return -1;
  }
  return (detector - 1) * 4 + crystal;
}

// ============== ParseSegmentNumber ==============
// Purpose: Extract a TIGRESS core or segment number from a channel name.
// Inputs: TIGRESS channel name.
// Outputs: Segment number 0--8, or -1 for an invalid name.
int ParseSegmentNumber(const std::string& name) {
  if(name.size() < 9 ||
     name[7] < '0' || name[7] > '9' ||
     name[8] < '0' || name[8] > '9') {
    return -1;
  }

  const int segment = (name[7] - '0') * 10 + (name[8] - '0');
  if(segment < 0 || segment > 8) {
    return -1;
  }
  return segment;
}

// ============== PositionFromName ==============
// Purpose: Read the forward TIGRESS position for one named core or segment.
// Inputs: TIGRESS channel name.
// Outputs: Position vector, or (0,0,0) for an invalid name.
TVector3 PositionFromName(const std::string& name) {
  const int detector = ParseDetectorNumber(name);
  const int crystal = ParseCrystalNumber(name);
  const int segment = ParseSegmentNumber(name);
  if(detector < 0 || crystal < 0 || segment < 0) {
    return TVector3();
  }

  const double (*positions)[17][9][3] = nullptr;
  switch(crystal) {
    case 0: positions = &TigressGeometry::GeBluePosition; break;
    case 1: positions = &TigressGeometry::GeGreenPosition; break;
    case 2: positions = &TigressGeometry::GeRedPosition; break;
    case 3: positions = &TigressGeometry::GeWhitePosition; break;
    default: return TVector3();
  }

  return TVector3((*positions)[detector][segment][0],
                  (*positions)[detector][segment][1],
                  (*positions)[detector][segment][2]);
}

// ============== SelectCore ==============
// Purpose: Select the core that owns one segment using the S2426 time rule.
// Inputs: Segment timestamp, candidate hit indices, and built core hits.
// Outputs: Selected hit index, or max size_t when no candidate exists.
size_t SelectCore(long segmentTimestampNs,
                  const std::vector<size_t>& candidates,
                  const std::vector<TigressHit>& hits) {
  const size_t invalid = std::numeric_limits<size_t>::max();
  size_t futureIndex = invalid;
  size_t pastIndex = invalid;
  long long futureDistance = std::numeric_limits<long long>::max();
  long long pastDistance = std::numeric_limits<long long>::max();

  for(const size_t index : candidates) {
    const long long dt = static_cast<long long>(segmentTimestampNs) -
                         static_cast<long long>(hits[index].TimestampNs());
    if(dt == 0) {
      return index;
    }
    if(dt < 0 && -dt < futureDistance) {
      futureDistance = -dt;
      futureIndex = index;
    } else if(dt > 0 && dt < pastDistance) {
      pastDistance = dt;
      pastIndex = index;
    }
  }

  return futureIndex != invalid ? futureIndex : pastIndex;
}

}

// ============== TigressChannelHit ==============
// Purpose: Copy persistent channel quantities from one Fragment.
// Inputs: Source Fragment.
// Outputs: Constructed channel hit.
TigressChannelHit::TigressChannelHit(const Fragment& frag)
  : fEnergy(frag.Energy()),
    fCharge(frag.Charge()),
    fAddress(frag.Address()),
    fNumber(frag.Number()),
    fName(frag.Name()),
    fTimestamp(frag.Timestamp()),
    fTimestampNs(frag.TimestampNs()),
    fTime(frag.Time()),
    fCFD(frag.Cfd()) { }

// ============== Clear ==============
// Purpose: Reset all stored channel quantities.
// Inputs: ROOT option string, unused.
// Outputs: Cleared channel hit.
void TigressChannelHit::Clear(Option_t *opt) {
  (void)opt;
  fEnergy = -1;
  fCharge = -1;
  fAddress = -1;
  fNumber = -1;
  fName.clear();
  fTimestamp = -1;
  fTimestampNs = -1;
  fTime = -1;
  fCFD = -1;
}

// ============== Print ==============
// Purpose: Print all stored channel quantities.
// Inputs: ROOT option string, unused.
// Outputs: Values written to stdout.
void TigressChannelHit::Print(Option_t *opt) const {
  (void)opt;
  printf("TIGRESS channel hit\n");
  printf("\tname:        %s\n", fName.c_str());
  printf("\taddress:     0x%08x\n", fAddress);
  printf("\tnumber:      %i\n", fNumber);
  printf("\ttimestamp:   %ld\n", fTimestamp);
  printf("\ttimestampNs: %ld\n", fTimestampNs);
  printf("\tcfd:         %i\n", fCFD);
  printf("\ttime:        %.3f\n", fTime);
  printf("\tcharge:      %.3f\n", fCharge);
  printf("\tenergy:      %.3f\n", fEnergy);
}

// ============== TigressHit ==============
// Purpose: Build the core portion of one TIGRESS physics hit.
// Inputs: Selected CoreA or CoreB Fragment.
// Outputs: Constructed core-centered hit.
TigressHit::TigressHit(const Fragment& frag)
  : TigressChannelHit(frag) {
  fDetectorNumber = ParseDetectorNumber(Name());
  fArrayNumber = CalculateArrayNumber(Name());
}

// ============== Clear ==============
// Purpose: Reset core, segment, suppression, and position information.
// Inputs: ROOT option string.
// Outputs: Cleared TIGRESS hit.
void TigressHit::Clear(Option_t *opt) {
  TigressChannelHit::Clear(opt);
  fDetectorNumber = -1;
  fArrayNumber = -1;
  fBGOFire = false;
  fPosition.SetXYZ(0., 0., 0.);
  fSegments.clear();
}

// ============== Print ==============
// Purpose: Print the core, geometry, suppression, and attached segments.
// Inputs: ROOT option string.
// Outputs: Values written to stdout.
void TigressHit::Print(Option_t *opt) const {
  TigressChannelHit::Print(opt);
  printf("\tdetector:    %i\n", fDetectorNumber);
  printf("\tarray:       %i\n", fArrayNumber);
  printf("\tBGO fired:   %s\n", fBGOFire ? "true" : "false");
  printf("\tposition:    (%.3f, %.3f, %.3f)\n",
         fPosition.X(), fPosition.Y(), fPosition.Z());
  printf("\tsegments:    %lu\n",
         static_cast<unsigned long>(fSegments.size()));
  for(const auto& segment : fSegments) {
    segment.Print(opt);
  }
}

// ============== Doppler ==============
// Purpose: Apply a Doppler correction for a beam along positive z.
// Inputs: Recoil beta with absolute value below one.
// Outputs: Corrected energy, or raw energy when position is invalid.
double TigressHit::Doppler(double beta) const {
  if(fPosition.Mag2() <= 0.) {
    return Energy();
  }

  const double gamma = 1.0 / std::sqrt(1.0 - beta * beta);
  return Energy() * gamma *
         (1.0 - beta * TMath::Cos(fPosition.Theta()));
}

// ============== UpdatePosition ==============
// Purpose: Store the highest-energy segment position or the core position.
// Inputs: Attached segments and the core name.
// Outputs: Updated position vector.
void TigressHit::UpdatePosition() {
  if(fSegments.empty()) {
    fPosition = PositionFromName(Name());
    return;
  }

  const auto highestEnergy = std::max_element(
    fSegments.begin(), fSegments.end(),
    [](const TigressChannelHit& lhs, const TigressChannelHit& rhs) {
      return lhs.Energy() < rhs.Energy();
    });
  fPosition = PositionFromName(highestEnergy->Name());
}

// ============== Clear ==============
// Purpose: Clear all built TIGRESS cores and BGO hits.
// Inputs: ROOT option string, unused.
// Outputs: Cleared TIGRESS object.
void Tigress::Clear(Option_t *opt) {
  (void)opt;
  fHits.clear();
  fBGOHits.clear();
}

// ============== Print ==============
// Purpose: Print all TIGRESS core and BGO hit information.
// Inputs: ROOT option string.
// Outputs: Values written to stdout.
void Tigress::Print(Option_t *opt) const {
  printf("Tigress hits: %lu, BGO hits: %lu\n",
         static_cast<unsigned long>(fHits.size()),
         static_cast<unsigned long>(fBGOHits.size()));
  for(const auto& hit : fHits) {
    hit.Print(opt);
  }
  for(const auto& bgo : fBGOHits) {
    bgo.Print(opt);
  }
}

// ============== BuildHits ==============
// Purpose: Build CoreA-preferred TIGRESS hits and assign each segment once.
// Inputs: One DetectorEvent in fragment time order.
// Outputs: Built core-centered hits and event BGO hits.
void Tigress::BuildHits(const DetectorEvent& event) {
  Clear();

  std::array<bool, ARRAY_COUNT> hasCoreA{};
  std::array<std::vector<const Fragment*>, ARRAY_COUNT> segments;

  for(const auto& frag : event.Fragments()) {
    if(frag.DetType() == 3) {
      fBGOHits.emplace_back(frag);
      continue;
    }
    if(frag.DetType() != 0 && frag.DetType() != 2) {
      continue;
    }

    const int arrayNumber = CalculateArrayNumber(frag.Name());
    if(arrayNumber < 0) {
      continue;
    }
    if(frag.DetType() == 0) {
      hasCoreA[arrayNumber] = true;
    } else {
      segments[arrayNumber].push_back(&frag);
    }
  }

  std::array<std::vector<size_t>, ARRAY_COUNT> coresByArray;
  for(const auto& frag : event.Fragments()) {
    if(frag.DetType() != 0 && frag.DetType() != 1) {
      continue;
    }

    const int arrayNumber = CalculateArrayNumber(frag.Name());
    if(arrayNumber < 0) {
      continue;
    }

    const bool selectedCoreA = frag.DetType() == 0;
    const bool selectedCoreB = frag.DetType() == 1 &&
                               !hasCoreA[arrayNumber];
    if(!selectedCoreA && !selectedCoreB) {
      continue;
    }

    coresByArray[arrayNumber].push_back(fHits.size());
    fHits.emplace_back(frag);
  }

  const size_t invalid = std::numeric_limits<size_t>::max();
  for(int arrayNumber = 0; arrayNumber < ARRAY_COUNT; ++arrayNumber) {
    for(const Fragment* segment : segments[arrayNumber]) {
      const size_t coreIndex = SelectCore(segment->TimestampNs(),
                                          coresByArray[arrayNumber],
                                          fHits);
      if(coreIndex != invalid) {
        fHits[coreIndex].fSegments.emplace_back(*segment);
      }
    }
  }

  for(auto& hit : fHits) {
    hit.UpdatePosition();
  }
}

// ============== UpdateBGOFire ==============
// Purpose: Recalculate current-GRSISort BGO suppression for every core.
// Inputs: Exclusive minimum and maximum core-minus-BGO time limits in ns.
// Outputs: Updated BGOFire flags.
void Tigress::UpdateBGOFire(const std::array<double, 2>& timeWindow) {
  for(auto& hit : fHits) {
    hit.fBGOFire = false;
  }

  if(timeWindow[0] >= timeWindow[1]) {
    return;
  }

  for(auto& hit : fHits) {
    for(const auto& bgo : fBGOHits) {
      const double dt = hit.Time() - bgo.Time();
      if(hit.DetectorNumber() == ParseDetectorNumber(bgo.Name()) &&
         dt > timeWindow[0] && dt < timeWindow[1] &&
         bgo.Energy() > SUPPRESSION_ENERGY) {
        hit.fBGOFire = true;
        break;
      }
    }
  }
}
