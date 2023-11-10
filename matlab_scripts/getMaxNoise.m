data1 = readUint16File("../adc_recordings/Feb22/silence1")';
data2 = readUint16File("../adc_recordings/Feb22/silence2")';
data3 = readUint16File("../adc_recordings/Feb22/silence4")';
data4 = readUint16File("../adc_recordings/Feb22/silence4")';
data5 = readUint16File("../adc_recordings/Feb22/silence5")';

freqsOfInterest = [11000, 18000, 25000, 36000, 42000, 50000];

plotFreqsOfInterest(data1, 500, 500000, freqsOfInterest, 100);
plotFreqsOfInterest(data2, 500, 500000, freqsOfInterest, 100);
plotFreqsOfInterest(data3, 500, 500000, freqsOfInterest, 100);
plotFreqsOfInterest(data4, 500, 500000, freqsOfInterest, 100);
plotFreqsOfInterest(data5, 500, 500000, freqsOfInterest, 100);