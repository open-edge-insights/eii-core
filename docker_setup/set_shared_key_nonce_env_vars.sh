#!/bin/bash

key_nonce=$(python3 generate_key_nonce.py)
key=`echo $key_nonce | awk -F, '{print $1}'`
nonce=`echo $key_nonce | awk -F, '{print $2}'`

export SHARED_KEY=$key
export SHARED_NONCE=$nonce