# syntax=docker/dockerfile:1
FROM ubuntu:latest

ENV NOVNC_PASSWORD=$NOVNC_PASSWORD

EXPOSE 5901

RUN apt-get update
RUN export DEBIAN_FRONTEND=noninteractive DEBCONF_NONINTERACTIVE_SEEN=true && \
    apt-get install -yq \
    x11vnc \
    vim \
    wget \
    curl \
    screen \
    xvfb \
    novnc \
    xdotool \
    kate \
    konsole \
    git \
    git-lfs \
    bc \
    lxde

RUN curl -fsSL https://deb.nodesource.com/setup_19.x | bash - && apt-get install -y nodejs

#RUN export DEBIAN_FRONTEND=noninteractive DEBCONF_NONINTERACTIVE_SEEN=true && apt-get install -yq fuse
#RUN wget https://apprepo.de/appimage/download/vscode --output-document=VSCode.AppImage
#RUN chmod +x ./VSCode.AppImage

COPY entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh
ENTRYPOINT '/entrypoint.sh'
