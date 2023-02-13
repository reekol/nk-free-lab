# Office dev infrastructure

# Stack

- Openvpn ( vpn with dnsmasq for internal dns resolver )
- Traefik (Loadballancer, TLS cert, resolver, lb and inbound trafic manager )
- Freeipaserver (Identity managenent system)
- Nextcloud ( Document Storage, Chats, Calendars, Contacts )
- Gitlab ( Source and CI/CD )
- Mariadb ( Database )
- Maxscale ( Database Manager and monitoring )
- Grafana ( Genaral monitoring tool ) 
- API ( Laravel based API build around OpenAPI Specification with integrated Swagger sanbox ) 
- Redis ( Centralised user sessions cache storage )

### 1) Openvpn.

The entyre infrastructure sits inside of private network. 
All of the other services can beaccessed only from inside of it.
Connected to LDAP (FreeIPA), allows users from  _employee_vpn  group to login.

### 2) Traefik

Generates, maintains, resolves certificates and URI-s from inside of VPN.
Keep in mind that while it generates valid certificates for each of the cervices of the cluster, thay are not reachable from outside of it.
While form inside of the vpn, their TLS certificates are valid!

### 3) FreeIPA - Server.

Identity management system that stores all the Users and groups.
Initial setup is located in ./config/setup.sh script

Basic users and groups:

```
LDAP_GROUP_SERVICES="${PREFIX}_services"      # Services Bind users (are/should be) part of this Group.

LDAP_USER_SERVICE_APACHE=service.apache       # Bind user for Api        service to check for credentials 
LDAP_USER_SERVICE_GITLAB=service.gitlab       # Bind user for Gitlab     service to check for credentials 
LDAP_USER_SERVICE_CLOUD=service.cloud         # Bind user for Nextcloud  service to check for credentials 
LDAP_USER_SERVICE_GRAFANA=service.grafana     # Bind user for Grafana    service to check for credentials 
LDAP_USER_SERVICE_VPN=service.vpn             # Bind user for VPN        service to check for credentials 

LDAP_GROUP_API_READ="${PREFIX}_api_read"        # Group for users with READ  access to Api Service
LDAP_GROUP_API_WRITE="${PREFIX}_api_write"      # Group for users with WRITE access to Api Service
LDAP_GROUP_CLOUD="${PREFIX}_employee_cloud"     # Group for users with access to Nextcoud  Service
LDAP_GROUP_GITLAB="${PREFIX}_employee_gitlab"   # Group for users with access to Gitlab    Service
LDAP_GROUP_GRAFANA="${PREFIX}_employee_grafana" # Group for users with access to Grafana   Service
LDAP_GROUP_VPN="${PREFIX}_employee_vpn"         # Group for users with access to VPN       Service *NOTE All other services are in a VPNetwork! 

```
