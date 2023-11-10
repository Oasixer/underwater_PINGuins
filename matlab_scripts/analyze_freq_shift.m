dists = [0.4, 1.8, 3.9];
freqs_sent = [10, 18, 26, 34, 42, 48];

fs = 500000;

magnitude_f_sent = zeros(1, length(dists) * length(freqs_sent));
f_max_magnitude = zeros(1, length(dists) * length(freqs_sent));
max_magnitude = zeros(1, length(dists) * length(freqs_sent));
ds = zeros(1, length(dists) * length(freqs_sent));
ffs = zeros(1, length(dists) * length(freqs_sent));
for d = 1:length(dists)
    for f = 1:length(freqs_sent)
        ds(1, (d-1)*length(freqs_sent)+f) = dists(d);
        ffs(1, (d-1)*length(freqs_sent)+f) = freqs_sent(f);

        name = "d" + num2str(dists(d)) + "_" + num2str(freqs_sent(f));
        signal = readUint16File("../adc_recordings/" + name)';

        fftData = abs(fft(signal));
        freqs_received = linspace(0, fs/2, length(signal)/2);

        % remove DC gain
        fftData = fftData(1, 1000:length(signal)/2)/length(signal);
        freqs_received = freqs_received(1, 1000: length(signal)/2);

        % find magnitude @ f sent
        idx = find(freqs_received > freqs_sent(f)*1000, 1);
        magnitude_f_sent(1, (d-1)*length(freqs_sent)+f) = fftData(1, idx);

        [val, idx] = max(fftData);
        max_magnitude(1, (d-1)*length(freqs_sent)+f) = val;
        f_max_magnitude(1, (d-1)*length(freqs_sent)+f) = freqs_received(1, idx);
    end
end

data = zeros(length(dists) * length(freqs_sent), 5);
for row = 1:(length(dists)*length(freqs_sent))
    data(row, 1) = ds(1, row);
    data(row, 2) = ffs(1, row) * 1000;
    data(row, 3) = magnitude_f_sent(1, row);
    data(row, 4) = f_max_magnitude(1, row);
    data(row, 5) = max_magnitude(1, row);
end
format long
data