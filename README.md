# S2426Sort Event Building Notes

## Contents

- [Event Building Rule](#event-building-rule)
- [Good Event Tagging](#good-event-tagging)
- [Fragment Class Contents](#fragment-class-contents)
- [Event Class Contents](#event-class-contents)
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

Important accessors:

- `IsGood()`: returns the good-event flag.
- `Size()`: returns the number of fragments.
- `Fragments()` and `FragmentAt(index)`: access stored fragments.
- `Cores()`, `Segments()`, `Bgos()`: access TIGRESS and BGO category indices.
- `Si()`, `ICs()`, `Anodes()`, `Left()`, `Right()`: access EMMA category indices.

## Output Trees

`EventProcess` receives built fragment groups from `EventBuilder`, converts them into `Event` objects, and sends them to `OutputManager`.

The current ROOT outputs are:

- `list<run>_<subrun>.root`: stores every sorted `Fragment` in `listTree`.
- `event<run>_<subrun>.root`: stores every built `Event` in `eventTree`.
- `goodevent<run>_<subrun>.root`: stores only events with `IsGood() == true` in `eventTree`.
