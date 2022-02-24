#!/bin/bash
#script for pushing changes to git and wiping local files from the RPs
git pull
git add .
git commit -m "auto commit before cleaning"
git push
sudo ip link delete lg0
cd ..
rm -rf EITN30_LongG-