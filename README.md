# Teensy-open303-TeeBeeFilter
Teensy audio filter based on the TeeBeeFilter form the open303 project https://sourceforge.net/projects/open303/

This filter has been developed and tested on Teensy 4.1. 

The onePole Highpass filter in the feedbackSignal is been implemented directly within the TeebeeFilter

## TODO

- Look into why the input signal can't more then 50%, else the output signal get's distorted (almost wavefolded).
- Implement external cutoff control
- Implement external resonance control 
