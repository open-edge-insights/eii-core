#!/bin/bash

touch classifier.log

# Insert camera location
python3.6 factory.py db insert cam-loc 0 0 0 
# Insert camera position
python3.6 factory.py db insert cam-pos 0 0 0 
# Insert camera
python3.6 factory.py db insert camera camera-serial-number 1 1 

# Starting classifier
python3 classifier.py factory.json 