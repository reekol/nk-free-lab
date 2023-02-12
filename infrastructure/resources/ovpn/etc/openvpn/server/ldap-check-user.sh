#!/bin/bash

source .env

[[ $(ldapsearch -x \
  -D "uid=$username,${LDAP_VPN_BASEDN}" -w $password \
  -h freeipaserver -p 389 -LLL -s sub \
  -b ${LDAP_VPN_BASEDN} ${LDAP_VPN_FILTER} \
  "dn"  | wc -l ) -gt 0 ]] || {

    echo "Error: Credentials failed for user $username on host $host:$port"
    exit 1
}

exit 0
