%% constants
fs = 500000;
timeStep = 1/fs;
freqsOfInterestExact = [10075, 18318, 26515, 34741, 43806, 50375];
nBins = 100;  % the number of bins in a histogram
frameDuration = 0.00001;  % the duration of each frame in animation
stepSize = 100;  % the step size between windows in animation
videoName = "test";  % the video name to save animation with
period = 200;
threshold = 300;

%% files
dists = 1:4;
Ns = [250, 500, 1250];
indices = 1:6;

%% iterate
for dist =dists
    for N = Ns
        resolution = fs/N;
        freqsOfInterest = round(freqsOfInterestExact ./ resolution) * resolution;
        for index = indices
            file = "dist-" + num2str(dist) + "-" + num2str(N) + "-" + num2str(index);
            data = readUint16File("../adc_recordings/Feb23/" + file)';
            plotFreqsOfInterest(data, N, fs, freqsOfInterest, nBins)
            sgtitle("N = " + num2str(N) + ", file = " + file);
        end
    end
end


% compareDistMeasurementModes(data, period, N, threshold, timeStep, freqsOfInterest(1))
