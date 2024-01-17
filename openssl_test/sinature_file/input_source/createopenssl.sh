#!/bin/bash
#openssl genrsa -3 -out rsa_private_key.pem 2048
openssl genrsa -out rsa_private_key.pem 2048
openssl rsa -in rsa_private_key.pem -out rsa_public_key.pem -pubout
openssl rsa -in rsa_private_key.pem  -noout -text
