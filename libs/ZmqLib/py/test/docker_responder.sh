#!/bin/bash
python3.6 put_etcd_public_keys.py --public-key ./public_keys/
python3.6 responder_test.py --private-key /run/secrets