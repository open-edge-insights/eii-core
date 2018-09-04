FROM ubuntu:16.04

USER root
WORKDIR /eta

ENV no_proxy localhost

RUN apt-get update -qq
RUN apt-get install -y --no-install-recommends --fix-missing python python-pip libgtk2.0-dev && \
    pip install --upgrade pip setuptools

ADD VisualHmiClient/ VisualHmiClient/
ADD DataBusAbstraction/ DataBusAbstraction/
ADD DataAgent/ DataAgent/
ADD Util/ Util/

RUN pip install -r VisualHmiClient/requirements.txt

ENV PYTHONPATH $PYTHONPATH:/eta/DataBusAbstraction/py:/eta
ENV PYTHONIOENCODING UTF-8

ENTRYPOINT [ "python2.7", "VisualHmiClient/VisualHmiEtaDataSync.py"]
CMD ["-c", "VisualHmiClient/config.json"]
