# EMMA ADC/TDC Event-Building Investigation

## Background

This document summarizes the investigation of EMMA ADC/TDC event-building behavior, debugging steps, implemented fixes, and remaining unresolved issues.

---

# 1. Initial Investigation in `EventProcess.cxx`

An investigation was performed to determine whether any built events contain only EMMA ADC data or only EMMA TDC data.

The event-building time window used was:

```text
2.5 μs
```

## Observations

### TDC-Only Events

A large number of events appeared to contain only TDC data.

However:

- These TDC-only events contained **0 fragments**
- Their origin is still unclear
- Further investigation is required to determine where they come from

### ADC-Only Events

Many events appeared to contain only ADC data.

Unlike the TDC-only case:

- These events all contained **non-zero fragments**

This behavior initially appeared inconsistent with the unpacking logic.

---

# 2. MIDAS Event Unpacking Checks

Additional debugging checks were added during MIDAS event unpacking.

## Check 1

Checked whether any MIDAS event entered the TDC unpacking code but eventually pushed no fragments into `fQueue`.

### Result

```text
0 cases
```

---

## Check 2

Checked whether any MIDAS event entered ADC unpacking but did not enter TDC unpacking.

### Result

```text
0 cases
```

---

## Check 3

Verified in `s2426Sort.cxx` that ADC and TDC are unpacked from the same MIDAS event at the same time.

### Result

```text
ADC unpacking function called: 94 times
TDC unpacking function called: 94 times
```

---

# 3. Conclusion from the Unpacking Investigation

Based on the current implementation:

- ADC and TDC data must originate from the same MIDAS event
- ADC and TDC share the same `TimestampNs()`
- The code explicitly forces:

```cpp
TDC timestamp = ADC timestamp;
```

Therefore:

- The issue is most likely **not caused during unpacking**
- The next place to investigate is the behavior of `fQueue`

---

# 4. Event-Building Modifications

The following changes were implemented in the event-building logic.

## Modification 1

Do not build any events until:

```cpp
fQueue.size() >= 200e6
```

---

## Modification 2

Ensure:

```cpp
fQueue.top()
```

is ordered using:

```cpp
TimestampNs()
```

---

## Modification 3

Use an event-building time window of:

```text
2.5 μs
```

---

## Modification 4

Continue building events after all MIDAS unpacking has finished until:

```cpp
fQueue.size() == 0
```

---

# 5. Results After the Fix

After implementing the above changes:

- EMMA ADC and TDC data originating from the same MIDAS event are now successfully built into the same event

This confirms that the issue was related to queue/event-building behavior rather than unpacking.

---

# 6. Remaining Unresolved Problems

## 6.1 Events Containing Only TIGRESS Cores or Only EMMA

A significant number of events still contain only:

- TIGRESS core data
- EMMA data

Further investigation is still required.

---

## 6.2 EMMA-Only Events

Possible next step:

Generate diagnostic plots such as:

- `Si vs IC`
- `Si vs PGAC`

to determine whether these events are physically meaningful.

---

## 6.3 TIGRESS-Core-Only Events

A quick inspection of TIGRESS core-only spectra suggests:

- Doppler shifts make the issue difficult to study using only a single subrun
- Multiple subruns are likely required for meaningful comparison

---

## 6.4 EMMA Local Trigger vs Master Trigger

The timing difference between:

```text
EMMA_local_trigger - EMMA_master_trigger
```

has not yet been carefully investigated.

### Expected Behavior

The timing difference should be approximately:

```text
~1 μs
```

Therefore:

- Whenever EMMA ADC/TDC data exist in an event
- The corresponding EMMA master trigger should also exist in the same event

This still needs explicit verification.

---

# Current Status

## Resolved

- ADC and TDC data from the same MIDAS event are now correctly built into the same event

## Still Under Investigation

- Origin of empty TDC-only events
- TIGRESS-core-only events
- EMMA-only events
- EMMA local trigger vs master trigger timing consistency
