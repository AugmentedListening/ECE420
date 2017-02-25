import numpy as np
import matplotlib.pyplot as plt
from scipy.io.wavfile import read, write
from scipy import signal
from numpy.fft import fft, ifft

FRAME_SIZE = 1024
ZP_FACTOR = 2
FFT_SIZE = FRAME_SIZE * ZP_FACTOR


################## YOUR CODE HERE ######################
def ece420ProcessFrame(frame):

    curFft = np.zeros(FFT_SIZE)
    window = signal.hamming(FRAME_SIZE)
    res = frame*window
    curFft[0: FRAME_SIZE] = res
    trans = fft(curFft)
    temp = np.log(np.square(np.abs(trans))+1)/25

    return temp[0:FRAME_SIZE]


################# GIVEN CODE BELOW #####################

Fs, data = read('test_vector.wav')

numFrames = int(len(data) /  FRAME_SIZE)
bmp = np.zeros((numFrames, (int) (FFT_SIZE / 2)))

for i in range(numFrames):
    frame = data[i * FRAME_SIZE : (i + 1) * FRAME_SIZE]
    curFft = ece420ProcessFrame(frame)
    bmp[i, :] = curFft

plt.figure()
plt.pcolormesh(bmp.T, vmin=0, vmax=1)
plt.axis('tight')
plt.xlabel('Time (ms)')
plt.ylabel('Frequency (Hz)')

plt.show()