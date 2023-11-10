function freqValues = slidingDFTatFreq(signal, N, fs, freqOfInterest)
% signal: the input signal as a vector
% windowSize: the size of the sliding window (in samples)
% stepSize: the step size between windows (in samples)
% fs: the sampling rate of the signal
% freqOfInterest: the frequency of interest (in Hz)

n = 0:N-1;
exp_j2pinf0n = exp(-1j*2*pi * freqOfInterest * n / fs);

freqValues = abs(conv(signal, exp_j2pinf0n,"full"));
freqValues = freqValues(1, 1:length(signal));
freqValues = freqValues / (N/2);
end