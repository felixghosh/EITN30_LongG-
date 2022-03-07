
#!/bin/bash

# forward tun0 -> eth0 
sudo iptables -A FORWARD -i lg0 -o eth0 -j ACCEPT

# forward eth0 -> tun0
sudo iptables -A FORWARD -i eth0 -o lg0 -m state --state RELATED,ESTABLISHED -j ACCEPT

# Setup masquerade
sudo iptables -t nat -A POSTROUTING -o eth0 -j MASQUERADE