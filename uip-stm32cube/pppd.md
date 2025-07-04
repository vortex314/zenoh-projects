#!/bin/bash

# PPP daemon setup for ttyUSB1 with debug on Linux

# Basic command with debug
sudo pppd /dev/ttyUSB1 115200 debug dump logfile /var/log/ppp.log

# More comprehensive command with common options
sudo pppd /dev/ttyUSB1 115200 \
    debug \
    dump \
    logfile /var/log/ppp.log \
    noauth \
    local \
    lock \
    crtscts \
    defaultroute \
    persist \
    maxfail 0

# Alternative with specific IP addresses
sudo pppd /dev/ttyUSB1 115200 \
    debug \
    dump \
    logfile /var/log/ppp.log \
    noauth \
    local \
    lock \
    crtscts \
    10.0.0.1:10.0.0.2 \
    persist

# For dial-up or modem connections
sudo pppd /dev/ttyUSB1 115200 \
    debug \
    dump \
    logfile /var/log/ppp.log \
    connect 'chat -v -f /etc/ppp/chatscript' \
    noauth \
    defaultroute \
    persist

# Minimal debug command
sudo pppd /dev/ttyUSB1 115200 debug

# Check PPP interface status
ip addr show ppp0

# Monitor PPP logs in real-time
sudo tail -f /var/log/ppp.log

# Kill PPP daemon
sudo pkill pppd
Here's how to start `pppd` on Linux for `/dev/ttyUSB1` with debug output:**Key options explained:**

**Debug Options:**
- `debug` - Enable debug logging
- `dump` - Log all control packets (LCP, IPCP, etc.)
- `logfile /var/log/ppp.log` - Specify log file location

**Connection Options:**
- `/dev/ttyUSB1` - Serial device
- `115200` - Baud rate
- `noauth` - Don't require authentication
- `local` - Don't use modem control lines
- `lock` - Create lock file to prevent other processes from using the port
- `crtscts` - Use hardware flow control

**Network Options:**
- `defaultroute` - Make PPP connection the default route
- `persist` - Restart connection if it fails
- `maxfail 0` - Retry indefinitely
- `10.0.0.1:10.0.0.2` - Local:Remote IP addresses

**Quick start command:**
```bash
sudo pppd /dev/ttyUSB1 115200 debug dump logfile /var/log/ppp.log noauth local
```

**Monitor the connection:**
```bash
# Watch logs
sudo tail -f /var/log/ppp.log

# Check interface
ip addr show ppp0

# Check routing
ip route show
```

**Troubleshooting:**
- Ensure user has permission to access `/dev/ttyUSB1`
- Check if device exists: `ls -l /dev/ttyUSB1`
- Verify baud rate matches your device
- Use `sudo dmesg | grep ttyUSB` to check device detection

The debug output will show LCP negotiation, IPCP configuration, and all PPP protocol exchanges, which is helpful for troubleshooting connectivity issues.