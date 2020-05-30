## Example below is applicable to nanopi-neo-air with usb-otg or sbc with debian ( or armbian )
 ( or any other sbc with otg capable interface )

### Configuring interfaces:
/etc/network/interfaces
```
source /etc/network/interfaces.d/*
# Network is managed by Network manager

auto lo
iface lo inet loopback

auto wlan0
iface wlan0 inet dhcp
        wpa-ssid my-wifi
        wpa-psk myWifisLongEncodedPassword


# The internal LAN interface (usb0)
allow-hotplug usb0
iface usb0 inet static
    address 10.0.0.1
    netmask 255.255.255.0
    network 10.0.0.0
    broadcast 10.0.0.255
```

### Configure usb-otg
```bash
rmmod g_serial
modprobe g_ether
```

### Make otg configuration persistent
/etc/modules
```
#...
#g_serial
g_ether
```
### Install prerequisite

```bash
apt-get install dnsmasq iptables iptables-persistent
```

### Configure dnsmasq

/etc/dnsmsq.conf

```
#...
interface=usb0
listen-address=127.0.0.1
dhcp-range=10.0.0.100,10.0.0.110,12h
```

### Configure iptables
/etc/iptables/rules.v4
```
# ...
*nat
-A POSTROUTING -o wlan0 -j MASQUERADE
COMMIT

*filter
-A INPUT -i lo -j ACCEPT

# allow ssh, so that we do not lock ourselves
-A INPUT -i wlan0 -p tcp -m tcp --dport 22 -j ACCEPT
-A INPUT -i wlan0 -p tcp -m tcp --dport 443 -j ACCEPT

# allow incoming traffic to the outgoing connections
-A INPUT -m state --state RELATED,ESTABLISHED -j ACCEPT

# prohibit everything else incoming
-A INPUT -i wlan0 -j DROP
COMMIT
```

### Load and make iptables changes persistent
```bash
iptables-restore < /etc/iptables/rules.v4
```

### Done
Now your nanopi will be detected as usb lan interface when connected
to usb port of any computer and will autoconfigure the interface.

