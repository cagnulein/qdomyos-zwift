FROM ubuntu:latest
MAINTAINER cagnulein

ARG DEBIAN_FRONTEND=noninteractive
ENV TZ=Europe/Moscow
ENV MAKEFLAGS -j8
WORKDIR /usr/local/src

RUN apt-get update && apt-get install -y tzdata
RUN apt -y update
RUN apt -y upgrade
RUN apt update -y && apt-get install -y git qt5-default libqt5bluetooth5 libqt5widgets5 libqt5positioning5 libqt5xml5 qtconnectivity5-dev qtpositioning5-dev libqt5charts5-dev libqt5charts5 qt5-default libqt5networkauth5-dev libqt5websockets5* libxcb-randr0-dev libxcb-xtest0-dev libxcb-xinerama0-dev libxcb-shape0-dev libxcb-xkb-dev build-essential

RUN git clone https://github.com/cagnulein/qdomyos-zwift.git
WORKDIR /usr/local/src/qdomyos-zwift

RUN git submodule update --init src/smtpclient/
RUN git submodule update --init src/qmdnsengine/
WORKDIR /usr/local/src/qdomyos-zwift/src
RUN qmake
RUN make -j4

WORKDIR /usr/local/src/qdomyos-zwift/src
CMD ["./qdomyos-zwift","-no-gui"]
