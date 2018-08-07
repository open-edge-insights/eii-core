#!/bin/bash

touch classifier.log

# Starting classifier
python3.6 classifier.py --config `echo $1` --log-dir /ETA/classifier_logs