source .env.dev

setup_freeipa () {
    docker container exec ${PREFIX}_freeipaserver /bin/bash -c "echo ${FREEIPASERVER_ADMIN_PASSWORD} | kinit"
    docker container exec ${PREFIX}_freeipaserver /bin/bash -c "
        ipa  group-add ${LDAP_GROUP_DEFAULT}    --gid 1
        ipa  group-add ${LDAP_GROUP_SERVICES}   --gid 2
        ipa  group-add ${LDAP_GROUP_CLOUD}      --gid 3
        ipa  group-add ${LDAP_GROUP_GITLAB}     --gid 4
        ipa  group-add ${LDAP_GROUP_API_WRITE}  --gid 5
        ipa  group-add ${LDAP_GROUP_API_READ}   --gid 6
        ipa  group-add ${LDAP_GROUP_GRAFANA}    --gid 7
        ipa  group-add ${LDAP_GROUP_VPN}        --gid 8

        echo ${LDAP_BIND_PASSWORD} | ipa user-add ${LDAP_USER_SERVICE_GITLAB}   --first=Bind --last=Gitlab  --password --gidnumber=2 --noprivate
        echo ${LDAP_BIND_PASSWORD} | ipa user-add ${LDAP_USER_SERVICE_CLOUD}    --first=Bind --last=Cloud   --password --gidnumber=2 --noprivate
        echo ${LDAP_BIND_PASSWORD} | ipa user-add ${LDAP_USER_SERVICE_APACHE}   --first=Bind --last=Apache  --password --gidnumber=2 --noprivate
        echo ${LDAP_BIND_PASSWORD} | ipa user-add ${LDAP_USER_SERVICE_GRAFANA}  --first=Bind --last=Grafana --password --gidnumber=2 --noprivate
        echo ${LDAP_BIND_PASSWORD} | ipa user-add ${LDAP_USER_SERVICE_VPN}      --first=Bind --last=vpn     --password --gidnumber=2 --noprivate

        ipa group-add-member ${LDAP_GROUP_SERVICES} \
            --users=${LDAP_USER_SERVICE_GITLAB}     \
            --users=${LDAP_USER_SERVICE_CLOUD}      \
            --users=${LDAP_USER_SERVICE_GRAFANA}    \
            --users=${LDAP_USER_SERVICE_APACHE}     \
            --users=${LDAP_USER_SERVICE_VPN}

       echo ${DEMO_PASS} | ipa user-add ${DEMO_USER} --first=Demo --last=User  --password --gidnumber=2 --noprivate
       ipa group-add-member ${LDAP_GROUP_CLOUD}     --users=${DEMO_USER}
       ipa group-add-member ${LDAP_GROUP_GITLAB}    --users=${DEMO_USER}
       ipa group-add-member ${LDAP_GROUP_API_WRITE} --users=${DEMO_USER}
       ipa group-add-member ${LDAP_GROUP_API_READ}  --users=${DEMO_USER}
       ipa group-add-member ${LDAP_GROUP_GRAFANA}   --users=${DEMO_USER}
       ipa group-add-member ${LDAP_GROUP_VPN}       --users=${DEMO_USER}
    "
}

setup_cloud () {

    docker container exec ${PREFIX}_mariadb mysql \
        -u${NEXTCLOUD_DB_USER} \
        -p${NEXTCLOUD_DB_PASSWORD} \
        -e "DROP DATABASE IF EXISTS cloud"

    docker container exec ${PREFIX}_cloud /bin/bash -c "
        apt-get update && apt-get -y install sudo vim

        sudo -u www-data php -d memory_limit=512M occ maintenance:install \\
        --database='mysql' \\
        --database-name='${NEXTCLOUD_DB_NAME}' \\
        --database-host='mariadb' \\
        --database-user='${NEXTCLOUD_DB_USER}' \\
        --database-pass='${NEXTCLOUD_DB_PASSWORD}' \\
        --admin-user='${NEXTCLOUD_ADMIN_USER}' \\
        --admin-pass='${NEXTCLOUD_ADMIN_PASSWORD}'

        sudo -u www-data php -d memory_limit=512M occ config:system:set overwriteprotocol  --value='https'
        sudo -u www-data php -d memory_limit=512M occ config:system:set datadirectory --value='/var/www/html/data'
        sudo -u www-data php -d memory_limit=512M occ config:system:set overwrite.cli.url --value='http://h.cloud.vpn'
        sudo -u www-data php -d memory_limit=512M occ config:system:set redis host --value='redis'
        sudo -u www-data php -d memory_limit=512M occ config:system:set redis port --value=6379 --type=integer
        sudo -u www-data php -d memory_limit=512M occ config:system:set redis password --value=''
        sudo -u www-data php -d memory_limit=512M occ config:system:set memcache.distributed --value='\\OC\\Memcache\\Redis'
        sudo -u www-data php -d memory_limit=512M occ config:system:set memcache.locking --value='\\OC\\Memcache\\Redis'

        sudo -u www-data php -d memory_limit=512M occ config:system:set --value true allow_local_remote_servers --type=boolean
        sudo -u www-data php -d memory_limit=512M occ config:system:set --value 0 remember_login_cookie_lifetime
        sudo -u www-data php -d memory_limit=512M occ config:system:set --value '173.20.0.2'     trusted_proxies 0
        sudo -u www-data php -d memory_limit=512M occ config:system:set --value 'h.cloud.vpn'    trusted_proxies 1

        sudo -u www-data php -d memory_limit=512M occ config:system:set --value 'cloud.${FQDN}'  trusted_domains 0
        sudo -u www-data php -d memory_limit=512M occ config:system:set --value 'h.cloud.vpn'    trusted_domains 1
        sudo -u www-data php -d memory_limit=512M occ config:system:set --value '173.20.0.2'     trusted_domains 2
        sudo -u www-data php -d memory_limit=512M occ config:system:set --value '173.20.0.4'     trusted_domains 3

        sudo -u www-data php -d memory_limit=512M occ app:install user_ldap
        sudo -u www-data php -d memory_limit=512M occ app:enable  user_ldap
        sudo -u www-data php -d memory_limit=512M occ ldap:create-empty-config
        sudo -u www-data php -d memory_limit=512M occ ldap:set-config s01 ldapHost             freeipaserver
        sudo -u www-data php -d memory_limit=512M occ ldap:set-config s01 ldapPort             389
        sudo -u www-data php -d memory_limit=512M occ ldap:set-config s01 ldapUserFilter       '${LDAP_NEXTCLOUD_ldapUserFilter}'
        sudo -u www-data php -d memory_limit=512M occ ldap:set-config s01 ldapLoginFilter      '${LDAP_NEXTCLOUD_ldapLoginFilter}'
        sudo -u www-data php -d memory_limit=512M occ ldap:set-config s01 ldapUserFilterGroups '${LDAP_NEXTCLOUD_ldapUserFilterGroups}'
        sudo -u www-data php -d memory_limit=512M occ ldap:set-config s01 ldapAgentName        '${LDAP_NEXTCLOUD_ldapAgentName}'
        sudo -u www-data php -d memory_limit=512M occ ldap:set-config s01 ldapAgentPassword    '${LDAP_NEXTCLOUD_ldapAgentPassword}'
        sudo -u www-data php -d memory_limit=512M occ ldap:set-config s01 ldapBase             '${LDAP_NEXTCLOUD_ldapBase}'
        sudo -u www-data php -d memory_limit=512M occ ldap:set-config s01 ldapBaseGroups       '${LDAP_NEXTCLOUD_ldapBaseGroups}'
        sudo -u www-data php -d memory_limit=512M occ ldap:set-config s01 ldapBaseUsers        '${LDAP_NEXTCLOUD_ldapBaseUsers}'
        sudo -u www-data php -d memory_limit=512M occ ldap:set-config s01 ldapConfigurationActive 1

        sudo -u www-data php -d memory_limit=512M occ config:system:set --value true enable_previews --type=boolean
        sudo -u www-data php -d memory_limit=512M occ config:system:set --value 'OC\\Preview\\Image'    enabledPreviewProviders 0
        sudo -u www-data php -d memory_limit=512M occ config:system:set --value 'OC\\Preview\\Movie'   enabledPreviewProviders 1
        sudo -u www-data php -d memory_limit=512M occ config:system:set --value 'OC\\Preview\\TXT'     enabledPreviewProviders 2
        sudo -u www-data php -d memory_limit=512M occ config:system:set --value 'OC\\Preview\\MP3'     enabledPreviewProviders 3
        sudo -u www-data php -d memory_limit=512M occ config:system:set --value 'OC\\Preview\\MKV'     enabledPreviewProviders 4
        sudo -u www-data php -d memory_limit=512M occ config:system:set --value 'OC\\Preview\\MP4'     enabledPreviewProviders 5
        sudo -u www-data php -d memory_limit=512M occ config:system:set --value 'OC\\Preview\\AVI'     enabledPreviewProviders 6

#        sudo -u www-data php -d memory_limit=512M occ app:enable richdocumentscode
        sudo -u www-data php -d memory_limit=512M occ app:enable  \\
            spreed \\
            calendar \\
            deck \\
            contacts \\
            forms \\
            mail \\
            weather_status \\
            dashboard \\
            tasks \\
            files_pdfviewer \\
            richdocuments \\
            bruteforcesettings \\
            files_external \\
            twofactor_totp \\
            admin_audit \\
            cospend \\
            announcementcenter \\
            files_accesscontrol \\
            files_automatedtagging \\
            groupfolders \\
            maps \\
            notes \\
            files_retention \\
            twofactor_webauthn \\
            externalportal
    "
}


setup_freeipa
setup_cloud
#setup_grafana
#setup_vpn
