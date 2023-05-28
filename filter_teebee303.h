#ifndef filter_teebee303_h_
#define filter_teebee303_h_

#include "Arduino.h"
#include "AudioStream.h"

#define ONE_OVER_SQRT2 0.70710678118654752440084436210485F
#define TINY 1.17549435E-38F /* b**(emin-1) */
#define EPS 2.2204460492503131e-16F

class AudioFilterTeeBee303 : public AudioStream {
public:
  /** Constructor. */
  AudioFilterTeeBee303()
    : AudioStream(3, inputQueueArray) {
    cutoff = 1000.0;
    drive = 0.0;
    driveFactor = 1.0;
    resonanceRaw = 0.0;
    resonanceSkewed = 0.0;
    g = 1.0;

    twoPiOverSampleRate = 2.0f * PI / AUDIO_SAMPLE_RATE_EXACT;

    hp_cutoff = 150.0;

    calculateHighPassFilterCoefficients();
    calculateCoefficientsExact();
  };

  virtual void update(void);

  void setCutoff(float newCutoff);
  void setResonance(float newResonance);
  void setDrive(float newDrive);

private:
  float b0, a1;              // coefficients for the first order sections
  float y1, y2, y3, y4;      // output signals of the 4 filter stages
  float c0, c1, c2, c3, c4;  // coefficients for combining various ouput stages
  float k;                   // feedback factor in the loop
  float g;                   // output gain
  float driveFactor;         // filter drive as raw factor
  float cutoff;              // cutoff frequency
  float drive;               // filter drive in decibels
  float resonanceRaw;        // resonance parameter (normalized to 0...1)
  float resonanceSkewed;     // mapped resonance parameter to make it behave more musical
  float twoPiOverSampleRate;

  float hp_cutoff;
  // buffering:
  float hp_x1, hp_y1;

  // filter coefficients:
  float hp_b0;  // feedforward coeffs
  float hp_b1;
  float hp_a1;  // feedback coeff

  void calculateHighPassFilterCoefficients(); 
  void calculateCoefficientsExact();
  void calculateCoefficientsApprox4();

  audio_block_t *inputQueueArray[3];
};

#endif