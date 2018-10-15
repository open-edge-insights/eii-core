FROM ubuntu:16.04

WORKDIR /eta

ENV no_proxy localhost

RUN apt-get update -qq
RUN apt-get install -y --no-install-recommends --fix-missing python python-pip libgtk2.0-dev && \
    pip install --upgrade pip setuptools

RUN apt-get install -y cron

ADD VisualHmiClient/VisualHmiCleaner/requirements.txt ./VisualHmiClient/VisualHmiCleaner/requirements.txt
RUN pip install -r VisualHmiClient/VisualHmiCleaner/requirements.txt

ADD agent/__init__.py agent/__init__.py
ADD agent/etr_utils agent/etr_utils
ADD VisualHmiClient/ VisualHmiClient/

ENV PYTHONPATH $PYTHONPATH:/eta/DataBusAbstraction/py:/eta
ENV PYTHONIOENCODING UTF-8

ENTRYPOINT ["python2.7", "/eta/VisualHmiClient/VisualHmiCleaner/VisualHmiCleaner.py","-c", "/eta/VisualHmiClient/VisualHmiCleaner/config.json"]
CMD [""]
