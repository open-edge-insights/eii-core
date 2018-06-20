# dockerfile for DataAgent
FROM ubuntu:16.04
LABEL version="1.0"
LABEL description="DataAgent image"

ENV DIR /ETA
WORKDIR ${DIR}
RUN mkdir -p log
VOLUME ${DIR}/log

# Adding DataAgent binary and conf file
ADD DataAgent . 
ADD DataAgent.conf .
ENTRYPOINT ./DataAgent -config=./DataAgent.conf -log_dir=${DIR}/log

