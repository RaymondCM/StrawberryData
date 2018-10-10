import os
import shutil
from tqdm import tqdm
import sys
import random


def generate_labelbox_data():
    data_dir = os.path.abspath("../data/")

    if os.path.isdir(data_dir):
        print("Found data directory.")
    else:
        sys.exit("Could not find data directory")

    hostname = 'happy'
    port = 8000
    print("Setting hostname '{}' and port '{}' for flat data set in '{}'".format(hostname, port, data_dir))

    rgb_files_dir = "rgb_files/"
    rgb_files_dir_path = os.path.join(data_dir, rgb_files_dir)
    if os.path.isdir(rgb_files_dir_path):
        print("Found flattened data set directory.")
    else:
        sys.exit("Could not find flattened data set directory")

    print("Using relative file path '{}'".format(rgb_files_dir_path))

    rgb_files = next(os.walk(rgb_files_dir_path))[2]

    print("Found '{}' files".format(len(rgb_files)))

    if len(rgb_files) <= 0:
        sys.exit("Flattened rgb files path contains no files")

    root = os.path.join(data_dir, "label_box/")
    keys = ["810512060234", "810512060307", "810512060222"]
    labels = ["top_" + keys[0], "middle_" + keys[1], "bottom_" + keys[2]]

    group_paths = labels

    for f in group_paths:
        tf = os.path.join(root, f)
        if not os.path.exists(tf):
            os.makedirs(tf)
        else:
            sys.exit("Path already exists, please create something new to avoid potential data loss")

    group_count = len(keys)
    random.shuffle(rgb_files)

    group_csvs = [open(os.path.join(root, x + ".csv"), "w", newline='') for x in labels]
    group_counts = [0, 0, 0]
    all_csv = open(os.path.join(root, "all_cameras.csv"), "w", newline='')

    print("Writing Headers")
    header = "Data URL,Unique ID\n"
    all_csv.write(header)
    for g in group_csvs:
        g.write(header)

    print("Copying and creating data files")
    for f in tqdm(rgb_files):
        matching_key = -1
        for i in range(group_count):
            if keys[i] in f:
                matching_key = i
                break
        if matching_key != -1:
            basename = os.path.basename(f)
            old_path = os.path.join(rgb_files_dir_path, f)
            new_path = os.path.join(group_paths[matching_key], basename)
            url="http://{}:{}/{},{}\n".format(hostname, port, new_path, basename)

            shutil.copyfile(old_path, os.path.join(root, new_path))

            group_csvs[matching_key].write(url)
            all_csv.write(url)
            group_counts[matching_key] += 1

    print(list(zip(labels, group_counts)))

    for c in group_csvs:
        c.close()
    all_csv.close()


if __name__ == '__main__':
    generate_labelbox_data()
