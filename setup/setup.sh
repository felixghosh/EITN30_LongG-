#!/bin/bash
  git config --global user.email "zanzaniman@gmail.com"
  git config --global user.name "Felix Ghosh"
  wget http://tmrh20.github.io/RF24Installer/RPi/install.sh
  chmod +x install.sh
  ./install.sh
  cd rf24libs/RF24/examples_linux