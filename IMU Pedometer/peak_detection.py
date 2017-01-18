import numpy as np
import matplotlib.pyplot as plt

csv_filename = 'sample_sensor_data.csv'
data = np.genfromtxt(csv_filename, delimiter = ',').T
timestamps = (data[0] - data[0,0]) /1000

accel_data = data[1:4]
gyro_data = data[4:-1]

def peak_detection(t, sig, threshold):
    peaks = []
    max_val = -np.Inf
    N = len(sig)
    for i in range(2, N-2):
        if sig[i] - sig[i+2] > threshold and sig[i] - sig[i-2] > threshold and sig[i] > 0:
            max_val = sig[i]
            position = t[i]
            peaks.append((position, max_val))
    return np.array(peaks)

max_peaks = peak_detection(timestamps, accel_data[0], 0.01)

plt.scatter(max_peaks[:,0], max_peaks[:,1], color = 'red')
plt.plot(timestamps, accel_data[0])
plt.show()