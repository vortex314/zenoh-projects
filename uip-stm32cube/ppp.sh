sudo pppd /dev/ttyUSB1 115200 noauth local debug dump logfile /var/log/ppp.log
sudo pppd /dev/ttyUSB1 115200 noauth local debug dump nodetach

sudo pppd /dev/ttyUSB1 115200 noauth local debug dump nodetach mtu 1024 nocrtscts