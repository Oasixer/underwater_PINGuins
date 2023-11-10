clc; clear all; close all;
%% parameters
file_name = "dft.gif";
total_gif_time = 15; % seconds
skip_size = 5;  % how many samples to skip between frames
minimum_freq = 10000; % Hz
maximum_freq = 50000; % Hz

%% constants
fs = 500000;
N = 500;
freq_sent = 26000;
freqs = (minimum_freq:fs/N:maximum_freq) / 1000;
main_color = "#FFFFFF";
background_color = 'none';

%% get the real data
addpath 'C:\Users\Ahmad\Desktop\fourth_year\FYDP\fydp_software\Matlab_scripts'
signal = readUint16File("../../adc_recordings/Feb23/dist-4-500-3")';
signal = signal(1, 131250:135000);
signal = double(signal) ./ 4096 .* 3.3;

%% calculated parameters
frame_count = floor((length(signal) - N) / skip_size) + 1;
delay_time = total_gif_time / frame_count;

%% create the sent data
t = (1:length(signal)) ./ fs * 1000;
sent = zeros(1, length(t)) + 30;
sent(N: 2*N) = sin(2 * pi * freq_sent / 1000 * t(N: 2*N)) * 30 + 30;

%% set up the plot
fig = figure;
ax1 = subplot(3, 1, 1);
line_handle1 = plot(ax1, t(1:N), sent(1, N), 'Color', main_color);
ylim(ax1, [0 60])
xlim(ax1, [0 1])
xlabel(ax1, "Time [ms]")
ylabel(ax1, "Sent")
set(ax1, 'ytick', [])
set(ax1, 'Color', background_color)
set(ax1, 'XColor', main_color)
set(ax1, 'YColor', main_color)
set(ax1, "xtick", 0:0.25:8);

ax2 = subplot(3, 1, 2);
line_handle2 = plot(ax2, t(1: N), signal(1: N), "Color", main_color);
ylim(ax2, [0 3.3])
xlim(ax2, [0 1])
xlabel(ax2, "Time [ms]")
ylabel(ax2, "Received")
set(ax2, 'ytick', [])
set(ax2, 'Color', background_color)
set(ax2, 'XColor', main_color)
set(ax2, 'YColor', main_color)
set(ax2, "xtick", 0:0.25:8);

ax3 = subplot(3, 1, 3);
dft = abs(fft(signal(1: N))) / (N/2);
dft = dft(11:51);
line_handle3 = bar(ax3, freqs, dft, "FaceColor", main_color);
ylim(ax3, [-0.01 1.01])
set(ax3, 'ytick', [])
xlabel(ax3, "Frequency [KHz]")
ylabel(ax3, "Magnitude");
set(ax3, 'Color', background_color)
set(ax3, 'XColor', main_color)
set(ax3, 'YColor', main_color)

set(fig, 'Color', 'None');

%% create the plot
im = cell(1, frame_count);
for i = 1:frame_count
    % get indices
    start_index = (i-1) * skip_size + 1;
    end_index = start_index + N - 1;

    % top subplot
    set(line_handle1, 'XData', t(1, start_index: end_index), 'YData', sent(1, start_index: end_index));
    set(ax1, "XLim", [t(start_index) t(end_index)]);
    set(ax1, "xtick", 0:0.25:8);

    % middle subplot
    window = signal(1, start_index: end_index);
    set(line_handle2, 'XData', t(1, start_index: end_index), 'YData', window);
    xlim(ax2, [t(start_index) t(end_index)]);
    set(ax2, "xtick", 0:0.25:8);

    % bottom subplot
    dft = abs(fft(window)) / (N / 2);
    dft = dft(11:51);
    set(line_handle3, 'YData', dft);

    % ensure the color is transparent
    set(fig, 'Color', 'None');
    
    % Save the frame as an image
    frame = getframe(fig);
    im{i} = frame2im(frame);
    
    % Pause for a short time to control the frame rate
    pause(delay_time);
end


% Write the images to a GIF file
for i = 1:frame_count
    [A,map] = rgb2ind(im{i},256);
    if i == 1
        imwrite(A,map,file_name,'gif','LoopCount',Inf,'DelayTime',0.02, 'TransparentColor',0, 'DisposalMethod','restoreBG');
    else
        imwrite(A,map,file_name,'gif','WriteMode','append','DelayTime',0.02, 'TransparentColor',0, 'DisposalMethod','restoreBG');
    end
end

