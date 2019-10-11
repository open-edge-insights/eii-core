import os
import shutil
import yaml
from distutils.util import strtobool 

def generate_prod_config_files():

    f = open('./Certificates/ca/ca_certificate.pem', 'r')
    lines = f.readlines()
    tlsCACert = "\\n".join([line.strip() for line in lines])
    f = open('./Certificates/Grafana/Grafana_client_certificate.pem', 'r')
    lines = f.readlines()
    tlsClientCert = "\\n".join([line.strip() for line in lines])
    f = open('./Certificates/Grafana/Grafana_client_key.pem', 'r')
    lines = f.readlines()
    tlsClientKey = "\\n".join([line.strip() for line in lines])


    with open('../grafana/datasource_sample.yml', 'r') as fin:
        with open("../grafana/datasource.yml", "w+") as fout:
            for line in fin.readlines():
                if "url:" in line:
                    line=line.replace('http://localhost:8086', 'https://localhost:8086')
                    fout.write(line)
                elif "tlsAuth:" in line:
                    line=line.replace('false', 'true')
                    fout.write(line)
                elif "tlsAuthWithCACert:" in line:
                    line=line.replace('false', 'true')
                    fout.write(line)
                elif "tlsCACert:" in line:
                    line=line.replace('"..."', '"' + tlsCACert + '"')
                    fout.write(line)
                elif "tlsClientCert:" in line:
                    line=line.replace('"..."', '"' + tlsClientCert + '"')
                    fout.write(line)
                elif "tlsClientKey:" in line:
                    line=line.replace('"..."', '"' + tlsClientKey + '"')
                    fout.write(line)
                else: 
                    fout.write(line)

    with open('../grafana/grafana_template.ini', 'r') as fin:
        with open("../grafana/grafana.ini", "w+") as fout:
            cert_file_updated = False
            for line in fin.readlines():
                if ";protocol =" in line:
                    line=line.replace(';protocol = http','protocol = https')
                    fout.write(line)
                elif ";cert_file =" in line and not cert_file_updated:
                    line=line.replace(';cert_file =','cert_file = /etc/ssl/Grafana/Grafana_client_certificate.pem')
                    fout.write(line)
                    cert_file_updated = True
                elif ";cert_key =" in line:
                    line=line.replace(';cert_key =','cert_key = /etc/ssl/Grafana/Grafana_client_key.pem')
                    fout.write(line)
                else: 
                    fout.write(line)           
        
    
def generate_dev_config_files():
    # No action required at this point
    return

def copy_config_files(dev_mode):

    if dev_mode:
        print("copying dev file")
        shutil.copy('../grafana/grafana_template.ini','../grafana/grafana.ini')
        shutil.copy('../grafana/datasource_sample.yml', '../grafana/provisioning/datasources/datasource.yml')
    else:
        shutil.copy('../grafana/datasource.yml', '../grafana/provisioning/datasources/datasource.yml')


def check_grafana_enabled():
    with open('../docker-compose.yml') as f:
            docs = yaml.load_all(f, Loader=yaml.FullLoader)
            for doc in docs:
                for key, value in doc.items():
                    if key == "services":
                        if 'ia_grafana' in value.keys():
                            return True

if __name__ == "__main__":
    
    grafana_enabled = check_grafana_enabled()
    if grafana_enabled:
        print("ia_grafana enabled")
        dev_mode = strtobool(os.environ['DEV_MODE'])
        print ("mode is ", dev_mode)
        if not dev_mode :
            print("generating prod mode config files for grafana")
            generate_prod_config_files()
        else :
            print("generating dev mode config files for grafana")
            generate_dev_config_files()

        copy_config_files(dev_mode)
    else:
        print("ia_grafana not enabled")
        