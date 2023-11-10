function plotFreqsOfInterest(signal, N, fs, freqsOfInterest, nBins)
% signal: the input signal as a vector
% N: the size of the sliding window (in samples)
% fs: the sampling rate of the signal
% freqsOfInterst: the frequencies to plot the data at
% nBins: the number of bins in the histogram

t = 0:1/fs:(length(signal)-1)/fs;

%% make a sliding DFT at freqsOfInterest
dftData = zeros(length(freqsOfInterest), length(signal));
for f = 1:length(freqsOfInterest)
    dftData(f, :) = slidingDFTatFreq(signal, N, fs, freqsOfInterest(f));
end

%% plot all freqsOfInterest on the same graph
figure
for f = 1:length(freqsOfInterest)
    plot(t, dftData(f, :), 'DisplayName', num2str(freqsOfInterest(f)) + "Hz"); hold on;
end
legend();
xlabel("Time [s]");
ylabel("Freq amplitude");
title("Freqs of interest over time");

%% plot each of freqsOfInterest on its own 
% figure
% for f = 1:length(freqsOfInterest)
%     subplot(3, 4, f*2-1)
%     plot(t, dftData(f, :))
%     xlabel("Time [s]")
%     ylabel("Freq amplitude")
%     title(num2str(freqsOfInterest(f)) + " Hz over time")
%     subplot(3, 4, f*2)
%     histogram(dftData(f, :), nBins)
%     xlabel("Amplitude")
%     ylabel("Num Occurance")
%     title(num2str(freqsOfInterest(f)) + " Hz histogram")
% end

% %% plot histogram of all data
% figure
% for f = 1:length(freqsOfInterest)
%     histogram(dftData(f, :), nBins); hold on;
% end
% xlabel("Amplitude")
% ylabel("Num Occurance")
% title("Histogram of freqs of interest")
end