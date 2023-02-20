function animateSlidingDFT(signal, windowSize, stepSize, fs, frameDuration, outputFilename)
% signal: the input signal as a vector
% windowSize: the size of the sliding window (in samples)
% stepSize: the step size between windows (in samples)
% fs: the sampling rate of the signal
% frameDuration: the duration of each frame
% outputFilename: the filename of the output video file

% Compute the number of steps needed to cover the entire signal
numSteps = floor((length(signal)-windowSize)/stepSize) + 1;

% Preallocate the DFT matrix and the x-axis vector
DFT = zeros(windowSize, numSteps);
f = linspace(0, fs/2, windowSize/2+1);

% create a new figure
figure

% Create a VideoWriter object to write the output to a video file
v = VideoWriter(outputFilename);
open(v);

% Loop over each window and compute the DFT
for i = 1:numSteps
    % Extract the current window
    window = signal((i-1)*stepSize+1:(i-1)*stepSize+windowSize);
    
    % Compute the DFT and store it in the DFT matrix
    DFT(:,i) = fft(window);
    
    % Plot the current DFT as a bar graph
    bar(f, 2*abs(DFT(1:windowSize/2+1,i))/windowSize);
    axis([0 fs/2 0 max(abs(DFT(:)))/windowSize]);
    xlabel('Frequency (Hz)');
    ylabel('Magnitude');
    title(sprintf('Sliding DFT at time %.4f', (i-1)*stepSize/fs));

    % Add the current frame to the video
    frame = getframe(gcf);
    writeVideo(v,frame);
    
    % Pause to create the animation effect
    pause(frameDuration);
end
% Close the video file
close(v);

end
