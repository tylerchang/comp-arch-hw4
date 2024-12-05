#!/bin/bash

# Redirect all output to log file
exec 1>/tmp/startup-script.log 2>&1

echo "==== Starting startup script at $(date) ===="

# Force IPv4
echo 'Acquire::ForceIPv4 "true";' | sudo tee /etc/apt/apt.conf.d/99force-ipv4
echo "Added IPv4 force configuration"

# Function to check and log command status
run_command() {
    echo "Running command: $@"
    if ! "$@"; then
        echo "ERROR: Command failed: $@"
        exit 1
    fi
    echo "Command completed successfully: $@"
}

# Install required packages
echo "==== Installing packages ===="
run_command apt-get update
run_command apt-get install -y build-essential git

# Create and verify working directory
echo "==== Setting up working directory ===="
run_command mkdir -p /mergesort
cd /mergesort || exit 1
echo "Current directory: $(pwd)"

# Download and verify input files
echo "==== Downloading input files ===="
run_command gsutil ls gs://homework4-data/inputs/
run_command gsutil cp gs://homework4-data/inputs/* .
echo "Files in working directory:"
ls -la

# Compile program
echo "==== Compiling program ===="
run_command gcc -O3 -o mergesort mergesort.c -pthread 
echo "Compilation completed. Executable details:"
ls -l mergesort

# Run program
echo "==== Running mergesort program ===="
RESULT_FILE="results_$(hostname)_$(date +%Y%m%d_%H%M%S).txt"
run_command ./mergesort > "${RESULT_FILE}"
echo "Program execution completed. Results file:"
ls -l "${RESULT_FILE}"

# Upload results
echo "==== Uploading results ===="
run_command gsutil cp "${RESULT_FILE}" gs://homework4-data/results/mergesort/
echo "Results uploaded successfully"

echo "==== Startup script completed at $(date) ===="
