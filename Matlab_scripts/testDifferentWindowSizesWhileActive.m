freqsOfInterestExactAll = [10075, 18318, 26515, 34741, 43806, 50375];
filesAll = [
    "d0.4_10", "d1.8_10", "d3.9_10", "harmonics1"
    "d0.4_18", "d1.8_18", "d3.9_18", "harmonics2"
    "d0.4_26", "d1.8_26", "d3.9_26", "harmonics3"
    "d0.4_34", "d1.8_34", "d3.9_34", "harmonics4"
    "d0.4_42", "d1.8_42", "d3.9_42", "harmonics5"
    "d0.4_48", "d1.8_48", "d3.9_48", "harmonics6"
    ];

fs = 500000;
Ns = [50, 100, 200, 500, 1000, 1250, 2000, 2500, 5000];

NsForAllFrequencies = zeros(length(filesAll(:)), length(Ns));
meansForAllFrequencies = zeros(length(filesAll(:)), length(Ns));
i_file = 1;
for idxFreqOfInterest = 1:length(freqsOfInterestExactAll)
    files = filesAll(idxFreqOfInterest, :);
    freqsOfInterestExact = [freqsOfInterestExactAll(idxFreqOfInterest)];
    
    for j = 1:length(Ns)
        resolution = fs/Ns(j);
        freqsOfInterest = round(freqsOfInterestExact ./ resolution) * resolution;
        for i = 1:length(files)
            data = readUint16File("../adc_recordings/" + files(i))';
    
            curr_means = zeros(1, length(freqsOfInterest));
            for f = 1:length(freqsOfInterest)
                freqValues = slidingDFTatFreq(data, Ns(j), fs, freqsOfInterest(f));
                curr_means(1, f) = mean(freqValues);
            end

            meansForAllFrequencies((idxFreqOfInterest-1)*length(files)+i, j) = mean(curr_means);
            NsForAllFrequencies((idxFreqOfInterest-1)*length(files)+i, j) = Ns(j);
            i_file = i_file + 1
        end
    end
end

figure
for f = 1:length(filesAll(:))
    scatter(NsForAllFrequencies(f, :), meansForAllFrequencies(f, :)); hold on;
end
ylabel("mean magnitude");
xlabel("Window Size")

x = NsForAllFrequencies(:)';
y = meansForAllFrequencies(:)';
figure
invmodel = fittype('a/x', 'independent', 'x', 'dependent', 'y');
fitresult = fit(x', y', invmodel)
plot(x, y, 'o'); hold on;
plot(fitresult);
ylabel("mean magnitude");
xlabel("Window Size")
