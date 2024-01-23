#!/bin/sh

set -e

parent_path=$(dirname "$0")/../
cd $parent_path

# Process flags
debug_build=false
while getopts :d opt; do
    case $opt in
        d)
            debug_build=true
            ;;
        ?)
            echo "Invalid option: -${OPTARG}"
            exit 1
            ;;
    esac
done

if [[ $debug = true ]]
then
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
else
    cmake -S . -B build
fi

cmake --build build