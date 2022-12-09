# syntax=docker/dockerfile:1.3-labs
FROM alpine:latest

RUN apk update
RUN apk add bash vim
RUN apk add apache2 apache2-ldap php81 php81-session php81-zip php81-pdo_mysql php81-phar php81-mbstring php81-iconv php81-tokenizer php81-xml php81-dom php81-xmlwriter php81-curl php81-fileinfo
RUN apk add php81-apache2

RUN php -r "copy('https://getcomposer.org/installer', 'composer-setup.php');" && \
    php composer-setup.php && \
    php -r "unlink('composer-setup.php');" && \
    mv composer.phar /usr/local/bin/composer && \
    composer self-update --2

RUN composer global require laravel/installer
RUN chmod a+x /usr/local/bin/*

COPY httpd.conf /etc/apache2/httpd.conf
COPY create_api_model.sh /create_api_model.sh
COPY _modelController_.php /_modelController_.php
COPY _modelName_.php /_modelName_.php
COPY _migration_.php /_migration_.php

RUN echo "#!/bin/bash" > /entrypoint.sh
RUN echo "touch /log" >> /entrypoint.sh
RUN echo "[ -d \"/project/public\"  ] || {" >> /entrypoint.sh
RUN echo "composer create-project laravel/laravel /project" >> /entrypoint.sh
RUN echo "cd /project && composer require darkaonline/l5-swagger" >> /entrypoint.sh

RUN echo "}" >> /entrypoint.sh

RUN echo "httpd -k start -DBACKGROUND" >> /entrypoint.sh
RUN echo "tail -F /log" >> /entrypoint.sh

RUN chmod a+x /entrypoint.sh
RUN chmod a+x /create_api_model.sh

ENTRYPOINT ["/entrypoint.sh"]
