FROM ghcr.io/savoirfairelinux/opendht/opendht-alpine:latest

RUN apk add --no-cache \
        build-base cmake ninja git wget \
		nettle-dev \
        cppunit-dev gnutls-dev jsoncpp-dev \
        argon2-dev openssl-dev fmt-dev \
        http-parser-dev asio-dev msgpack-cxx-dev

# Build restinio
RUN mkdir restinio && cd restinio \
    && wget https://github.com/aberaud/restinio/archive/bbaa034dbcc7555ce67df0f8a1475591a7441733.tar.gz \
    && ls -l && tar -xzf bbaa034dbcc7555ce67df0f8a1475591a7441733.tar.gz \
    && cd restinio-bbaa034dbcc7555ce67df0f8a1475591a7441733/dev \
    && cmake -DCMAKE_INSTALL_PREFIX=/usr -DRESTINIO_TEST=OFF -DRESTINIO_SAMPLE=OFF \
             -DRESTINIO_INSTALL_SAMPLES=OFF -DRESTINIO_BENCH=OFF -DRESTINIO_INSTALL_BENCHES=OFF \
             -DRESTINIO_FIND_DEPS=ON -DRESTINIO_ALLOW_SOBJECTIZER=Off -DRESTINIO_USE_BOOST_ASIO=none . \
    && make -j8 && make install \
    && cd ../../.. && rm -rf restinio

# Build pjproject
RUN wget https://github.com/savoirfairelinux/pjproject/archive/e4b83585a0bdf1523e808a4fc1946ec82ac733d0.tar.gz \
    && tar -xzf e4b83585a0bdf1523e808a4fc1946ec82ac733d0.tar.gz \
    && mv pjproject-e4b83585a0bdf1523e808a4fc1946ec82ac733d0 pjproject \
    && cd pjproject \
    && EXCLUDE_APP=1 ./aconfigure --prefix=/usr --disable-sound \
                     --enable-video         \
                     --enable-ext-sound     \
                     --disable-speex-aec    \
                     --disable-g711-codec   \
                     --disable-l16-codec    \
                     --disable-gsm-codec    \
                     --disable-g722-codec   \
                     --disable-g7221-codec  \
                     --disable-speex-codec  \
                     --disable-ilbc-codec   \
                     --disable-opencore-amr \
                     --disable-silk         \
                     --disable-sdl          \
                     --disable-ffmpeg       \
                     --disable-v4l2         \
                     --disable-openh264     \
                     --disable-resample     \
                     --disable-libwebrtc    \
                     --with-gnutls=/usr \
    && EXCLUDE_APP=1 make -j8 && make install

COPY . dhtnet

RUN mkdir /install
ENV DESTDIR /install

#RUN cd dhtnet && mkdir build_dev && cd build_dev \
#	&& cmake .. -DCMAKE_INSTALL_PREFIX=/usr \
#				-DCMAKE_INTERPROCEDURAL_OPTIMIZATION=On \
#	&& make -j8 && make install
