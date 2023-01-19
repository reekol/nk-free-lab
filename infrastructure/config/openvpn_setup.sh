source .env.dev

CONTAINER=${PREFIX}_vpn

openvpn_install () {
 docker container exec ${CONTAINER} /bin/bash -c "
    mkdir -p /dev/net
    mknod /dev/net/tun c 10 200
    apt update && apt install -y wget iproute2 systemd openvpn easy-rsa
    wget https://git.io/vpn -O openvpn-install.sh
    chmod +x openvpn-install.sh
  "
}
openvpn_install

