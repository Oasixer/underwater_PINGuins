%% constants
N = 1250;
fs = 500000;
timeStep = 1/fs;
resolution = fs/N;
freqsOfInterestExact = [10075, 18318, 26515, 34741, 43806, 50375];
freqsOfInterest = round(freqsOfInterestExact ./ resolution) * resolution;
nBins = 100;  % the number of bins in a histogram
frameDuration = 0.00001;  % the duration of each frame in animation
stepSize = 100;  % the step size between windows in animation
videoName = "test";  % the video name to save animation with
period = 200;
threshold = 200;

%% data
file = "d4.7_10_1250_200"
data = readUint16File("../adc_recordings/Feb22/" + file)';

%% plot
animateSlidingDFT(data, N, stepSize, fs, frameDuration, file)
% plotDataAndHistogram(data, timeStep, nBins);
% fullFFT(data, fs);

% dists = 1:4;
% indices = 1:6;
% for dist=dists
%     for index=indices
%         file = "dist-" + num2str(dist) + "-250-" + num2str(index);
%         data = readUint16File("../adc_recordings/Feb23/" + file)';
%         plotFreqsOfInterest(data, N, fs, freqsOfInterest, nBins)
%         sgtitle("N = " + num2str(N) + ", file = " + file);
%     end
% end
% 
% for index=indices
%     file = "dist_4_250_" + num2str(index);
%     data = readUint16File("../adc_recordings/Feb23/" + file)';
%     plotFreqsOfInterest(data, N, fs, freqsOfInterest, nBins)
%     sgtitle("N = " + num2str(N) + ", file = " + file);
% end

% plotDataAndHistogram(data, timeStep, nBins); sgtitle("Raw data")
% freq = slidingDFTatFreq(data, N, fs, freqsOfInterest(1));
% plotDataAndHistogram(freq, timeStep, nBins); sgtitle("Freq of interest")
% 
% compareDistMeasurementModes(data, period, N, threshold, timeStep, freqsOfInterest(1))


% fullFFT(data, fs)
% axis([5000 55000 0 0.1]);
% plotDataAndHistogram(data, timeStep, nBins);

% t = 0:0.000002:0.4-0.000002;
% figure
% plot(t, data)
% xlabel("time [s]")
% ylabel("raw amplitude")
% 

% figure
% t = 0:timeStep:0.2-timeStep;
% for i = 1:2
%     plot(t, freq(1, i*100000:(i+1)*100000-1)); hold on;
% end
% xlabel("time [s]")


% animateSlidingDFT(data, N, stepSize, fs, frameDuration, videoName)
