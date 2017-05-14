#!/usr/bin/env python3

import os
import sys

with open(sys.argv[1], 'wb') as outFile:
    for i in range(30):
        outFile.write((i).to_bytes(4, byteorder='little', signed=True))
        outFile.write(os.urandom(1020))

