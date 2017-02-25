%matplotlib inline
import numpy as np
import matplotlib.pyplot as plt
from scipy.io.wavfile import read
from scipy.io.wavfile import write
from scipy import signal
from IPython.display import Audio
fig = plt.figure()

#FFT of signal

sampling_rate, data = read('with_hum.wav')
trans = np.fft.fft(data)
mag = np.abs(trans)
freq = np.fft.fftfreq(data.size, 1/sampling_rate)
freq_norm = 2*freq/(0.5*sampling_rate)

#Creating the filter
nyquist = sampling_rate/
b, a = signal.butter(4, [390/nyquist, 410/nyquist], btype='bandstop', analog=False)
w, h = signal.freqz(b,a)
q = w*sampling_rate*(1/(2*np.pi))


#Applying filter to signal

y = signal.lfilter(b, a, data)
write('filtered.wav',sampling_rate,asarray(y,dtype=int16))
Audio('filtered.wav')


sampling_ratey, datay = read('filtered.wav')
transy = np.fft.fft(datay)
magy = np.abs(transy)
freqy = np.fft.fftfreq(y.size, 1/sampling_rate)