function peaks = findPeaks(data, period, windowSize, threshold, timeStep)
% data: the data to analyze
% period: the period in ms
% windowSize: the windowSize
% threshold: the threshold to use
% timeStep: the time step between samples (in seconds)

i = 1;
peaks = zeros(1, ceil(length(data)/(period/1000/timeStep)));
nPeaks = 0;
while (i < length(data))
    if (data(i) < threshold)  % below min
        i = i+1;
    else % detected
        i_start = i;
        i_max = i;
        peak = data(i);
        while (i - i_start < windowSize*2 && i < length(data))
            if (data(i) > peak)
                peak = data(i);
                i_max = i;
            end
            i = i + 1;
        end
        nPeaks = nPeaks + 1;
        peaks(nPeaks) = i_max;
        i = i + round(period/1000/timeStep - windowSize * 10);
    end
end
end