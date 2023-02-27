N = 500;
fs = 500000;

freqs = [11000, 18000, 25000, 36000, 42000, 50000];

f_natural = fs / N;

k = freqs ./ f_natural;

vector = vpa(exp(2*pi*1j /N .* k));
for k = 1:length(vector)
    fprintf('{%.30g, %.30g},  // freq = %iHz, N = %i\n', real(vector(k)), imag(vector(k)), freqs(k), N);
end

% t = 0:0.000002:0.000098;
% round(2048*sin(2*pi*10000*t) + 2048)'
