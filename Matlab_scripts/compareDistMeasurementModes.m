function compareDistMeasurementModes(data, period, N, threshold, timeStep, freq)
% data: the data to analyze
% period: the period in ms
% M: the windowSize
% threshold: the threshold to use
% timeStep: the time step between samples (in seconds)
% freq: the frequency in which to do the analysis (in Hz)

signal = slidingDFTatFreq(data, N, 1/timeStep, freq);

peaks = findPeaks(signal, period, N, threshold, timeStep);
diffBetweenPeaks = zeros(size(peaks));
for i = 1:length(peaks)-1
    diffBetweenPeaks(i) = (period / 1000 - (peaks(i+1) - peaks(i)) * timeStep) * 1500;
end

riseTimes = findRiseTime(signal, period, N, threshold, timeStep);
diffBetweenRiseTimes = zeros(size(riseTimes));
for i = 1:length(riseTimes)-1
    diffBetweenRiseTimes(i) = (period / 1000 - (riseTimes(i+1) - riseTimes(i)) * timeStep) * 1500;
end

figure
plot(diffBetweenPeaks, 'DisplayName', "Peak Detection"); hold on;
plot(diffBetweenRiseTimes, 'DisplayName', "Rising Edge"); 
legend()
ylabel("Dist error [m]")
xlabel("sample")
title("Rising Edge vs Peak Detection")

diffBetweenPeakAndRiseTime = (peaks - riseTimes) * timeStep;
diffBetweenConsistencies = diffBetweenPeaks - diffBetweenRiseTimes;
figure
subplot(1, 2, 1)
plot(diffBetweenPeakAndRiseTime, 'o');
xlabel("sample")
ylabel("Time difference [s]");
title("Difference between peak time and rise time");
subplot(1, 2, 2)
plot(diffBetweenConsistencies, 'o')
xlabel("sample")
ylabel("Dist difference [m]");
title("Difference between measurements of peak time and rise time");
end
