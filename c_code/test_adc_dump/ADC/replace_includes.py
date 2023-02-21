import os
import re

conflicts = []
def update_includes(root_dir=os.getcwd()):
    '''Go through the file structure recursively and change filenames in #include statements to be relative filepaths instead of simply filenames. For example, #include "filename.h" should be #include "../filename.h", if filename.h is in the parent directory. For another example, if the file tree is:
    outer_dir/
        inner_dir1/
            foo.h
        inner_dir2/
            blah.h

    In this file tree, if blah.h has a statement: `#include "foo.h"` , change line to `#include "../inner_dir1/foo.h"`.
    So, for each #include statement followed by a filename, search for the actual location of that file and fix the filepath in the include.
    First, collect all the #include statements in each file and store them in a list. Then, write all the file dirs into file_dirs by doing an os.walk(). Finally, loop through the list of #include statements and update them with the correct relative paths.
    '''

    # A dictionary to keep track of the directory of each file
    file_dirs = {}

    # A list to store all the #include statements in each file
    include_statements = []

    # Walk through the directory recursively
    for dirpath, dirnames, filenames in os.walk(root_dir):
        for filename in filenames:
            # Check if the file has a .h or .cpp extension
            if filename.endswith(".h") or filename.endswith(".cpp") or filename.endswith(".c"):
                filepath = os.path.join(dirpath, filename)


                if filename in file_dirs:
                    print(f'conflict! {filename}, {filepath}')
                    conflicts.append(filename)
                # Add the directory of the file to the dictionary
                file_dirs[filename] = dirpath

                # Read the contents of the file and collect the #include statements
                with open(filepath, "r") as f:
                    contents = f.readlines()
                    for line in contents:
                        if line.lstrip().startswith('#include "'):
                            include_statements.append((filepath, line))

    # Loop through the list of #include statements and update them with the correct relative paths
    for filepath, line in include_statements:
        # Get the filename from the line
        match = re.search(r'"(.+)"', line)

        # killme = False
        if 'port/cpu/mimxrt/fnet_mimxrt1062_config.h' in line:
            print('found!!')
            killme = True
        if match:
            filename = match.group(1)
            if '/' in filename:
                filename = filename.split('/')[-1]

            # Check if the filename is in the dictionary
            if filename in file_dirs:
                if filename in conflicts:
                    raise Exception("Filename conflict: " + filename)
                # Get the directory of the file being included
                include_dir = file_dirs[filename]

                # Get the relative path from the file's directory to the include file's directory
                relpath = os.path.relpath(include_dir, start=os.path.dirname(filepath))

                # Modify the line to use the relative path
                match = re.search(r'"(.+)"', line)
                filename_or_naive_relpath = match.group(1)
                new_line = line.replace(filename_or_naive_relpath, os.path.join(relpath, filename).replace('\\', '/'))
                # Replace the old line with the modified line in the file
                with open(filepath, "r") as f:
                    contents = f.readlines()
                with open(filepath, "w") as f:
                    for l in contents:
                        if l != line:
                            f.write(l)
                        else:
                            f.write(new_line)
                print(f"Updated line in {filepath}: {line.strip()} -> {new_line.strip()}")
            else:
                print(f"Could not find {filename} in {file_dirs}!")
 

    print("Done updating includes.")

update_includes(os.getcwd())
