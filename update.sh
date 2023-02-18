#!/bin/bash
make clean
git add * && git add -u * . && git commit -m "Update" && git push