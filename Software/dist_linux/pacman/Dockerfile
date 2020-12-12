ARG OS=archlinux
ARG RELEASE=latest
FROM $OS:$RELEASE
RUN pacman -Syu --noconfirm && pacman -S --noconfirm \
    base-devel \
    fftw \
    gcc \
    git \
    libpulse \
    libusb \
    lsb-release \
    namcap \
    qt5-base \
    qt5-serialport \
    qt5-tools \
    && (yes | pacman -Scc) \
    && rm -rf /etc/localtime

VOLUME ["/Lightpack", "/build.sh", "/etc/localtime"]

ENTRYPOINT /build.sh
