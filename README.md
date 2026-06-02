# S2426Sort Event Building Notes

## Contents

- [Event Building Rule](#event-building-rule)
- [Good Event Tagging](#good-event-tagging)
- [Fragment Class Contents](#fragment-class-contents)
- [Event Class Contents](#event-class-contents)
- [Tigress and Emma Classes](#tigress-and-emma-classes)
- [Analysis Conversion](#analysis-conversion)
- [ROOT Interactive Use](#root-interactive-use)
- [Output Trees](#output-trees)

## Event Building Rule

`EventBuilder` collects unpacked `Fragment` objects in a priority queue ordered by `Fragment::TimestampNs()`. The fragment with the smallest nanosecond timestamp is always used as the first fragment of the next built event.

The current event-building rule is:

1. Push every unpacked fragment into `EventBuilder::fQueue`.
2. Do not build events while the queue contains fewer than `20,000,000` fragments, unless input reading has stopped.
3. When building starts, pop the earliest fragment from the queue.
4. Use that earliest fragment timestamp as `firstTime`.
5. Continue adding fragments from the queue while:

```text
top fragment TimestampNs() - firstTime <= 2500 ns
```

6. Stop the event when the next queued fragment is more than `2500 ns` later than `firstTime`, or when the queue is empty.
7. After MIDAS input reading is complete, continue popping events until the queue is fully drained.

This means one built event contains all queued fragments that fall within a `2.5 us` window starting from the earliest available fragment timestamp.

## Good Event Tagging

Every built event is stored in the normal event tree. The `Event` class also marks an event as good with `Event::IsGood()`.

An event is currently tagged as good only when it contains all of the following detector categories:

- At least one TIGRESS core fragment.
- At least one EMMA Si fragment.
- At least one EMMA PGAC left fragment.
- At least one EMMA PGAC right fragment.
- At least one EMMA anode fragment.
- At least one EMMA ion chamber fragment.

In code, the good-event flag is set by `Event::Set()` as:

```cpp
fGood = !fCores.empty()
     && !fSi.empty()
     && !fLeft.empty()
     && !fRight.empty()
     && !fAnodes.empty()
     && hasIC;
```

Only events with `IsGood() == true` are written to the good-event output tree.

## Fragment Class Contents

`Fragment` stores the information for one unpacked detector hit. It contains detector identity, timing, waveform status, charge and energy information, and helper methods for detector mapping.

Main stored fields:

- `fAddress`: raw channel address.
- `fDetType`: detector type code.
- `fTimestamp`: raw timestamp.
- `fTimestampUnit`: timestamp conversion factor used by `TimestampNs()`.
- `fCfd`: CFD timing value.
- `fFilterPattern`: filter pattern.
- `fPileup`: pileup flag.
- `fHasWave`: waveform-present flag.
- `fWaveSamples`: number of waveform samples.
- `fInt`: integer payload values.
- `fCharge`: charge values.
- `fEnergy`: calibrated energy values.
- `fTheta`: detector angle used by Doppler correction.

Important accessors and helpers:

- `TimestampNs()`: returns `fTimestamp * fTimestampUnit`.
- `Time()`: returns a CFD-refined time.
- `Charge()` and `Energy()`: return charge and calibrated energy.
- `Number()`, `DetNumber()`, `XtalNumber()`, and `ArryNumber()`: return mapped detector indices.
- `Name()`: returns the mapped channel name.
- `Doppler(beta)`: returns Doppler-corrected energy.

## Event Class Contents

`Event` stores one built event as a collection of copied `Fragment` objects. It also builds detector-category index lists so analysis code can quickly find the fragments belonging to each detector group.

Main stored fields:

- `fFragments`: all fragments in the built event.
- `fCores`: indices of TIGRESS core fragments.
- `fSegments`: indices of TIGRESS segment fragments.
- `fBgos`: indices of BGO fragments.
- `fSi`: indices of EMMA Si fragments.
- `fICs`: EMMA ion chamber indices grouped by IC channel.
- `fAnodes`: indices of EMMA anode fragments.
- `fLeft`: indices of EMMA PGAC left fragments.
- `fRight`: indices of EMMA PGAC right fragments.
- `fTop`: indices of EMMA PGAC top fragments.
- `fBot`: indices of EMMA PGAC bottom fragments.
- `fGood`: good-event flag.

Detector-category mapping is currently based on `Fragment::DetType()` and the low byte of `Fragment::Address()`:

- `DetType() == 0` or `1`: TIGRESS core.
- `DetType() == 2`: TIGRESS segment.
- `DetType() == 3`: BGO.
- `DetType() == 13` and channel `3`: EMMA Si.
- `DetType() == 13` and channels `16` to `19`: EMMA IC groups.
- `DetType() == 14` and channels `0` to `2`: EMMA anodes.
- `DetType() == 14` and channel `3`: EMMA PGAC left.
- `DetType() == 14` and channel `4`: EMMA PGAC right.
- `DetType() == 14` and channel `5`: EMMA PGAC top.
- `DetType() == 14` and channel `6`: EMMA PGAC bottom.

Important accessors:

- `IsGood()`: returns the good-event flag.
- `Size()`: returns the number of fragments.
- `Fragments()` and `FragmentAt(index)`: access stored fragments.
- `Cores()`, `Segments()`, `Bgos()`: access TIGRESS and BGO category indices.
- `Si()`, `ICs()`, `Anodes()`, `Left()`, `Right()`, `Top()`, `Bot()`: access EMMA category indices.

## Tigress and Emma Classes

`Fragment`, `Event`, `TigressHit`, `Tigress`, and `Emma` are data-model classes. They are built into the same `DATA` shared library and ROOT dictionary.

`TigressHit` stores copied TIGRESS `Fragment` objects that share one `Fragment::ArryNumber()`:

- `fCores`: TIGRESS core fragments.
- `fSegments`: TIGRESS segment fragments.
- `fBgos`: BGO fragments.

`Tigress` groups all TIGRESS hits in one event. `Tigress::Set(event)` builds a map keyed by `ArryNumber()` and then stores the resulting `TigressHit` objects.

`Emma` stores copied EMMA fragments:

- `fSi`: EMMA Si fragments.
- `fICs`: EMMA ion chamber fragments grouped by IC channel.
- `fAnodes`: EMMA anode fragments.
- `fLeft`: EMMA PGAC left fragments.
- `fRight`: EMMA PGAC right fragments.
- `fTop`: EMMA PGAC top fragments.
- `fBot`: EMMA PGAC bottom fragments.

`Emma::Set(event)` calculates and stores `fPGACX` from left, right, and anode charges. If multiple left or right fragments exist, the last stored charge is used. If multiple anodes exist, the smallest anode charge is used. `Emma::PGACX()` returns the stored value, `Emma::CalculatePGACX()` recalculates it from the stored fragments, and `Emma::SetPGACX(value)` can be used to override it later.

## Analysis Conversion

`analysis` converts a good-event ROOT file into detector-level analysis objects:

```text
./bin/analysis goodevent<run>_<subrun>.root
```

The output file is:

```text
Analysis<run>_<subrun>.root
```

The output tree is `AnalysisTree` with three branches:

- `fTigress`: a `Tigress` object built from the input `Event`.
- `fEmma`: an `Emma` object built from the input `Event`.
- `fEvent`: an `Event` object containing input fragments that were not copied into `Tigress` or `Emma`, excluding fragments whose `Name()` is `dummy`.

The converter reads the same `Channel` calibration file as `s2426Sort`, because `Fragment::Name()`, `Fragment::Energy()`, and `Fragment::ArryNumber()` use `Channel`.

## ROOT Interactive Use

The `CHANNEL` library builds a ROOT dictionary, so `Channel` can be used from an interactive ROOT session after loading the shared library:

```cpp
gSystem->Load("build/lib/libCHANNEL.dylib");
Channel::Read("cal/CalibrationFile_May1526_pol1.cal");
```

## Output Trees

`EventProcess` receives built fragment groups from `EventBuilder`, converts them into `Event` objects, and sends them to `OutputManager`.

The current ROOT outputs are:

- `Fragment<run>_<subrun>.root`: stores every sorted `Fragment` in `FragmentTree`.
- `event<run>_<subrun>.root`: stores every built `Event` in `eventTree`.
- `goodevent<run>_<subrun>.root`: stores only events with `IsGood() == true` in `eventTree`.
- `Analysis<run>_<subrun>.root`: stores `Tigress`, `Emma`, and remaining-fragment `Event` objects in `AnalysisTree`.
