#!/bin/bash

kapacitor/kapacitor define classifier_task -tick classifier.tick
kapacitor/kapacitor enable classifier_task
