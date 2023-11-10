function riseTimes = findRiseTime(data, period, windowSize, threshold, timeStep)
% data: the data to analyze
% period: the period in ms
% windowSize: the windowSize
% threshold: the threshold to use
% timeStep: the time step between samples (in seconds)

i = 1;
riseTimes = zeros(1, ceil(length(data)/(period/1000/timeStep)));
nPeaks = 1;
while (i < length(data))
    if (data(i) < threshold)  % below min
        i = i+1;
    else % detected
        riseTimes(nPeaks) = i;
        nPeaks = nPeaks + 1;
        i = i + round(period/1000/timeStep - windowSize * 10);
    end
end
end