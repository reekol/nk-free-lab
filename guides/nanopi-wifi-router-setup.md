Make a router out of debian


install hostapd bridge-utils isc-dhcp-server bind9 rfkill


/etc/network/interfaces
```
auto wlan0
iface wlan0 inet static
    wireless-mode Master
    address 192.168.2.1
    netmask 255.255.255.0
    
```

/etc/sysctl.conf
```
net.ipv4.ip_forward=1
```

/etc/network/if-pre-up.d/iptables
```
#!/bin/sh
/sbin/iptables-restore < /etc/network/iptables
```
Run:
```
chown root /etc/network/if-pre-up.d/iptables ; chmod 755 /etc/network/if-pre-up.d/iptables
```

/etc/network/iptables
```
*nat
:PREROUTING ACCEPT [0:0]
:INPUT ACCEPT [0:0]
:OUTPUT ACCEPT [0:0]
:POSTROUTING ACCEPT [0:0]
# eth0 is WAN interface
-A POSTROUTING -o eth0 -j MASQUERADE

COMMIT

*filter
:INPUT ACCEPT [0:0]
:FORWARD ACCEPT [0:0]
:OUTPUT ACCEPT [0:0]

# Forward traffic from wlan0 (LAN) to eth0(WAN)
-A FORWARD -i wlan0 -o eth0 -j ACCEPT

# Service rules
-A INPUT -j ACCEPT

# Forwarding rules
-A FORWARD -j ACCEPT

COMMIT
```
/etc/dhcp/dhcpd.conf
```
subnet 192.168.2.0 netmask 255.255.255.0 {
 range 192.168.2.100 192.168.2.199;
 option routers 192.168.2.1;
 option domain-name-servers 192.168.2.1;
 option broadcast-address 192.168.2.255;
}

host laptopY510p {
        hardware ethernet e6:9e:40:4b:2c:b7;
        fixed-address 192.168.2.11;
}
```

/etc/hostapd/hostapd.conf
```
interface=wlxec22800eba39
driver=nl80211
ieee80211n=1
ht_capab=[SHORT-GI-40][HT40+][HT40-][DSSS_CCK-40]
channel=10
ssid=ReeKolNode
hw_mode=g
wpa=2
wpa_passphrase=somepassword
wpa_key_mgmt=WPA-PSK
```

```
hostapd /etc/hostapd/hostapd.conf
update-rc.d hostapd defaults
udevadm control --reload-rule
```


/etc/rc.local
```
systemctl stop NetworkManager
ifdown wlan0
ifup wlan0 
hostapd /etc/hostapd/hostapd.conf -B
```


Or instead all of this, use:
```
nmcli dev wifi hotspot ifname wlp4s0 ssid test password "test1234"
```
