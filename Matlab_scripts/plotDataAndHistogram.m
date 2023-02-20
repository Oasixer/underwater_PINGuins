function plotDataAndHistogram(data, timeStep, nBins)

figure
subplot(1,2,1)
t = 0:timeStep:(length(data)*timeStep-timeStep);
plot(t, data);
xlabel("Time [s]");

subplot(1,2,2)
histogram(data, nBins)
xlabel("Amplitude")
ylabel("Frequency")

end