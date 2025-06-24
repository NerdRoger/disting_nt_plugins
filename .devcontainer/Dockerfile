FROM ubuntu:24.04

ARG TARGETPLATFORM
ARG BUILDPLATFORM
ARG TOOLS_PATH=/opt/gcc-arm-none-eabi
ARG ARM_VERSION=14.2.rel1

# Preconfigure tzdata
ENV DEBIAN_FRONTEND=noninteractive \
    TZ=Etc/UTC

# Install required packages
RUN apt-get update && apt-get install -y \
    build-essential \
    gcc-arm-none-eabi \
    make \
    cmake \
    git \
    python3 \
    pkg-config \
    curl

WORKDIR /opt

# Get ARM Toolchain
RUN if [ "$TARGETPLATFORM" = "linux/amd64" ]; then export ARM_ARCH=x86_64; else export ARM_ARCH=aarch64; fi \
	&& mkdir ${TOOLS_PATH} \
	&& curl -Lo gcc-arm-none-eabi.tar.xz "https://developer.arm.com/-/media/Files/downloads/gnu/${ARM_VERSION}/binrel/arm-gnu-toolchain-${ARM_VERSION}-${ARM_ARCH}-arm-none-eabi.tar.xz" \
	&& tar xf gcc-arm-none-eabi.tar.xz --strip-components=1 -C ${TOOLS_PATH} \
	&& rm gcc-arm-none-eabi.tar.xz \
	&& rm ${TOOLS_PATH}/*.txt \
	&& rm -rf ${TOOLS_PATH}/share/doc

# Add Toolchain to PATH
ENV PATH="$PATH:${TOOLS_PATH}/bin"    

WORKDIR /app

# Build and Install Faust
RUN git clone https://github.com/grame-cncm/faust.git
RUN cd faust && make && make install
RUN rm -rf faust

# Files created by this container have no permission restrictions
# Allows the files created to be deleted by the host, SMB shares, etc
RUN echo >> /root/.bashrc
RUN echo "umask 0000" >> /root/.bashrc

CMD ["/bin/bash"]