# Section 1: Build the application
FROM ubuntu:18.04 as builder

# Coalesce the run statements to decrease image layers
RUN apt-get update -y && \
    apt-get upgrade -y && \
    apt-get dist-upgrade -y && \
    apt-get install -y --no-install-recommends \
        cmake \
        build-essential \
        gcovr

ADD . /opt/sources
WORKDIR /opt/sources

RUN cd /opt/sources && \
    mkdir build && \
    cd build && \
    cmake -D CMAKE_BUILD_TYPE=Release .. && \
    make && make test && gcovr -r ../ -f "../PrimeChecker.cpp" -f "../helloworld.cpp" --xml-pretty --exclude-unreachable-branches --print-summary -o coverage.xml && cp helloworld coverage.xml /tmp

# Section 2: Bundle the application
# Changed the image from Ubuntu to Scratch
FROM scratch

WORKDIR /opt
COPY --from=builder /tmp ./
ENTRYPOINT ["/opt/helloworld"]
