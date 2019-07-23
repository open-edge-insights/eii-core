# Etcd EIS setup

Follow below steps to run etcd daemon and pre-load EIS data

* Run etcd daemon as container

    Below script starts `etcd` as a container
    ```
    $ sudo ./start_etcd_container.sh
    ```

* Add pre-loaded EIS data to etcd store

    Below script adds [etcd_pre_load.json](etcd_pre_load.json) into `etcd`.
    
    1. Install `etcd3-python`
       
        ```
        $ export PY_ETCD3_VERSION=cdc4c48bde88a795230a02aa574df84ed9ccfa52
        $ git clone https://github.com/kragniz/python-etcd3 && \
          cd python-etcd3 && \
          git checkout ${PY_ETCD3_VERSION} && \
          python3.6 setup.py install && \
          cd .. && \
          rm -rf python-etcd3
        
        ```
    2. Add pre-loaded json data to etcd
       
        ```
         $ cd test
         $ python3.6 etcd_put.py
        ```

    3. Download [etcdkeeper](https://github.com/evildecay/etcdkeeper) platform 
       binary. Run it and open your browser and enter the address: 
       http://127.0.0.1:8080/etcdkeeper to see and update the config
