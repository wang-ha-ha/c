#!/bin/bash
#openssl genrsa -3 -out rsa_private_key.pem 2048
openssl genrsa -out input_source/rsa_private_key.pem 2048
openssl rsa -in input_source/rsa_private_key.pem -out input_source/rsa_public_key.pem -pubout
openssl rsa -in input_source/rsa_private_key.pem  -noout -text
