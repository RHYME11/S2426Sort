## EMMA ADC/TDC Event-Building Investigation

### Initial Checks in `EventProcess.cxx`

I investigated whether any events contain only EMMA ADC or only EMMA TDC data using an event-building time window of **2.5 μs**.

### Observations

- **TDC-only events**
  - A large number of events appeared to contain only TDC data.
  - However, these TDC events had **0 fragments**.
  - The origin of these empty TDC events still needs to be understood.

- **ADC-only events**
  - Many events contained only ADC data.
  - All of these events had **non-zero fragments**.
  - This appeared inconsistent with the unpacking logic.

### Additional Debugging During MIDAS Event Unpacking

The following checks were performed:

- Checked whether any MIDAS event entered the TDC unpacking code but ultimately pushed no fragments into `fQueue`.
  - Result: **0 cases**

- Checked whether any MIDAS event entered ADC unpacking but did not enter TDC unpacking.
  - Result: **0 cases**

- Verified in `s2426Sort.cxx` that ADC and TDC are unpacked from the same MIDAS event at the same time.
  - ADC unpacking function called: **94 times**
  - TDC unpacking function called: **94 times**

### Conclusion

Based on the current implementation:

- ADC and TDC data must originate from the same MIDAS event.
- They also share the same `TimestampNs()`.
- In fact, the code explicitly forces:
  
  ```cpp
  TDC timestamp = ADC timestamp

