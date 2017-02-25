//
// Created by daran on 1/12/2017.
//
#include <queue>
#include "ece420_main.h"
#define FRAME_SIZE 128

// TODO: Change this to match your filter
#define N_TAPS 81

int16_t firFilter(int16_t sample);

void ece420ProcessFrame(sample_buf *dataBuf) {
    // Keep in mind, we only have a small amount of time to process each buffer!
    struct timeval start;
    struct timeval end;
    gettimeofday(&start, NULL);

    // Using {} initializes all values in the array to zero
    int16_t bufferIn[FRAME_SIZE] = {};
    int16_t bufferOut[FRAME_SIZE] = {};


    // ******************** START YOUR CODE HERE ********************

    // Your buffer conversion here

    for(int index = 0; index < FRAME_SIZE; index++) {
        int16_t sample = int16_t(uint16_t(dataBuf->buf_[2 * index]) | (dataBuf->buf_[2 * index + 1] << 8));
        bufferIn[index] = sample;
    }

    // Loop code provided as a suggestion. This loop simulates sample-by-sample processing.
    for (int sampleIdx = 0; sampleIdx < FRAME_SIZE; sampleIdx++) {
        int16_t sample = bufferIn[sampleIdx];

        // Your function implementation
        int16_t output = firFilter(sample);

        bufferOut[sampleIdx] = output;
    }

    // Your buffer conversion here

    for (int index =0; index < FRAME_SIZE; index++)
    {
        uint8_t bottom = bufferOut[index] & 0xFF;
        dataBuf->buf_[2*index] = bottom;
        uint8_t top = bufferOut[index] >> 8;
        dataBuf->buf_[2*index+1] = top;
    }
    // ********************* END YOUR CODE HERE *********************



    gettimeofday(&end, NULL);
    LOGD("Loop timer: %ld us",  ((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec)));

}


int16_t circBuf[N_TAPS] = {};
int circBufIdx = 0;
int circBufIdx_next = 0;

float coefs[] = {0.000686674172222, 0.000878107026649, 0.00119463907757, 0.00166341923736, 0.00228957036003, 0.00304629975257, 0.00386975979641, 0.00466129483695, 0.00529855175334, 0.00565518737889, 0.00562688857049, 0.00515955040847, 0.00427417757722, 0.00308274990303, 0.00179012450469, 0.000679004855374, 7.78186466009e-05, 0.000314533426561, 0.00166240332483, 0.00428578104431, 0.00819496861376, 0.0132183684855, 0.0189979746716, 0.0250108478122, 0.0306152217452, 0.0351159946246, 0.0378412588659, 0.038219769391, 0.035849132982, 0.030546007316, 0.0223724092586, 0.0116358002251, -0.00113571989701, -0.015238657549, -0.0298532637575, -0.0441092006924, -0.0571507918707, -0.0681962918292, -0.0765875051227, -0.0818279951121, 0.916390376364, -0.0818279951121, -0.0765875051227, -0.0681962918292, -0.0571507918707, -0.0441092006924, -0.0298532637575, -0.015238657549, -0.00113571989701, 0.0116358002251, 0.0223724092586, 0.030546007316, 0.035849132982, 0.038219769391, 0.0378412588659, 0.0351159946246, 0.0306152217452, 0.0250108478122, 0.0189979746716, 0.0132183684855, 0.00819496861376, 0.00428578104431, 0.00166240332483, 0.000314533426561, 7.78186466009e-05, 0.000679004855374, 0.00179012450469, 0.00308274990303, 0.00427417757722, 0.00515955040847, 0.00562688857049, 0.00565518737889, 0.00529855175334, 0.00466129483695, 0.00386975979641, 0.00304629975257, 0.00228957036003, 0.00166341923736, 0.00119463907757, 0.000878107026649, 0.000686674172222};

int16_t firFilter(int16_t sample) {
    double output = 0;

    // This function simulates sample-by-sample processing. Here you will
    // implement an FIR filter such as:
    //
    // y[n] = a x[n] + b x[n-1] + c x[n-2] + ...
    //
    // You will maintain a circular buffer to store your prior samples
    // x[n-1], x[n-2], ..., x[n-k]. Suggested initializations circBuf
    // and circBufIdx are given.
    //
    // Input 'sample' is the current sample x[n].
    // ******************** START YOUR CODE HERE *******************
    circBufIdx = circBufIdx_next;
    circBufIdx_next = (circBufIdx + 1) % N_TAPS;
    circBuf[circBufIdx] = sample;
    for (int i = 0; i < N_TAPS; ++i)
    {
        output += coefs[i]*circBuf[circBufIdx];
        circBufIdx = (circBufIdx + N_TAPS - 1) % N_TAPS;

    }
    // ********************* END YOUR CODE HERE *********************

    return (int16_t) output;
}
