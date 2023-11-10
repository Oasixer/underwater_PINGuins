import os

filename = './blah'
# Check if the file exists
if os.path.isfile(filename):
    # Get the size of the file
    file_size = os.path.getsize(filename)
    # Assert that the file size is a multiple of 200 bytes
    assert file_size % 200 == 0, "File size is not a multiple of 200 bytes"
    print(file_size)
    # Open the file and read the last 200 bytes
    with open(filename, 'rb') as f:
        f.seek(-200, os.SEEK_END)
        last_200_bytes = f.read()
    print(last_200_bytes)
else:
    print("File doesn't exist")
