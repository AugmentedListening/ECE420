//
// Created by daran on 1/12/2017.
//

#include <jni.h>
#include "ece420_main.h"
#include "ece420_lib.h"
#include "kiss_fft/kiss_fft.h"
#include <math.h>


float fftOut[FFT_SIZE] = {};
bool isWritingFft = false;

float circBuf[FFT_SIZE] = {};


extern "C" {
JNIEXPORT void JNICALL
        Java_com_ece420_lab3_MainActivity_getFftBuffer(JNIEnv *env, jclass, jobject bufferPtr);
}

void ece420ProcessFrame(sample_buf *dataBuf) {
    isWritingFft = true;

    // Keep in mind, we only have a small amount of time to process each buffer!
    struct timeval start;
    struct timeval end;
    gettimeofday(&start, NULL);

    // Data is encoded in signed PCM-16, little-endian, mono channel
    int16_t data[FRAME_SIZE];
    for (int i = 0; i < FRAME_SIZE; i++) {
        data[i] = ((uint16_t) dataBuf->buf_[2 * i]) | (((uint16_t) dataBuf->buf_[2 * i + 1]) << 8);
    }

    // This could be changed in lower levels, but for now, we will keep our data input as int16_t
    float dataFloat[FRAME_SIZE];
    for (int i = 0; i < FRAME_SIZE; i++) {
        dataFloat[i] = (float) data[i];
    }

    // Used to store 2 frames at a time for 50% overlap
    for(int i = 0; i < FRAME_SIZE; i++)
    {
        circBuf[i] = circBuf[i+FRAME_SIZE];
        circBuf[i+FRAME_SIZE] = dataFloat[i];
    }

//    for(int i =0; i<FFT_SIZE - FRAME_SIZE; i++)
//    {
//        circBuf[i] = circBuf[i+FRAME_SIZE];
//    }
//
//    for (int i =0; i <FRAME_SIZE; i++)
//    {
//        circBuf[i+(FFT_SIZE - FRAME_SIZE)] = dataFloat[i];
//    }

    // ********************* START YOUR CODE HERE ***********************
    // NOTE: This code block is a suggestion to get you started. You will have to
    // add/change code outside this block to implement FFT buffer overlapping.
    //
    // Keep all of your code changes within java/MainActivity and cpp/ece420_*

    float window[FFT_SIZE];
    float zeropad[FFT_SIZE * ZP_FACTOR];

    kiss_fft_cpx in[FFT_SIZE*ZP_FACTOR];
    kiss_fft_cpx out[FFT_SIZE*ZP_FACTOR];

    float mag;

    for (int i = 0; i < FFT_SIZE; i++)
    {
        window[i] = 0.54 - 0.46*cos((2*M_PI*i)/(FFT_SIZE-1));
        zeropad[i] = circBuf[i] * window[i];
    }

    kiss_fft_cfg cfg = kiss_fft_alloc(FFT_SIZE*ZP_FACTOR,0,NULL,NULL);

    for (int n = 0; n < FFT_SIZE*ZP_FACTOR; n++)
    {
        in[n].r = zeropad[n];
        in[n].i = 0;
    }

    kiss_fft(cfg, in, out);

    free(cfg);

    for (int n = 0; n < FFT_SIZE; n++)
    {
        mag = log10(pow(out[n].r, 2) + pow(out[n].i, 2) + 1)/15;
        fftOut[n] = mag;
    }
    // ********************* END YOUR CODE HERE *************************

    isWritingFft = false;

    gettimeofday(&end, NULL);
    LOGD("Time delay: %ld us",  ((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec)));
}


// http://stackoverflow.com/questions/34168791/ndk-work-with-floatbuffer-as-parameter
// Do not modify
JNIEXPORT void JNICALL
Java_com_ece420_lab3_MainActivity_getFftBuffer(JNIEnv *env, jclass, jobject bufferPtr) {

    jfloat *buffer = (jfloat *) env->GetDirectBufferAddress(bufferPtr);

    // thread-safe, kinda
    while (isWritingFft) {}

    for (int i = 0; i < FFT_SIZE; i++) {
        buffer[i] = fftOut[i];
    }
}
