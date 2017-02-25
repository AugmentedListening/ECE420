import numpy as np
import matplotlib.pyplot as plt
from scipy import signal

#test_data
F_s = 48000
t = [i / F_s for i in range(2 * F_s)]

test_data = signal.chirp(t, 1, t[-1], 24000, method='logarithmic')


# Your filter design here
# firls() can be called via signal.firls()
nyq = 24000
bands = [0, 1000, 2000, 3000, 4000, 24000]
amp = [1, 0, 0, 0, 1, 1]
b = signal.firls(35, bands, amp, nyq=nyq)

# Signal analysis
w, h = signal.freqz(b)

# Convoling the filter

def convo(x, h):
    N = x.size
    K = h.size
    y = np.zeros(N + K - 1)
    for n in range(N + K):
        for k in range(K):
            if (n - k >= 0) and (n-k < K):
                y[n] += (x[n-k]*h[k])
    return y

filt = convo(test_data, b)

plt.plot(filt)



#plt.figure()
#plt.subplot(2,1,1)
#plt.title('Digital filter frequency response, N = ' + str(len(b)))
#plt.plot(w, 20 * np.log10(abs(h)), 'b')
#plt.ylabel('Amplitude [dB]', color='b')
#plt.grid()
#plt.axis('tight')

#plt.subplot(2,1,2)
#angles = np.unwrap(np.angle(h))
#plt.plot(w / np.pi, angles, 'g')
#plt.ylabel('Angle (radians)', color='g')
#plt.grid()
#plt.axis('tight')
#plt.xlabel('Frequency [0 to Nyquist Hz, normalized]')
plt.show()