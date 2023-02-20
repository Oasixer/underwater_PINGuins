function data = readUint16File(filename)
% filename: the name of the file to read
% data: a vector containing the uint16 data from the file

% Open the file for reading
fileID = fopen(filename, 'r');

% Read the data as a column vector of uint16 values
data = fread(fileID, Inf, 'uint16');

% Close the file
fclose(fileID);

end
