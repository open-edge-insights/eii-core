FROM ubuntu:16.04

WORKDIR /iei

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

ADD algos/__init__.py algos/__init__.py
ADD algos/etr_utils algos/etr_utils
ADD VisualHmiClient/ VisualHmiClient/

ENV PYTHONPATH $PYTHONPATH:/iei/DataBusAbstraction/py:/iei
ENV PYTHONIOENCODING UTF-8

ENTRYPOINT ["python3.6", "/iei/VisualHmiClient/VisualHmiCleaner/VisualHmiCleaner.py","-c", "/iei/VisualHmiClient/VisualHmiCleaner/config.json"]
CMD [""]
