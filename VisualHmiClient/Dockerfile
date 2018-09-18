FROM ubuntu:16.04

USER root
WORKDIR /eta

ENV no_proxy localhost

RUN apt-get update -qq
RUN apt-get install -y --no-install-recommends --fix-missing python python-pip libgtk2.0-dev && \
    pip install --upgrade pip setuptools

ADD VisualHmiClient/requirements.txt ./VisualHmiClient/requirements.txt
RUN pip install -r VisualHmiClient/requirements.txt

ADD agent/__init__.py agent/__init__.py
ADD agent/etr_utils agent/etr_utils
ADD VisualHmiClient/ VisualHmiClient/
ADD DataBusAbstraction/ DataBusAbstraction/
ADD DataAgent/ DataAgent/
ADD Util/ Util/

ENV PYTHONPATH $PYTHONPATH:/eta/DataBusAbstraction/py:/eta
ENV PYTHONIOENCODING UTF-8

ENTRYPOINT [ "python2.7", "VisualHmiClient/VisualHmiEtaDataSync.py"]
CMD ["-c", "VisualHmiClient/config.json"]
