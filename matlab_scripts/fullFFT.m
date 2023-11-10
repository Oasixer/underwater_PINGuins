function fullFFT(signal, fs)
% signal: the input signal as a vector
% fs: the sampling rate of the signal

% calculate an FFT of the entire data
fftData = abs(fft(signal));
freqs = linspace(0, fs/2, length(signal)/2);

% Plot the current FFT as a bar graph
figure
bar(freqs, 2*fftData(1, 1:length(signal)/2)/length(signal));
xlabel('Frequency (Hz)');
ylabel('Magnitude');
title("FFT of entire signal");

end