%% get the data
addpath 'C:\Users\Ahmad\Desktop\fourth_year\FYDP\fydp_software\Matlab_scripts'
data = readUint16File("../../adc_recordings/Feb23/dist-4-500-2")';
data = data(1, 216700:220900);

freqOfInterest = 18000;
freqsOfInterest = [10000, 18000, 26000, 34000, 42000, 48000];
fs = 500000;
N = 500;

t = (1:length(data)) ./ fs;
ideal = sin(2 * pi * 26000 * t) * 100 + 2048;

freqValues = zeros(6, length(t));
for i = 1:6
    freqValues(i, :) = slidingDFTatFreq(data, N, fs, freqsOfInterest(i));
end

figure
subplot(2, 1, 1);
plot(t, data); hold on;
% plot(t, ideal);

subplot(2,1,2);
for i = 1:6
    plot(t, freqValues(i, :)); hold on;
end
legend(["10", "18", "26", "34", "42", "48"]);