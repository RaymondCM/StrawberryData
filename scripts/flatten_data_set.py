import os
import sys
from glob import glob, iglob
import shutil
from tqdm import tqdm


def flatten_data_set():
    data_dir = os.path.abspath("../data/")

    if os.path.isdir(data_dir):
        print("Found data directory.")
    else:
        sys.exit("Could not find data directory")

    project_folders = [os.path.abspath(os.path.join(data_dir, x)) for x in next(os.walk(data_dir))[1] if "flat" not in
                       x]
    print("Found these project directories (Ignoring any with 'flat' in the name to avoid cyclic runs):"
          "\n\t{}".format(',\n\t'.join(project_folders)))

    if not input("Continue [y/n]? ").lower() in ("yes", "y"):
        sys.exit("You did not answer yes, will not continue.")

    rgb_file_list = []
    bytes_required = 0
    b2mb = 1e-6
    for project_directory in project_folders:
        for file in iglob(project_directory + "/**/rgb_8UC3.png", recursive=True):
            rgb_file_list.append(os.path.normpath(file))
            bytes_required += os.path.getsize(file)

    print("Found {} files, total size {:,.0f}MB".format(len(rgb_file_list), bytes_required * b2mb))

    bytes_available = shutil.disk_usage("/").free
    safety_net = 50 / 1e-6
    if bytes_required > bytes_available - safety_net:
        sys.exit("File size required {:,.0f}MB, you have {:,.0f}MB available.".format(
            bytes_required * b2mb,
            bytes_available * b2mb
        ))

    flatten_dir = input("Enter the name for the new directory: ")
    flatten_dir = os.path.join(data_dir, flatten_dir)

    if not input("Is {} okay [y/n]? ".format(flatten_dir)).lower() in ("yes", "y"):
        sys.exit("You did not answer yes, will not continue.")

    if not os.path.exists(flatten_dir):
        os.makedirs(flatten_dir)
    else:
        sys.exit("Path already exists, please create something new to avoid potential data loss")

    print("Saving resulting files to {}".format(flatten_dir))

    length_to_remove = len(os.path.abspath(data_dir)) + 1
    for file in tqdm(rgb_file_list):
        new_file_name = '='.join(file[length_to_remove:].split(os.sep))
        new_file_path = os.path.join(flatten_dir, new_file_name)
        shutil.copyfile(file, new_file_path)


def expand_data_set():
    original_data_dir = os.path.abspath("../data/")
    flat_data_dir = os.path.abspath("../data/flat_rgb")

    if os.path.isdir(original_data_dir):
        print("Found original data directory.")
    else:
        sys.exit("Could not find original data directory")

    if os.path.isdir(flat_data_dir):
        print("Found data directory.")
    else:
        sys.exit("Could not find data directory")

    files = glob(flat_data_dir + "/*.png", recursive=False)

    length_to_remove = len(os.path.abspath(flat_data_dir)) + 1

    for file in tqdm(files):
        expanded_file = os.path.join(original_data_dir, os.sep.join(file.split("="))[length_to_remove:])
        shutil.copyfile(file, expanded_file)


if __name__ == '__main__':
    flatten_data_set()
