FROM alpine:3.14

ARG USER="builder"
RUN apk add alpine-sdk git sudo gcc g++ make cmake

RUN adduser -D $USER \
        && echo "$USER ALL=(ALL) NOPASSWD: ALL" > /etc/sudoers.d/$USER \
        && chmod 0440 /etc/sudoers.d/$USER
RUN addgroup $USER abuild

RUN mkdir -p /var/cache/distfiles
RUN chgrp abuild /var/cache/distfiles
RUN chmod g+w /var/cache/distfiles

COPY ./build.sh /bin/
RUN chmod +x /bin/build.sh

RUN mkdir /package
RUN chown $USER:$USER /package
USER ${USER}
RUN abuild-keygen -a -i -n
WORKDIR /package

CMD ["sh", "/bin/build.sh"]
