ARG OS=ubuntu
ARG RELEASE=20.04
FROM $OS:$RELEASE
ENV DEBIAN_FRONTEND=noninteractive
RUN apt update && apt dist-upgrade -y && apt install -y \
    bash \
    build-essential \
    gcc \
    git \
    hashdeep \
    libfftw3-dev \
    libpulse-dev \
    libqt5serialport5-dev \
    libudev-dev \
    libusb-1.0-0-dev \
    lsb-release \
    qt5-default \
    qttools5-dev-tools \
    && rm -rf /var/lib/apt/lists/* \
    && rm -rf /etc/localtime

VOLUME ["/Lightpack", "/build.sh", "/etc/localtime"]

ENTRYPOINT /build.sh
