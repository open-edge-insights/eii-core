#!/bin/bash
python3.6 put_etcd_public_keys.py --public-key ./public_keys/
python3.6 publisher_test.py --private-key /run/secrets