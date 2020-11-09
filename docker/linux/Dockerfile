FROM debian:stable
MAINTAINER cagnulein

ENV MAKEFLAGS -j8
WORKDIR /usr/local/src

# utils
RUN apt -y update
RUN apt -y upgrade
RUN apt -y install git libqt5bluetooth5 libqt5widgets5 libqt5positioning5 libqt5xml5 qtconnectivity5-dev qtpositioning5-dev libqt5charts5-dev libqt5charts5 qt5-default