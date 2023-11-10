% Define the data
x = linspace(0, 10, 1000);
y1 = sin(x);
y2 = cos(x);

% Set up the figure
fig = figure;
ax1 = subplot(3,1,1);
line_handle1 = plot(ax1, x(1:100), y1(1:100));
xlim(ax1, [x(1) x(100)]);
ylim(ax1, [-1 1]);
xlabel(ax1, 'Time');
ylabel(ax1, 'Amplitude');

ax2 = subplot(3,1,2);
line_handle2 = plot(ax2, x(1:100), y2(1:100));
xlim(ax2, [x(1) x(100)]);
ylim(ax2, [-1 1]);
xlabel(ax2, 'Time');
ylabel(ax2, 'Amplitude');

ax3 = subplot(3,1,3);
f = linspace(0, 10, 100);
dft_handle = bar(ax3, f, zeros(size(f)), 'BarWidth', 1);
xlim(ax3, [0 10]);
xlabel(ax3, 'Frequency');
ylabel(ax3, 'Magnitude');

% Set the size of the window and the DFT parameters
window_size = 100;
dft_size = 100;
skip_size = 7;

% Initialize the GIF file
filename = 'moving_window_and_dft.gif';
frame_rate = 60;
delay_time = 1 / frame_rate;
frame_count = floor((length(x) - window_size) / skip_size) + 1;
im = cell(1, frame_count);

    set(fig, 'Color', 'None');

% Update the plot and save each frame as an image
for i = 1:frame_count
    % Update the top subplot
    start_index = (i-1) * skip_size + 1;
    end_index = start_index + window_size - 1;
    set(line_handle1, 'XData', x(start_index:end_index), 'YData', y1(start_index:end_index));
    set(line_handle2, 'XData', x(start_index:end_index), 'YData', y2(start_index:end_index));
    xlim(ax1, [x(start_index) x(end_index)]);
    xlim(ax2, [x(start_index) x(end_index)]);
    
    % Update the bottom subplot
    dft_window = y1(start_index:start_index+dft_size-1) .* hann(dft_size)';
    dft = abs(fftshift(fft(dft_window))) / dft_size;
    dft = dft(1:length(f));
    set(dft_handle, 'YData', dft);

    set(fig, 'Color', 'None');

    
    % Save the frame as an image
    frame = getframe(fig);
    im{i} = frame2im(frame);
    
    % Pause for a short time to control the frame rate
    pause(delay_time);
end

% Write the images to a GIF file with transparent background
for i = 1:frame_count
    [A,map] = rgb2ind(im{i},256);
    if i == 1
        imwrite(A,map,filename,'gif','LoopCount',Inf,'DelayTime',delay_time, 'TransparentColor', 0);
    else
        imwrite(A,map,filename,'gif','WriteMode','append','DelayTime',delay_time, 'TransparentColor', 0);
    end
end
