# Teensy-open303-TeeBeeFilter
Teensy audio filter based on the TeeBeeFilter from the open303 project https://sourceforge.net/projects/open303/

This filter has been developed and tested on Teensy 4.1. 

The onePole Highpass filter in the feedbackSignal has been implemented directly within the TeebeeFilter

## TODO

- Look into why the input signal can't more than 50%, else the output signal gets distorted (almost wavefolded).
- Implement external cutoff control
- Implement external resonance control 
