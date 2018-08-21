#!/bin/bash

echo "1. Starting classifier.py, Running Kapacitor and Enabling the Classifier Task..."
python3.6 classifier_startup.py --config `echo $1` --log-dir /ETA/classifier_logs