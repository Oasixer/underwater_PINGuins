% Generate an input signal
t = 0:0.000002:1;
signal = sin(2*pi*20000*t) + randn(size(t));

for i = 1:20
    start = (i-1)*25000+1;
    last = start + 12500 - 1;
    signal(1, start: last) = randn(1, 12500);
end



%%
windowSize = 1250;
fs = 500000;
timeStep = 1/fs;

f = slidingDFTatFreq(signal, windowSize, fs, 20000);
plotDataAndHistogram(f, timeStep, 100);
sgtitle("Fourier analysis and raw data at 20KhZ")

animateSlidingDFT(signal, windowSize, 500, fs, 0.0001, "test")