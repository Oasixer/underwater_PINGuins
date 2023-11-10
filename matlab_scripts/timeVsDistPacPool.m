data = open("data.mat").data;

real_dists = data(:, 1);
% measured_dists = data(:, 2);
trip_times = data(:, 3);
magnitudes = data(:, 4) / 512000 * 1.65;

%% get a polynomial for distance vs trip time
p = polyfit(trip_times, real_dists, 1)

sprintf("Dist = %.20f * time_in_us + %.20f", p(1), p(2))
sprintf("speed of sound in water = %0.20f m/s", p(1)*2*1000000)
sprintf("calculated time delay = %0.20f us", abs(p(2))/p(1))

%% plot distance vs trip time
figure
scatter(trip_times, real_dists); hold on;
plot(trip_times, polyval(p, trip_times));
title("Distance Found Using Trip Time")
ylabel("dist [m]")
xlabel("trip time [us]")

%% plot real distance vs predicted distance
predicted_dists = polyval(p, trip_times);
figure
scatter(real_dists, predicted_dists);
title("Real vs predicted distance")
xlabel("Real Distance [m]")
ylabel("Predicted Distance [m]")

%% estimate error and variance
error = predicted_dists - real_dists;
figure
scatter(real_dists, error)
ylabel("Error [m]")
xlabel("Real dist [m]")

distances = 2:2:20;
stds = zeros(length(distances), 1);
for i = 1:length(distances)
    stds(i) = std(error((i-1)*50+1: i*50, 1));
end

figure
scatter(distances, stds)
title("Distance vs standard deviation")
xlabel("distance [m]")
ylabel("standard deviation")

%% get a polynomial for the magnitude vs distance
minimums = zeros(length(distances), 1);
for i = 1:length(distances)
    minimums(i) = min(magnitudes((i-1)*50+1: i*50, 1));
end

many_dists = 2:0.1:100;
% 
% p2 = polyfit(real_dists.^-1, magnitudes, 2)
% predicted_magnitudes = polyval(p2, many_dists.^-1);
% 
% p3 = polyfit(distances.^-1, minimums, 2)
% predicted_minimum_magnitudes = polyval(p3, many_dists.^-1);
% 
% figure
% scatter(real_dists, magnitudes); hold on;
% plot(many_dists, predicted_magnitudes);
% plot(many_dists, predicted_minimum_magnitudes);
% xlabel("Real Dist [m]")
% ylabel("Frequency Magnitude (amplitude) [V]")
% legend(["Measured data", "Fitted To all Data", "Fitted to minimum"])
% title("Distance vs frequency magnitude")

%% cap c to 0
eqn = fittype('a*x.^-2 + b*x.^-1');

% Create a fit options object
opts = fitoptions(eqn);

% Set the lower and upper bounds for the parameters
opts.Lower = [-Inf -Inf];
opts.Upper = [Inf Inf];

[fitted_curve_all, goodness_of_fit] = fit(real_dists, magnitudes, eqn, opts)
[fitted_curve_min, goodness_of_fit] = fit(distances', minimums, eqn, opts)

figure
scatter(real_dists, magnitudes); hold on;
plot(many_dists, fitted_curve_all(many_dists)); 
plot(many_dists, fitted_curve_min(many_dists));
line([min(many_dists) max(many_dists)], [0.080566 0.080566])
line([min(many_dists) max(many_dists)], [0.03223 0.03223])
xlabel("Real Dist [m]")
ylabel("Frequency Magnitude (amplitude) [V]")
legend(["Measured data", "Fitted To all Data", "Fitted to minimum", "Threshold"])
title("Distance vs frequency magnitude")