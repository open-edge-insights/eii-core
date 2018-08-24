#!/bin/bash

chmod 777 /tmp/classifier

#this unsets the proxy in this shell, this isn't a system-wide unsetting
unset {http,https}_proxy
unset {HTTP,HTTPS}_PROXY

kapacitord -hostname ia_data_analytics
