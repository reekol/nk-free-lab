source .env.dev

CONTAINER=${PREFIX}_vpn

openvpn_setup_vpn () {

  docker container exec ${CONTAINER} /bin/sh -c "
    apk add bind-tools iptables openssl openvpn openvpn-auth-ldap easy-rsa

    mkdir -p /dev/net
    mknod /dev/net/tun c 10 200

    /usr/share/easy-rsa/easyrsa --batch init-pki
    /usr/share/easy-rsa/easyrsa --batch build-ca nopass
    /usr/share/easy-rsa/easyrsa --batch gen-dh
    /usr/share/easy-rsa/easyrsa --batch build-server-full  vpn.${DOMAIN}   nopass

    iptables -A INPUT   -i eth0 -m state --state NEW -p udp --dport 1194 -j ACCEPT
    iptables -A INPUT   -i tun+ -j ACCEPT
    iptables -A FORWARD -i tun+ -j ACCEPT
    iptables -A FORWARD -i tun+ -o eth0 -m state --state RELATED,ESTABLISHED -j ACCEPT
    iptables -A FORWARD -i eth0 -o tun+ -m state --state RELATED,ESTABLISHED -j ACCEPT
    iptables -t nat -A POSTROUTING -s 10.8.0.0/24 -o eth0 -j MASQUERADE
    iptables -A OUTPUT -o tun+ -j ACCEPT
    "

}

openvpn_start_vpn () {
  docker container exec ${CONTAINER} /bin/sh -c "
    openvpn  --dev tun0 --daemon
    sleep 1
    ifconfig tun0 up"
}

openvpn_setup_client () {

  local MY_IP=$(dig +short ${DOMAIN})

  docker container exec ${CONTAINER} /bin/sh -c "/usr/share/easy-rsa/easyrsa --batch build-client-full  ${1}  nopass"

  echo "################ START ${1}.ovpn ################"
  echo "client"
  echo "dev tun"
  echo "proto udp"
  echo "remote ${MY_IP} 1194"
  echo "resolv-retry infinite"
  echo "nobind"
  echo "persist-key"
  echo "persist-tun"
  echo "remote-cert-tls server"
  echo "auth SHA512"
  echo "cipher AES-256-CBC"
  echo "ignore-unknown-option block-outside-dns"
  echo "block-outside-dns"
  echo "verb 3"
  echo "<dh>"
  docker container exec ${CONTAINER} /bin/sh -c "cat /pki/dh.pem"
  echo "</dh>"
  echo "<ca>"
  docker container exec ${CONTAINER} /bin/sh -c "cat /pki/ca.crt"
  echo "</ca>"
  echo "<cert>"
  docker container exec ${CONTAINER} /bin/sh -c "cat /pki/issued/${1}.crt"
  echo "</cert>"
  echo "<key>"
  docker container exec ${CONTAINER} /bin/sh -c "cat /pki/private/${1}.key"
  echo "</key>"
  echo "################ END ${1}.ovpn ################"

}

openvpn_setup_vpn
openvpn_setup_client ${MASTER_USER}
openvpn_setup_client ${DEMO_USER}
openvpn_start_vpn
