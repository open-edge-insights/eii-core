FROM ubuntu:16.04

WORKDIR /eta

ENV no_proxy localhost

RUN apt-get update
RUN apt-get install -y software-properties-common
RUN apt-get install -y pkg-config python python-pip
RUN add-apt-repository ppa:deadsnakes/ppa
RUN apt-get update
RUN apt-get install -y python3.6 python3.6-dev wget

RUN wget https://bootstrap.pypa.io/get-pip.py
RUN python3.6 get-pip.py

RUN apt-get install -y cron

ADD VisualHmiClient/VisualHmiCleaner/requirements.txt ./VisualHmiClient/VisualHmiCleaner/requirements.txt
RUN pip3.6 install -r VisualHmiClient/VisualHmiCleaner/requirements.txt

ADD agent/__init__.py agent/__init__.py
ADD agent/etr_utils agent/etr_utils
ADD VisualHmiClient/ VisualHmiClient/

ENV PYTHONPATH $PYTHONPATH:/eta/DataBusAbstraction/py:/eta
ENV PYTHONIOENCODING UTF-8

ENTRYPOINT ["python3.6", "/eta/VisualHmiClient/VisualHmiCleaner/VisualHmiCleaner.py","-c", "/eta/VisualHmiClient/VisualHmiCleaner/config.json"]
CMD [""]
