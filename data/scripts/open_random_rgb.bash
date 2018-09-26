#!/bin/bash

#Change to script directory
cd $(cd -P -- "$(dirname -- "$0")" && pwd -P)

#Change to data root directory
cd ..

echo -n "Enter number of random RGB files to open: "
read user_input

#Get all RGB files as array
rgb_files=()
while IFS=  read -r -d $'\0'; do
  rgb_files+=("$REPLY")
done < <(find . -type f -iname "rgb_8UC3.png" -print0)

#Display random subset of images
if [[ $user_input -lt 0 || $user_input -gt ${#rgb_files[@]} ]]
 then
  echo "Input is outside acceptable range (0-${#rgb_files[@]})"
else 
  #Generate array of random indecies
  shuffle_idx=($(shuf -i 0-${#rgb_files[@]} -n $user_input))

  for ((i = 0; i < ${user_input}; i++))
  do
    k=${shuffle_idx[$i]}
    file=${rgb_files[$k]}
    echo "Opening: ${file}"
    gnome-open "${file}" > /dev/null 2>&1 &
  done

fi
