# Office dev infrastructure

# Overview

- Configure behind NAT and forward ports
- tcp:80   (http)    Temporary:   for TLS certificates creation   only
- tcp:443  (https)   Temporary:   for TLS certificates validation only
- udp:1194 (openvpn) Permanently: for VPN access.

# Main Services

- Openvpn ( vpn with dnsmasq for internal dns resolver )
- Traefik (Loadballancer, TLS cert, resolver, lb and inbound trafic manager )
- Freeipaserver (Identity managenent system)
- Nextcloud ( Document Storage, Chats, Calendars, Contacts )
- Colabora CODE service ( Service for colaborative documents editing using open office backend ) 
- Gitlab ( Source and CI/CD )
- Mariadb ( Database )
- Maxscale ( Database Manager and monitoring )
- Grafana ( Genaral monitoring tool ) 
- API ( Laravel based API build around OpenAPI Specification with integrated Swagger sanbox ) 
- Redis ( Centralised user sessions cache storage )
- Portainer ( swarm manager with web gui )

### 1) Openvpn.
[docker-compose.yml#L115](docker-compose.yml#L115)
- Based on Debian image.
- The entire infrastructure sits inside a private network.
- All of the other services can be accessed only from inside it.
- Connected to LDAP (FreeIPA), allows users from _employee_vpn group to login. [resources/ovpn/etc/openvpn/server](resources/ovpn/etc/openvpn/server)

```
Network segmentation based on user roles (configured in a docker-compose.yml)
```

### 2) Traefik

- Generates, maintains, resolves certificates and URI-s from inside of VPN.
- Keep in mind that while it generates valid certificates for each of the services of the cluster, thay are not reachable from outside of it.
- While form inside of the vpn, their TLS certificates are valid also while accessing them from private network addresses!

### 3) FreeIPA - Server.

[config/setup.sh#L3](config/setup.sh#L3)

Identity management system that stores all the Users and groups.
Initial setup is located in ./config/setup.sh script with ./config/.env.dev variables

Basic users and groups:

```
LDAP_GROUP_SERVICES="${PREFIX}_services"        # Services Bind users (are/should be) part of this Group.

LDAP_USER_SERVICE_APACHE=service.apache         # Bind user for Api        service to check for credentials 
LDAP_USER_SERVICE_GITLAB=service.gitlab         # Bind user for Gitlab     service to check for credentials 
LDAP_USER_SERVICE_CLOUD=service.cloud           # Bind user for Nextcloud  service to check for credentials 
LDAP_USER_SERVICE_GRAFANA=service.grafana       # Bind user for Grafana    service to check for credentials 
LDAP_USER_SERVICE_VPN=service.vpn               # Bind user for VPN        service to check for credentials 

LDAP_GROUP_API_READ="${PREFIX}_api_read"        # Group for users with READ  access to Api Service
LDAP_GROUP_API_WRITE="${PREFIX}_api_write"      # Group for users with WRITE access to Api Service
LDAP_GROUP_CLOUD="${PREFIX}_employee_cloud"     # Group for users with access to Nextcoud  Service
LDAP_GROUP_GITLAB="${PREFIX}_employee_gitlab"   # Group for users with access to Gitlab    Service
LDAP_GROUP_GRAFANA="${PREFIX}_employee_grafana" # Group for users with access to Grafana   Service
LDAP_GROUP_VPN="${PREFIX}_employee_vpn"         # Group for users with access to VPN       Service *NOTE All other services are in a VPNetwork! 

```
- FreeIPA can be accessed by administrators on freeipa server.$(FQDN:-example.com}. Only users from GROUP ipausers, can log in.

### 4) Nextcloud
 [config/setup.sh#L38](config/setup.sh#L38)
  ISO complient, selfhosted cloud for storing documents, calendars, contacts and handling internal company's communications.
  Login using ldap (FreeIPA) credentions, for acive users in _employee_cloud group. [config/.env.dev#L52](config/.env.dev#L52)

  Implements CODE (Colabora) services for editing documents.
  Open source Apps available for: Linux, Android, M$, Apple, Iphone.
  
### 5) Colabora
  It is used as a service by Nextcloud to edit documents online.
  
### 6) Gitlab:
   We all know what this is, do we :)
   CI/CD Platform. 
   Login using LDAP Credentilas for acive users in _employee_gitlab group. [config/.env.dev#L46](config/.env.dev#L46)
   
### 7) Mariadb
   Main database storage for this infrastructure.
   Contains Nextcloud and API settings.
   
### 8) Maxscale
   Monitoring and database adminisration tool by mariadb project.
  
### 9) Grafana
   Basic monitoring visualisation tool.
   (Kibana + Elastic should be added to )
   Credentials: Ldap for acive users in _employee_grafana group.

### 10) Api
  - API may be used for automations, reporting, develeopement, etc.
  - Added basic Api model builder, complient with OpenAPI v3 specification. [resources/project](resources/project)
  - Added atomatic doocumentation builder + Sandbox ( Swagger ).
  - Utilises 2 auth profiles. [config/.env.dev#L61](config/.env.dev#L61)
  - + _Read  profile: accessible for users from _api_read group. 
  - + _Write profile: accessible for users from _api_write group.
  - Authenticate using basic auth behind apache2's ldap plugin

### 11) Redis
  - Stores the sessiona from Gitlab, Nextcloud, Api.
  - Clearing them from here, will immediately invalidate active user session from corresponding services.
  
### 11.1) Redisinsights
  WEB Ui manager for Redis from redis  project.
 
### 12 ) Portainer
  Manage, log, stats, cli, for containers in this swarm
  
#  TODO:
 Add Elastic and Kibana.
 Lock containers versions.
