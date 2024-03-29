//
// Created by daran on 1/12/2017.
//

#include "ece420_main.h"
#include "ece420_lib.h"
#include "kiss_fft/kiss_fft.h"
#include "audio_common.h"

#define VOICED_THRESHOLD 123456789 // Write your own!

// TODO: better initialization, this is not great
float lastFreqDetected = -1;

extern "C" {
JNIEXPORT float JNICALL
        Java_com_ece420_lab4_MainActivity_getFreqUpdate(JNIEnv *env, jclass);
}

void ece420ProcessFrame(sample_buf *dataBuf) {
    // Keep in mind, we only have 20ms to process each buffer!
    struct timeval start;
    struct timeval end;
    gettimeofday(&start, NULL);

    // Data is encoded in signed PCM-16, little-endian, mono
    float bufferIn[FRAME_SIZE];
    for (int i = 0; i < FRAME_SIZE; i++) {
        int16_t val = (int16_t) (((uint16_t) dataBuf->buf_[2 * i]) | (((uint16_t) dataBuf->buf_[2 * i + 1]) << 8));
        bufferIn[i] = (float) val;
    }

    // In this section, you will be computing the autocorrelation of bufferIn
    // and picking the delay corresponding to the best match. Naively computing the
    // autocorrelation in the time domain is an O(N^2) operation and will not fit
    // in your timing window.
    //
    // First, you will have to detect whether or not a signal is voiced.
    // We will implement a simple voiced/unvoiced detector by thresholding
    // the power of the signal.
    //
    // Next, you will have to compute autocorrelation in its O(N logN) form.
    // Autocorrelation using the frequency domain is given as:
    //
    //  autoc = ifft(fft(x) * conj(fft(x)))
    //
    // where the fft multiplication is element-wise.
    //
    // You will then have to find the index corresponding to the maximum
    // of the autocorrelation. Consider that the signal is a maximum at idx = 0,
    // where there is zero delay and the signal matches perfectly.
    //
    // Finally, write the variable "lastFreqDetected" on completion. If voiced,
    // write your determined frequency. If unvoiced, write -1.
    // ********************** START YOUR CODE HERE ************************ //
    kiss_fft_cpx in[FRAME_SIZE];
    kiss_fft_cpx out[FRAME_SIZE];
    float autoc[FRAME_SIZE];

    kiss_fft_cfg cfg = kiss_fft_alloc(FRAME_SIZE, 0, 0, 0);   //Set up cfg
    kiss_fft_cfg icfg = kiss_fft_alloc(FRAME_SIZE, 1, 0, 0);  //Set up icfg

    for (int n = 0; n < FRAME_SIZE; n++)        //Convert data to cpx
    {
        in[n].r = bufferIn[n];
        in[n].i = 0;
    }

    float energy = 0;
    for (int i = 0; i < FRAME_SIZE; ++i)
    {
        energy += pow(bufferIn[i],2);
    }

    if (energy > 2000000000)    //If threshold met perform autocorr
    {
        kiss_fft(cfg, in, out);

        for (int n = 0; n < FRAME_SIZE; n++) {
            out[n].r = out[n].r * out[n].r + out[n].i * out[n].i;
            out[n].i = 0;
        }

        kiss_fft(icfg, out, out);

        for (int n = 0; n < FRAME_SIZE; n++) {
            autoc[n] = out[n].r;
        }

        lastFreqDetected = F_S/(findMaxArrayIdx(autoc, 45, FRAME_SIZE/2));
    }
    else
    {
        lastFreqDetected = -1;
    }

    free(cfg);
    free(icfg);
    // ************************ END YOUR CODE HERE ************************ //

    gettimeofday(&end, NULL);
    LOGD("Time delay: %ld us",  ((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec)));
}

JNIEXPORT float JNICALL
Java_com_ece420_lab4_MainActivity_getFreqUpdate(JNIEnv *env, jclass) {
    return lastFreqDetected;
}




