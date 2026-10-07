#ifndef S2426_TIP_GEOMETRY_H
#define S2426_TIP_GEOMETRY_H

#include <array>
#include <TVector3.h>

// Source: GRSISort/GRSIData/libraries/TGRSIAnalysis/TTip/TTip.cxx.
// Coordinates are in mm, with the target at (0, 0, 0).
// All 128 coordinates retain their original values and ordering.
// Array indices are 0--127; physical TipChannel numbers are 1--128.
// Index 0 is TIP detector 1, not a placeholder.
// TipChannel is parsed from the three digits in a name such as TPC039N00X.
// The calibration ChannelNumber (e.g. 732 for TPC039N00X) is not this index.
// Match GRSISort's hit lookup: GetPosition(hit.TipChannel() - 1).

namespace TipGeometry {

  inline const std::array<TVector3, 128> fPositionVectors = {
    TVector3(12.8039, 0.0000, 91.1047),
    TVector3(0.0000, 12.8039, 91.1047),
    TVector3(-12.8039, 0.0000, 91.1047),
    TVector3(0.0000, -12.8039, 91.1047),
    TVector3(17.2004, 0.0000, 54.5526),
    TVector3(8.60019, 14.896, 54.5526),
    TVector3(-8.60019, 14.896, 54.5526),
    TVector3(-17.2004, 0.0000, 54.5526),
    TVector3(-8.60019, -14.896, 54.5526),
    TVector3(8.60019, -14.896, 54.5526),
    TVector3(31.1534, 0.0000, 47.972),
    TVector3(26.9796, 15.5767, 47.972),
    TVector3(15.5767, 26.9796, 47.972),
    TVector3(0.0000, 31.1534, 47.972),
    TVector3(-15.5767, 26.9796, 47.972),
    TVector3(-26.9796, 15.5767, 47.972),
    TVector3(-31.1534, 0.0000, 47.972),
    TVector3(-26.9796, -15.5767, 47.972),
    TVector3(-15.5767, -26.9796, 47.972),
    TVector3(0.0000, -31.1534, 47.972),
    TVector3(15.5767, -26.9796, 47.972),
    TVector3(26.9796, -15.5767, 47.972),
    TVector3(42.8403, 0.0000, 37.9019),
    TVector3(39.5792, 16.3943, 37.9019),
    TVector3(30.2926, 30.2926, 37.9019),
    TVector3(16.3943, 39.5792, 37.9019),
    TVector3(0.0000, 42.8403, 37.9019),
    TVector3(-16.3943, 39.5792, 37.9019),
    TVector3(-30.2926, 30.2926, 37.9019),
    TVector3(-39.5792, 16.3943, 37.9019),
    TVector3(-42.8403, 0.0000, 37.9019),
    TVector3(-39.5792, -16.3943, 37.9019),
    TVector3(-30.2926, -30.2926, 37.9019),
    TVector3(-16.3943, -39.5792, 37.9019),
    TVector3(0.0000, -42.8403, 37.9019),
    TVector3(16.3943, -39.5792, 37.9019),
    TVector3(30.2926, -30.2926, 37.9019),
    TVector3(39.5792, -16.3943, 37.9019),
    TVector3(51.411, 0.0000, 25.0748),
    TVector3(48.8948, 15.8869, 25.0748),
    TVector3(41.5924, 30.2186, 25.0748),
    TVector3(30.2186, 41.5924, 25.0748),
    TVector3(15.8869, 48.8948, 25.0748),
    TVector3(0.0000, 51.411, 25.0748),
    TVector3(-15.8869, 48.8948, 25.0748),
    TVector3(-30.2186, 41.5924, 25.0748),
    TVector3(-41.5924, 30.2186, 25.0748),
    TVector3(-48.8948, 15.8869, 25.0748),
    TVector3(-51.411, 0.0000, 25.0748),
    TVector3(-48.8948, -15.8869, 25.0748),
    TVector3(-41.5924, -30.2186, 25.0748),
    TVector3(-30.2186, -41.5924, 25.0748),
    TVector3(-15.8869, -48.8948, 25.0748),
    TVector3(0.0000, -51.411, 25.0748),
    TVector3(15.8869, -48.8948, 25.0748),
    TVector3(30.2186, -41.5924, 25.0748),
    TVector3(41.5924, -30.2186, 25.0748),
    TVector3(48.8948, -15.8869, 25.0748),
    TVector3(56.2422, 0.0000, 10.4239),
    TVector3(52.8504, 19.236, 10.4239),
    TVector3(43.084, 36.1518, 10.4239),
    TVector3(28.1211, 48.7072, 10.4239),
    TVector3(9.76635, 55.3877, 10.4239),
    TVector3(-9.76635, 55.3877, 10.4239),
    TVector3(-28.1211, 48.7072, 10.4239),
    TVector3(-43.084, 36.1518, 10.4239),
    TVector3(-52.8504, 19.236, 10.4239),
    TVector3(-56.2422, 0.0000, 10.4239),
    TVector3(-52.8504, -19.236, 10.4239),
    TVector3(-43.084, -36.1518, 10.4239),
    TVector3(-28.1211, -48.7072, 10.4239),
    TVector3(-9.76635, -55.3877, 10.4239),
    TVector3(9.76635, -55.3877, 10.4239),
    TVector3(28.1211, -48.7072, 10.4239),
    TVector3(43.084, -36.1518, 10.4239),
    TVector3(52.8504, -19.236, 10.4239),
    TVector3(56.9823, 0.0000, -4.98531),
    TVector3(53.5459, 19.4891, -4.98531),
    TVector3(43.651, 36.6275, -4.98531),
    TVector3(28.4912, 49.3482, -4.98531),
    TVector3(9.89488, 56.1166, -4.98531),
    TVector3(-9.89488, 56.1166, -4.98531),
    TVector3(-28.4912, 49.3482, -4.98531),
    TVector3(-43.651, 36.6275, -4.98531),
    TVector3(-53.5459, 19.4891, -4.98531),
    TVector3(-56.9823, 0.0000, -4.98531),
    TVector3(-53.5459, -19.4891, -4.98531),
    TVector3(-43.651, -36.6275, -4.98531),
    TVector3(-28.4912, -49.3482, -4.98531),
    TVector3(-9.89488, -56.1166, -4.98531),
    TVector3(9.89488, -56.1166, -4.98531),
    TVector3(28.4912, -49.3482, -4.98531),
    TVector3(43.651, -36.6275, -4.98531),
    TVector3(53.5459, -19.4891, -4.98531),
    TVector3(53.0722, 0.0000, -21.3349),
    TVector3(47.8164, 23.0272, -21.3349),
    TVector3(33.09, 41.4935, -21.3349),
    TVector3(11.8097, 51.7416, -21.3349),
    TVector3(-11.8097, 51.7416, -21.3349),
    TVector3(-33.09, 41.4935, -21.3349),
    TVector3(-47.8164, 23.0272, -21.3349),
    TVector3(-53.0722, 0.0000, -21.3349),
    TVector3(-47.8164, -23.0272, -21.3349),
    TVector3(-33.09, -41.4935, -21.3349),
    TVector3(-11.8097, -51.7416, -21.3349),
    TVector3(11.8097, -51.7416, -21.3349),
    TVector3(33.09, -41.4935, -21.3349),
    TVector3(47.8164, -23.0272, -21.3349),
    TVector3(43.6891, 0.0000, -36.9202),
    TVector3(37.8359, 21.8446, -36.9202),
    TVector3(21.8446, 37.8359, -36.9202),
    TVector3(0.0000, 43.6891, -36.9202),
    TVector3(-21.8446, 37.8359, -36.9202),
    TVector3(-37.8359, 21.8446, -36.9202),
    TVector3(-43.6891, 0.0000, -36.9202),
    TVector3(-37.8359, -21.8446, -36.9202),
    TVector3(-21.8446, -37.8359, -36.9202),
    TVector3(0.0000, -43.6891, -36.9202),
    TVector3(21.8446, -37.8359, -36.9202),
    TVector3(37.8359, -21.8446, -36.9202),
    TVector3(29.8869, 0.0000, -48.771),
    TVector3(21.1332, 21.1332, -48.771),
    TVector3(0.0000, 29.8869, -48.771),
    TVector3(-21.1332, 21.1332, -48.771),
    TVector3(-29.8869, 0.0000, -48.771),
    TVector3(-21.1332, -21.1332, -48.771),
    TVector3(0.0000, -29.8869, -48.771),
    TVector3(21.1332, -21.1332, -48.771),
  };

  // ============== GetPosition ==============
  // Purpose: Read one TIP position using GRSISort's zero-based array index.
  // Inputs: Detector index 0--127, equal to physical TipChannel minus one.
  // Outputs: Position in mm; (0, 0, 1) for an invalid index.
  inline TVector3 GetPosition(int detNbr) {
    if (detNbr < 0 || detNbr > 127) {
      return {0, 0, 1};
    }
    return fPositionVectors[static_cast<std::size_t>(detNbr)];
  }

} // namespace TipGeometry

#endif
