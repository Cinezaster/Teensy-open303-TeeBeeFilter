#include "filter_teebee303.h"

void AudioFilterTeeBee303::setCutoff(float newCutoff) {
  if (newCutoff != cutoff) {
    cutoff = constrain(newCutoff, 200, 20000);
    calculateCoefficientsApprox4();
  }
}

void AudioFilterTeeBee303::setResonance(float newResonance) {
  resonanceRaw = newResonance;
  resonanceSkewed = (1.0f - exp(-3.0f * resonanceRaw)) / (1.0f - exp(-3.0f));
  calculateCoefficientsApprox4();
}

void AudioFilterTeeBee303::setDrive(float newDrive) {
  drive = newDrive;
  //driveFactor = dB2amp(drive);
  driveFactor = exp(drive * 0.11512925464970228420089957273422);
}

void AudioFilterTeeBee303::calculateHighPassFilterCoefficients() {
  float hp_x = exp(-2.0f * PI * hp_cutoff * (1.0f / AUDIO_SAMPLE_RATE_EXACT));
  hp_b0 = 0.5f * (1.0f + hp_x);
  hp_b1 = -0.5f * (1.0f + hp_x);
  hp_a1 = hp_x;
}

void AudioFilterTeeBee303::calculateCoefficientsExact() {
  // calculate intermediate variables:
  float wc = twoPiOverSampleRate * cutoff;
  float c = cos(wc);
  float s = sin(wc);
  // sinCos(wc, &s, &c);  // c = cos(wc); s = sin(wc);
  float t = tan(0.25f * (wc - PI));
  float r = resonanceSkewed;

  // calculate filter a1-coefficient tuned such the resonance frequency is just right:
  float a1_fullRes = t / (s - c * t);

  // calculate filter a1-coefficient as if there were no resonance:
  float x = exp(-wc);
  float a1_noRes = -x;

  // use a weighted sum between the resonance-tuned and no-resonance coefficient:
  a1 = r * a1_fullRes + (1.0 - r) * a1_noRes;

  // calculate the b0-coefficient from the condition that each stage should be a leaky
  // integrator:
  b0 = 1.0f + a1;

  // calculate feedback factor by dividing the resonance parameter by the magnitude at the
  // resonant frequency:
  float gsq = b0 * b0 / (1.0f + a1 * a1 + 2.0 * a1 * c);
  k = r / (gsq * gsq);

  k *= (17.0f / 4.0f);
}

void AudioFilterTeeBee303::calculateCoefficientsApprox4() {
  // calculate intermediate variables:
  float wc = twoPiOverSampleRate * cutoff;
  float wc2 = wc * wc;
  float r = resonanceSkewed;
  float tmp;

  // compute the filter coefficient via a 12th order polynomial approximation (polynomial
  // evaluation is done with a Horner-rule alike scheme with nested quadratic factors in the hope
  // for potentially better parallelization compared to Horner's rule as is):
  const float pa12 = -1.341281325101042e-02;
  const float pa11 = 8.168739417977708e-02;
  const float pa10 = -2.365036766021623e-01;
  const float pa09 = 4.439739664918068e-01;
  const float pa08 = -6.297350825423579e-01;
  const float pa07 = 7.529691648678890e-01;
  const float pa06 = -8.249882473764324e-01;
  const float pa05 = 8.736418933533319e-01;
  const float pa04 = -9.164580250284832e-01;
  const float pa03 = 9.583192455599817e-01;
  const float pa02 = -9.999994950291231e-01;
  const float pa01 = 9.999999927726119e-01;
  const float pa00 = -9.999999999857464e-01;
  tmp = wc2 * pa12 + pa11 * wc + pa10;
  tmp = wc2 * tmp + pa09 * wc + pa08;
  tmp = wc2 * tmp + pa07 * wc + pa06;
  tmp = wc2 * tmp + pa05 * wc + pa04;
  tmp = wc2 * tmp + pa03 * wc + pa02;
  a1 = wc2 * tmp + pa01 * wc + pa00;
  b0 = 1.0 + a1;

  // compute the scale factor for the resonance parameter (the factor to obtain k from r) via an
  // 8th order polynomial approximation:
  const float pr8 = -4.554677015609929e-05;
  const float pr7 = -2.022131730719448e-05;
  const float pr6 = 2.784706718370008e-03;
  const float pr5 = 2.079921151733780e-03;
  const float pr4 = -8.333236384240325e-02;
  const float pr3 = -1.666668203490468e-01;
  const float pr2 = 1.000000012124230e+00;
  const float pr1 = 3.999999999650040e+00;
  const float pr0 = 4.000000000000113e+00;
  tmp = wc2 * pr8 + pr7 * wc + pr6;
  tmp = wc2 * tmp + pr5 * wc + pr4;
  tmp = wc2 * tmp + pr3 * wc + pr2;
  tmp = wc2 * tmp + pr1 * wc + pr0;  // this is now the scale factor
  k = r * tmp;
  g = 1.0;

  float fx = wc * ONE_OVER_SQRT2 / (2 * PI);
  b0 = (0.00045522346f + 6.1922189f * fx) / (1.0 + 12.358354f * fx + 4.4156345f * (fx * fx));
  k = fx * (fx * (fx * (fx * (fx * (fx + 7198.6997f) - 5837.7917f) - 476.47308f) + 614.95611f) + 213.87126f) + 16.998792f;
  g = k * 0.058823529411764705882352941176471;  // 17 reciprocal
  g = (g - 1.0f) * r + 1.0f;                    // r is 0 to 1.0
  g = (g * (1.0f + r));
  k = k * r;  // k is ready now
}

void AudioFilterTeeBee303::update(void) {
  audio_block_t *block;
  block = receiveWritable();
  if (!block) return;

  for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
    float input = block->data[i] * (1.0f / 32768);
    float hp_input = k * y4;  // calculate highpass feedback
    hp_y1 = (hp_b0 * hp_input + hp_b1 * hp_x1 + hp_a1 * hp_y1 + TINY);
    float y0 = input - hp_y1;
    hp_x1 = hp_input;

    y1 += 2 * b0 * (y0 - y1 + y2);
    y2 += b0 * (y1 - 2 * y2 + y3);
    y3 += b0 * (y2 - 2 * y3 + y4);
    y4 += b0 * (y3 - 2 * y4);
    block->data[i] = 2 * g * y4 * 32767.0f;
  }
  transmit(block);
  release(block);
}