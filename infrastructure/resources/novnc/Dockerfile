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

RUN wget https://az764295.vo.msecnd.net/stable/97dec172d3256f8ca4bfb2143f3f76b503ca0534/code_1.74.3-1673284829_amd64.deb
RUN dpkg -i code_1.74.3-1673284829_amd64.deb
RUN curl -fsSL https://deb.nodesource.com/setup_19.x | bash - && apt-get install -y nodejs

COPY entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh
ENTRYPOINT '/entrypoint.sh'
