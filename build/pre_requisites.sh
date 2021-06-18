#!/bin/bash

# Copyright (c) 2021 Intel Corporation.

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

# Provision
# Usage: sudo ./pre_requisites.sh [--proxy=<proxy address with port number>]


working_dir=$(pwd)
RED=$(tput setaf 1)
GREEN=$(tput setaf 2)
MAGENTA=$(tput setaf 5)
NC=$(tput sgr0)
BOLD=$(tput bold)
INFO=$(tput setaf 3)   # YELLOW (used for informative messages)

 # Variable to store user proxy provided by command line
USER_PROXY=""
DOCKER_REPO="deb [arch=amd64] https://download.docker.com/linux/ubuntu $(lsb_release -cs) stable"
DOCKER_GPG_KEY="0EBFCD88"

# vars
PROXY_EXIST="no"

REQ_DOCKER_COMPOSE_VERSION=1.24.0
REQ_DOCKER_VERSION=19.03.8

#------------------------------------------------------------------------------
# system_info
#
# Description:
#        Display System Information
# Usage:
#        system_info
#------------------------------------------------------------------------------

system_info()
{
    reg=$'\xae';
    echo -e "Software Information :\n--------------------\n"
    echo "Ubuntu Version : $(lsb_release -d | cut -d':' -f2 | awk '{$1=$1};$1')"
    echo "Kernel Version : $(uname -r)"
    echo -e "\nHardware Information :\n--------------------\n"
    echo "Device Manufacturer : $(sudo dmidecode -t baseboard | grep "Manufacturer" | cut -d':' -f2 | awk '{$1=$1};$1')"
    echo "Hardware Architecture : $(uname -p)"
    echo "Processor : $(cat /proc/cpuinfo | grep 'model name' | uniq | cut -d":" -f2 | awk '{$1=$1};$1')"
    echo "Total RAM size : $(free -h | grep "Mem" | awk '{print $2}')"
    echo -e "\nHDD Configurations :\n------------------\n$(sudo parted -ls)"
    echo -e "\nHardware Accelerators :\n---------------------\n"
    VPU=$(lsusb | grep "03e7" | wc -l)
    if [ ! -z "${VPU}" ] && [ "${VPU}" -ne 0 ];then
        echo -e "Found ${VPU} Core Intel$(iconv -f latin1 <<< $reg) Movidius VPU is Connected\n"
    fi
    FPGA=$(lspci | grep "Processing accelerators:.*.Altera" | wc -l)
    if [ ! -z "${FPGA}" ] && [ "${FPGA}" -ne 0 ];then
        echo -e "Found ${FPGA} Intel$(iconv -f latin1 <<< $reg) Arria$(iconv -f latin1 <<< $reg) 10 GX FPGA is Connected\n"
    fi
}


#------------------------------------------------------------------
# check_for_errors
#
# Description:
#        Check the return code of previous command and display either
#        success or error message accordingly. Exit if there is an error.
# Args:
#        string : return-code
#        string : failuer message to display if return-code is non-zero
#        string : success message to display if return-code is zero (optional)
# Return:
#       None
# Usage:
#        check_for_errors "return-code" "failure-message" <"success-message">
#------------------------------------------------------------------
check_for_errors()
{
    return_code=${1}
    error_msg=${2}
    if [ "${return_code}" -ne "0" ];then
        echo "${RED}ERROR : (Err Code: ${return_code}) ${error_msg}${NC}"
        exit 1
    else
        if [ "$#" -ge "3" ];then
            success_msg=${3}
            echo ${success_msg}
        fi
    fi
    return 0
}

#------------------------------------------------------------------------------
# check_root_user
#
# Description:
#        Checking if this script is run with root user.
# Usage:
#        check_root_user
#------------------------------------------------------------------------------

check_root_user()
{
   echo "${INFO}Checking for root user...${NC}"    
   if [[ $EUID -ne 0 ]]; then
    	echo "${RED}This script must be run as root.${NC}"
	echo "${GREEN}E.g. sudo ./<script_name>${NC}"
    	exit 1
   else
   	echo "${GREEN}Script is running with root user.${NC}"
	dpkg --configure -a
   fi
   return 0
   
}

#------------------------------------------------------------------------------
# check_internet_connection
#
# Description:
#        Checking the internet connection
# Usage:
#        check_internet_connection
#------------------------------------------------------------------------------
check_internet_connection()
{
	if [ ! -z $USER_PROXY ] && [ $PROXY_EXIST == "yes" ]
	then 
		echo "${GREEN}Script is running with --proxy mode ${NC}"
		
		sed '/httpProxy/d;/httpsProxy/d;/noProxy/d' -i /etc/environment
        sed '/http_proxy/d;/https_proxy/d;/no_proxy/d' -i /etc/environment
		echo "http_proxy=\"http://${USER_PROXY}\"
            https_proxy=\"http://${USER_PROXY}\"
            no_proxy=\"localhost,127.0.0.0/8\"" >> /etc/environment
        echo "Acquire::http::Proxy \"http://${USER_PROXY}\";
            Acquire::Ftp::Proxy \"http://${USER_PROXY}\";" >> /etc/apt/apt.conf

        export http_proxy="http://${USER_PROXY}"
        export https_proxy=${http_proxy}
        export no_proxy="localhost,127.0.0.1"
	fi
	    echo "${INFO}Checking for Internet Connection...${NC}"    
	    wget http://www.google.com > /dev/null 2>&1
	if [ "$?" != 0 ]; then
		echo "${RED}No Internet Connection. Please check your internet connection and proxy configuration...!${NC}"
		echo "${RED}Internet connection is required.${NC}"
		exit 1
	else
		echo "${GREEN}Internet is available.${NC}"
        # Update the apt package index:
        echo "${GREEN}Update the apt package index.${NC}"
        apt-get -y update
		#Removing temporary file
		if [ -e "index.html" ];then
		    rm -rf index.html
		fi
	fi
	    return 0
}

#------------------------------------------------------------------------------
# install_basic_packages
#
# Description:
#        Installs basic softwares & python packages as pre-requisites.
# Usage:
#        install_basic_packages
#------------------------------------------------------------------------------
install_basic_packages()
{
    echo "${INFO}Installing basic packages required ${NC}"
    # Installing dependent packages
    apt-get update > /dev/null && apt-get -y install build-essential python3-pip wget curl patch jq > /dev/null
    if [ "$?" -ne "0" ]; then
	    echo "${RED}failed to download basic packages. Please check if your basic package manager is working or not using apt-get update command and try to re-run the script${NC}"
	    exit 1
    fi
        pip3 install -r requirements.txt
        if [ "$?" -ne 0 ]; then
		    echo "${RED}the Pre-requisite python packages in IEdgeInsights/build/requirements.txt were not installed properly...!${NC}"
		    exit 1
	    fi
    return 0
}

# ----------------------------
# Changing file permissions
# ----------------------------
change_file_permissions()
{
    chown -R ${SUDO_USER}:${SUDO_USER} *
}

#------------------------------------------------------------------------------
# endOfScript
#
# Description:
#        The final function to be called in the script.
# Usage:
#        endOfScript
#------------------------------------------------------------------------------
endOfScript()
{
    echo "${GREEN}>>>>>${NC}"
	echo "${GREEN}====================================================================="
	echo "${GREEN}============= All pre-requisites are successfully installed ========="
    echo "${GREEN}============= SYSTEM NEEDS TO REBOOT NOW TO APPLY CHANGES TO SYSTEM WIDE ENVIRONMENTAL VARIABLES INCLUDING SYSTEM PROXY & APT PROXY ========="
    echo "Please choose either 1 for 'sript to reboot system' or press any other key if you don't want script to reboot." 
    echo "1) Yes, reboot"
    echo "Any other key) No, system not to be rebooted now"

    read yn

    case ${yn} in
        1)
            #Reboot the system
            echo "System rebooting now"
            reboot
            break;;
        *)
            echo "System did not reboot as per user choice. Kindly note that the system needs to be manually rebooted later to apply system-wide proxy & other environment settings: ${yn}"
    esac
	echo "${GREEN}====================================================================="
}


#------------------------------------------------------------------------------
# validate_action_user_input
#
# Description:
#        Validate & action on the input provided as command line argument to this script.
# Usage:
#        validate_action_user_input <Argument passed to this script>
#------------------------------------------------------------------------------
validate_action_user_input()
{
	if [ $PROXY_EXIST == "no" ]
	then
		echo "${GREEN}Script is running without proxy${NC}"
		return;
	elif [ -z USER_PROXY ]
	then
		echo "${GREEN}Script is running in interactive mode to take proxy from user${NC}"
		proxy_settings
	elif [ $PROXY_EXIST == "yes" ] && [ ! -z USER_PROXY ]
	then 
		echo "${GREEN}Script is running with --proxy mode ${NC}"
		
		#Creating config.json
		if [ ! -d ~/.docker ];then
			mkdir ~/.docker
		fi
		if [ ! -f ~/.docker/config.json ];then
			touch ~/.docker/config.json
			chmod 766 ~/.docker/config.json
		fi
		
		echo "${GREEN}Configuring proxy setting in the system${NC}"
		echo "Docker services will be restarted after proxy settings configured"
		proxy_enabled_network
		check_for_errors "$?" "Failed to configure proxy settings on the system. Please check logs" \
			"${GREEN}Configured proxy settings in the system successfully.${NC}"
		dns_server_settings
		check_for_errors "$?" "Failed to update DNS server settings in the system. Please check logs" \
			"${GREEN}Updated DNS server settings in the system.${NC}"
		echo "${GREEN}proxy is set to :: ${NC}" $USER_PROXY
	else
		Usage "Invalid argument, $1 provided"
	fi
	return 0
}


#------------------------------------------------------------------------------
# proxy_enabled_network
#
# Description:
#        Configure proxy settings for docker client and docker daemon to connect 
#        to internet and also for containers to access internet
# Usage:
#        proxy_enabled_network
#------------------------------------------------------------------------------
proxy_enabled_network()
{
    # 1. Configure the Docker client for http and https proxy
    if [ ! -s "$HOME/.docker/config.json" ]
    then  # if file is empty or if file does not exist
        echo "{
            \"proxies\":
            {
                \"default\":
                {
                    \"httpProxy\": \"http://${USER_PROXY}\",
                    \"httpsProxy\": \"http://${USER_PROXY}\",
                    \"noProxy\": \"127.0.0.1,localhost\"
                }
            }
        }" > ~/.docker/config.json
    else  # if the file already exists && also has some JSON content in it, then append the below JSON object
        HTTP_USER_PROXY="http://${USER_PROXY}"
        jq -r --arg UPROXY ${HTTP_USER_PROXY} '.proxies = {
            "default": {
            "httpProxy": $UPROXY,
            "httpsProxy": $UPROXY,
            "noProxy": "127.0.0.1,localhost"
            }
            }'  ~/.docker/config.json > tmp && mv tmp ~/.docker/config.json
    fi


    # 2. Configure the Docker daemon for http and https proxy
    DOCKER_SERVICE_DIR="/etc/systemd/system/docker.service.d"
    if [ -d $DOCKER_SERVICE_DIR ];then
       rm -rf $DOCKER_SERVICE_DIR
    fi
    mkdir -p $DOCKER_SERVICE_DIR
    touch $DOCKER_SERVICE_DIR/http-proxy.conf
    touch $DOCKER_SERVICE_DIR/https-proxy.conf

    echo "[Service]" > $DOCKER_SERVICE_DIR/http-proxy.conf
    echo "Environment=\"HTTP_PROXY=http://${USER_PROXY}/\" \"NO_PROXY=localhost,127.0.0.1\"" >> $DOCKER_SERVICE_DIR/http-proxy.conf
    check_for_errors "$?" "Failed to update http-proxy.conf files. Please check logs"
    echo "[Service]" > $DOCKER_SERVICE_DIR/https-proxy.conf
    echo "Environment=\"HTTPS_PROXY=http://${USER_PROXY}/\" \"NO_PROXY=localhost,127.0.0.1\"" >> $DOCKER_SERVICE_DIR/https-proxy.conf
    check_for_errors "$?" "Failed to update https-proxy.conf files. Please check logs"

    # Flush the changes
    systemctl daemon-reload
    check_for_errors "$?" "Failed to flush the changes. Please check logs"
    # Restart docker
    systemctl restart docker
    check_for_errors "$?" "Failed to restart docker service. Please check logs"

    return 0
}



#------------------------------------------------------------------------------
# dns_server_setting
#
# Description:
#        Updating correct DNS server details in /etc/resolv.conf
# Usage:
#        dns_server_setting
#------------------------------------------------------------------------------
dns_server_settings()
{
    UBUNTU_VERSION=$(grep "DISTRIB_RELEASE" /etc/lsb-release | cut -d "=" -f2)

    echo "${INFO}Updating correct DNS server details in /etc/resolv.conf${NC}"
    # DNS server settings for Ubuntu 16.04 or earlier
    VERSION_COMPARE=$(echo "${UBUNTU_VERSION} <= 16.04" | bc)
    if [  "${VERSION_COMPARE}" -eq "1" ];then
        if [ -f "/etc/NetworkManager/NetworkManager.conf" ];then
            grep "#dns=dnsmasq" /etc/NetworkManager/NetworkManager.conf
            if [ "$?" -ne "0" ];then
                sed -i 's/dns=dnsmasq/#dns=dnsmasq/g' /etc/NetworkManager/NetworkManager.conf
                systemctl restart network-manager.service
                #Verify on the host
                echo "${GREEN}Udpated DNS server details on host machine${NC}"
                cat /etc/resolv.conf
            fi
        fi
    fi

    # DNS server settings for Ubuntu 18.04 or later
    VERSION_COMPARE=$(echo "${UBUNTU_VERSION} >= 18.04" | bc)
    if [ ${VERSION_COMPARE} -eq "1" ];then
        if [ -f "/run/systemd/resolve/resolv.conf" ];then
            ln -sf /run/systemd/resolve/resolv.conf /etc/resolv.conf
            #Verify on the host
            echo "${GREEN}Udpated DNS server details on host machine${NC}"
            cat /etc/resolv.conf
        fi
    fi

    return 0
}

#------------------------------------------------------------------------------
# uninstall_docker
#
# Description:
#        Utility function to delete the file or content of the directory
# Usage:
#        uninstall_docker <file or directory path>
#------------------------------------------------------------------------------
del_file() 
{
    if [[ -f $1 ]]; then
        rm -rf $1
        if [[ $? -ne 0 ]]; then
            clean_exit
        fi
    fi 
}

#------------------------------------------------------------------------------
# uninstall_docker
#
# Description:
#        Uninstall docker from the system.
# Usage:
#        uninstall_docker
#------------------------------------------------------------------------------
function uninstall_docker()
{
    # UNINSTALLING DOCKER 
    echo -e "${INFO}---------------------------------------Uninstalling Docker---------------------------------------${NC}"
    
    dpkg --purge --force-all docker-ce docker-ce-cli containerd.io
    apt-get purge -y docker docker.io 
    
    # Removing Docker GPG and removing the repository from sources
    apt-key del $DOCKER_GPG_KEY
    add-apt-repository --remove "$DOCKER_REPO"
    echo -e "${GREEN}-------------------------------Docker uninstalled successfully-----------------------------------${NC}" 
    
    #RESET THE PROXY SETTING 
    echo -e "${INFO}---------------------------------------Resetting proxy setting-----------------------------------${NC}"
    del_file /etc/docker/daemon.json
    del_file /etc/systemd/system/docker.service.d/http-proxy.conf
    del_file /etc/systemd/system/docker.service.d/https-proxy.conf      
    del_file $HOME/.docker/config.json
    del_file /etc/systemd/system/docker.service.d
    echo -e "${GREEN}-------------------------------Proxy setting reset to default------------------------------------${NC}"
}


#------------------------------------------------------------------------------
# proxy_settings
#
# Description:
#        Configuring proxy if user in proxy enabled network else
#        the setup will be done with no-proxy settings
# Usage:
#        proxy_settings
#------------------------------------------------------------------------------
proxy_settings()
{
    # Prompt the user for proxy address
    while :
    do
        echo "Is this system in Proxy enabled network?"
        echo "Please choose 1 for 'Yes' and 2 for 'No'"
        echo "1) Yes"
        echo "2) No"

        read yn

        case ${yn} in
            1)
                #Creating config.json
                if [ ! -d ~/.docker ];then
                    mkdir ~/.docker
                fi
                if [ ! -f ~/.docker/config.json ];then
                    touch ~/.docker/config.json
                    chmod 766 ~/.docker/config.json
                fi
				echo "Please enter your proxy address ${GREEN}(Ex: <proxy.example.com>:<port-number>):${NC}"
				read USER_PROXY
				while [ -z "${USER_PROXY}" ]
					do
						echo "${RED}Proxy is empty, please enter again${NC}"
						read USER_PROXY
				done
			
                echo "${GREEN}Configuring proxy setting in the system${NC}"
                echo "Docker services will be restarted after proxy settings configured"
                proxy_enabled_network
                check_for_errors "$?" "Failed to configure proxy settings on the system. Please check logs" \
                    "${GREEN}Configured proxy settings in the system successfully.${NC}"
                dns_server_settings
                check_for_errors "$?" "Failed to update DNS server settings in the system. Please check logs" \
                    "${GREEN}Updated DNS server settings in the system.${NC}"
                break;;
            2)
                echo "${GREEN}Continuing the setup with system network settings.${NC}"
                break;;
            *)
                echo "Entered incorrect input : ${yn}"
        esac
    done

    return 0
}

#------------------------------------------------------------------------------
# docker_install
#
# Description:
#        Install docker-ce and verify it with hello-world image
# Usage:
#        docker_install
#------------------------------------------------------------------------------
docker_install()
{
	echo "${INFO}Installing docker...${NC}"

    # Install packages to allow apt to use a repository over HTTPS:
    apt-get -y install \
        apt-transport-https \
        ca-certificates \
        curl \
        gnupg-agent \
        software-properties-common
    check_for_errors "$?" "Installing packages to allow apt to use a repository over HTTPS failed. Please check logs" \
                    "${GREEN}Installed packages to allow apt to use a repository over HTTPS successfully.${NC}"


    #Add Docker’s official GPG key:
    curl -fsSL https://download.docker.com/linux/ubuntu/gpg | apt-key add -
    check_for_errors "$?" "Failed to add Docker’s official GPG key. Please check logs" \
                    "${GREEN}Added Docker’s official GPG key successfully.${NC}"

    # Verify that you now have the key with the fingerprint
    # 9DC8 5822 9FC7 DD38 854A E2D8 8D81 803C 0EBF CD88, by searching for
    # the last 8 characters of the fingerprint
    non_empty_key=$( apt-key fingerprint 0EBFCD88 | grep '9DC8 5822 9FC7 DD38 854A  E2D8 8D81 803C 0EBF CD88')
    check_for_errors "$?" "Docker's key verification failed. Please check logs" \
                    "${GREEN}Verified the key fingerprint 0EBFCD88 successfully.${NC}"

    # Set up the stable repository
    # Find Hardware-architecture
    # It should be one of amd64 (x86_64/amd64), armhf, arm64, ppc64le (IBM Power), s390x (IBM Z)
    # If there is any issue try one of the commands 'uname -m' or 'uname -p'
    hw_arch=$(uname --hardware-platform)    # uname -i
    if [ ${hw_arch}='amd64' ] || [ ${hw_arch}='x86_64' ]
    then 
        hw_arch='amd64'
    fi

    add-apt-repository \
       "deb [arch=${hw_arch}] https://download.docker.com/linux/ubuntu \
       $(lsb_release -cs) \
       stable"
    echo "${GREEN}Setting up the stable repository is done successfully.${NC}"
    # Stable repositoty set up done successfully
   
    apt-get install -y docker-ce docker-ce-cli containerd.io
    check_for_errors "$?" "Docker CE installation failed. Please check logs" \
                    "${GREEN}Installed Docker CE successfully.${NC}"
    groupadd docker
    usermod -aG docker ${SUDO_USER}
	echo "${GREEN}docker installation is done...${NC}"
    return 0
}

# Utility function to validate version
vercomp () {
    if [[ $1 == $2 ]]
    then
        return 0
    fi
    local IFS=.
    local i ver1=($1) ver2=($2)
    # fill empty fields in ver1 with zeros
    for ((i=${#ver1[@]}; i<${#ver2[@]}; i++))
    do
        ver1[i]=0
    done
    for ((i=0; i<${#ver1[@]}; i++))
    do
        if [[ -z ${ver2[i]} ]]
        then
            # fill empty fields in ver2 with zeros
            ver2[i]=0
        fi
        if ((10#${ver1[i]} > 10#${ver2[i]}))
        then
            return 1
        fi
        if ((10#${ver1[i]} < 10#${ver2[i]}))
        then
            return 2
        fi
    done
    return 0
}

# Utility function to compare two versions
testvercomp () {
    vercomp $1 $2
    case $? in
        0) op='=';;
        1) op='>';;
        2) op='<';;
    esac
    if [[ $op != $3 ]]
    then
        #echo "VERSION MISMATCH: Expected '$3', Actual '$op', Arg1 '$1', Arg2 '$2'"
        echo "${RED}VERSION MISMATCH: Expected '$2', Actual '$1'${NC}"
	return 1
    else
        echo "${GREEN}Pass: '$1 $op $2'${NC}"
	return 0
    fi
}




#------------------------------------------------------------------
# docker_compose_install
#
# Description:
#        Installing docker-compose on the system
# Return:
#        None
# Usage:
#        docker_compose_install
#------------------------------------------------------------------
docker_compose_install()
{
	echo "${INFO}Installing docker-compose...${NC}"
    rm -rf $(which docker-compose)
    pip3 uninstall -y docker-compose | rm -rf `which docker-compose`
    # Downloading docker-compose using curl utility.
    curl -L "https://github.com/docker/compose/releases/download/${REQ_DOCKER_COMPOSE_VERSION}/docker-compose-$(uname -s)-$(uname -m)" -o /usr/local/bin/docker-compose
    if [ "$?" -eq "0" ];then
        # Making the docker-compose executable. 
        chmod +x /usr/local/bin/docker-compose
        ln -sf /usr/local/bin/docker-compose /usr/bin/docker-compose
        echo "${GREEN}Installed docker-compose successfully.${NC}"
    else
        echo "${RED}ERROR: Docker-compose Downloading Failed.Please Check Manually.${NC}"
        exit 1
    fi
    return 0  
}

#------------------------------------------------------------------------------ 
# docker_compose_verify_installation
#
# Description: 
#       Verify if docker-compose already exists and if the existing version is older than the intended version
# Args:
#       None
# Return:
#       None
# Usage:
#       docker_compose_verify_installation
#------------------------------------------------------------------------------
docker_compose_verify_installation()
{
    echo "Verifying if docker_compose already exists."
    cur_v=$(echo $(docker-compose --version) | cut -d"n" -f 2 | cut -d"," -f 1)
	if [ -z $cur_v ];then
		echo "${INFO}docker-compose is not installed on machine${NC}"
		echo "${INFO}docker-compose needs to be Installed.${NC} "
        docker_compose_install
	else
		testvercomp $cur_v $REQ_DOCKER_COMPOSE_VERSION "="
		ret=$?
		if [ "$ret" -eq 0 ]; then
			echo "${GREEN}docker-compose is already installed, ${cur_v}${NC}"
		else
			echo "${INFO}docker-compose is either not installed or old one${NC}"
			echo "Required = "${REQ_DOCKER_COMPOSE_VERSION}, "Present version = " ${cur_v}
			read -p "Do you want to proceed with the installation of above required version of docker-compose tool?[y/n]" yn
			case $yn in
				[Yy] ) docker_compose_install;;
				[Nn] ) ;;
			esac
		fi
	fi
	echo "${GREEN}docker-compose installation is done...${NC}"
    return 0
}


#------------------------------------------------------------------------------ 
# docker_verification_installation
#
# Description: 
#       verifies the docker installation.
# Args:
#       None
# Return:
#       None
# Usage:
#       docker_verification_installation
#------------------------------------------------------------------------------
docker_verification_installation()
{
    echo "Verifying if docker already exists."
    cur_v=$(echo $(docker --version) | cut -d"n" -f 2 | cut -d"," -f 1)
	if [ -z $cur_v ];then
		echo "${INFO}docker is not installed on machine${NC}"
		echo "${INFO}docker needs to be Installed.${NC} "
		docker_install
	else
		testvercomp $cur_v $REQ_DOCKER_VERSION ">"
		ret=$?
		if [ "$ret" -eq 0 ]; then
			echo "${GREEN} Expected docker version is already installed, ${cur_v}${NC}"
		else
			echo "${INFO}docker version is either not installed or old one${NC}"
			echo "Required = "${REQ_DOCKER_VERSION}, "Present version = " ${cur_v}
			read -p "Do you want to proceed with the installation of above required version of docker?[y/n]" yn
			case $yn in
				[Yy] )
						uninstall_docker
						docker_install;;
				[Nn] ) ;;
			esac
		fi
	fi
    validate_action_user_input "$@"
    return 0
}

#------------------------------------------------------------------
# parse_commandLine_Args
#
# Description:
#        This function is used to Parse command line arguments passed to this script
# Return:
#        None
# Usage:
#        parse_commandLine_Args <list of arguments>
#------------------------------------------------------------------
parse_commandLine_Args()
{
	if [ $# == "0" ];then
		return;
	fi
	
	echo "${INFO}Reading the command line args...${NC}"
	for ARGUMENT in "$@"
	do
	    KEY=$(echo $ARGUMENT | cut -f1 -d=)
	    VALUE=$(echo $ARGUMENT | cut -f2 -d=)   

	   #echo ${GREEN}$KEY "=" $VALUE${NC}
	   #echo "${GREEN}==========================================${NC}"

	    case "$KEY" in
		    --proxy) USER_PROXY=${VALUE} PROXY_EXIST="yes";;   
		    --help) Usage ;; 
            -h) Usage ;; 
		     *) echo "${RED}Invalid arguments passed..${NC}"; Usage; ;;  
	    esac    
	done
	
	print_all_args
	
	#echo "${GREEN}==========================================${NC}"
}

#------------------------------------------------------------------
# Usage
#
# Description:
#        Help function 
# Return:
#        None
# Usage:
#        Usage
#------------------------------------------------------------------
Usage()
{
	echo 
	echo "${BOLD}${INFO}==================================================================================${NC}"
	echo
	echo "${BOLD}${GREEN}Usage :: sudo ./pre_requisites.sh [OPTION...] ${NC}"
	echo
	echo "${INFO}List of available options..."
    echo
	echo "${INFO}--proxy		proxies, required when the gateway/edge node running EII (or any of EII profile) is connected behind proxy"
	echo
	echo "${INFO}--help		display this help and exit"${NC}
    echo
    echo "${BOLD}${GREEN}Note : If --proxy option is not provided then script will run without proxy${NC}"
	echo
	echo "Different use cases..."
	echo "${BOLD}${MAGENTA}
		
		1. RUNS WITHOUT PROXY
		sudo ./pre_requisites.sh

		2.RUNS WITH PROXY
		sudo ./pre_requisites.sh --proxy=\"proxy.intel.com:211\"
	"
	echo "${INFO}===================================================================================${NC}"
	exit 1
}

#------------------------------------------------------------------
# print_all_args
#
# Description:
#        This function is used to print all given values on console 
# Return:
#        None
# Usage:
#        print_all_args
#------------------------------------------------------------------
print_all_args()
{
	echo "${GREEN}==========================================${NC}"
	echo "${GREEN}Given values..${NC}"
	if [ ! -z $USER_PROXY ]; then echo "${INFO}--proxy = $USER_PROXY${NC}";fi
	echo "${GREEN}==========================================${NC}"
}

echo "${GREEN}============================= Script START ============================================${NC}"
# Internal function calls to set up prerequisites
if [ $# -ne "0" ];then
	parse_commandLine_Args "$@"
fi
system_info 2>&1
check_root_user
check_internet_connection "$@"
install_basic_packages
docker_verification_installation	"$@"
docker_compose_verify_installation
change_file_permissions
endOfScript
cd "${working_dir}"
exit 0
