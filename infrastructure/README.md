# Office dev infrastructure

# Stack

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

### 1) Openvpn.
Based on Debian image.
The entyre infrastructure sits inside of private network. 
All of the other services can beaccessed only from inside of it.
Connected to LDAP (FreeIPA), allows users from  _employee_vpn  group to login.

```
Network segmentation based on user roles (configured in a docker-compose.yml)
```

### 2) Traefik

Generates, maintains, resolves certificates and URI-s from inside of VPN.
Keep in mind that while it generates valid certificates for each of the cervices of the cluster, thay are not reachable from outside of it.
While form inside of the vpn, their TLS certificates are valid while resolving them from private network addresses!

### 3) FreeIPA - Server.

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
FreeIPA can be accessed by admins on freeipaserver.$(FQDN:-example.com}. Only users from GROUP ipausers, can log in.

### 4) Nextcloud
  ISO complient cloud for storing documents, calendars, contacts and handling internal company's communications.
  Login using ldap (FreeIPA) credentions, for acive users in _employee_cloud group.
  
  Implements CODE (Colabora) services for editing documents.
  Open osurce Apps available for: Linux, Android, M$, Apple, Iphone.
  
### 5) Colabora
  This service is not, separetly used. It is used as a service by Nextcloud to edit documents online.
  
### 6) Gitlab:
   We all know what this is ... do we :) 
   CI/CD Platform. 
   Login using LDAP Credentilas for acive users in _employee_gitlab group.
   
### 7) Mariadb
   Main database storate for this infrastructure.
   Contains Nextcloud and API settings.
   
### 8) Maxscale
   Monitoring and database adminisration tool by mariadb project.
  
### 9) Grafana
   Basic monitoring visualisation tool.
   *(Kibana + Elastic shoulld be added to )
   Credentials: Ldap for acive users in _employee_grafana group.
   
### 10) Api
  Api may be used for automations, reporting, develeopement, etc. 
  Added basic Api model builder, complient with OpenAIP v3 specification.
  Added Atomatic doocumentation builder + Sandbox ( Swagger ).
  Utilises 2 auth profiles.
  _Read  profile: accessible for users from _api_read group.
  _Write profile: accessible for users from _api_write group.
  
### 11) Redis
  Stores the sessiona from Gitlab, Nextcloud, Api. 
  Clearen them from here, will immediatly invalidate active user seession from corresponding services. 
  Should be usebmosly for automations. 
  
### 11.1) Redisinsights
  WEB Ui manager for redis.
 
#  TODO:
 Add Elastic and Kibana.
 Lock containers versions.
