package main

import (
	util "ElephantTrunkArch/Util"
	"encoding/json"
	"errors"
	"flag"
	"io/ioutil"
	"os"

	"github.com/golang/glog"
	"github.com/hashicorp/vault/api"
)

func write_secret(config_file_path string, vlt_client *api.Client) error {

	var data map[string]interface{}
	byteValue, err := ioutil.ReadFile(config_file_path)
	if err != nil {
		glog.Errorf("Provision: Error Reading file %s, error: %s", config_file_path, err)
		return err
	}

	json.Unmarshal([]byte(byteValue), &data)
	//secret_paths := reflect.ValueOf(data).MapKeys()

	logical_clnt := vlt_client.Logical()
	for k, v := range data {
		_, err := logical_clnt.Write("secret/"+k, v.(map[string]interface{}))
		if err != nil {
			glog.Errorf("Failed to write secret %s to vault err:%s", "secret"+k, err)
			return err
		}
	}

	//Seal the VAULT
	sys_obj := vlt_client.Sys()
	err = sys_obj.Seal()
	if err != nil {
		glog.Errorf("Failed to Seal err: %s", err)
	}

	return err
}

func init_vault_eta() (*api.Client, error) {

	VAULT_SECRET_FILE := "/vault/file/vault_secret_file"

	// Enable the TLS config.
	vlt_config := api.DefaultConfig()

	// Read the secrets and populate the internal data structure.PANI
	vlt_client, err := api.NewClient(vlt_config)

	if err != nil {
		glog.Errorf("Provision: failed to create vault client: %s", err)
		return nil, err
	}

	vlt_client.SetAddress("http://localhost:8200")

	//Check for vault is initialized or not.
	sys_obj := vlt_client.Sys()
	vault_initialized, err := sys_obj.InitStatus()
	if err != nil {
		glog.Errorf("Provision: Failed to querry vault init status: %s", err)
		return nil, err
	}

	if vault_initialized {
		glog.Errorf("The vault container found to be tampered. Please do full reprovision.")
		return nil, errors.New("Tampered Vault")
	} else {
		resp, err := sys_obj.Init(&api.InitRequest{
			SecretShares:    1,
			SecretThreshold: 1,
		})

		if err != nil {
			glog.Errorf("Provision: Initialization of VAULT failed: %s", err)
			return nil, err
		}

		secret_blob, err := json.Marshal(resp)
		if err != nil {
			glog.Errorf("json marshal of vault secret failed")
			return nil, err
		}
		//Write secrets to file for vault for unsealing during ETA life

		err = ioutil.WriteFile(VAULT_SECRET_FILE, secret_blob, 0700)
		if err != nil {
			glog.Errorf("Failed to write unseal-key to file %s, err: %s", VAULT_SECRET_FILE, err)
		}
		//glog.Infof("The secret struct is %+v, %s", resp, secret_blob)

		sys_obj.Unseal(resp.Keys[0])
		vlt_client.SetToken(resp.RootToken)
	}

	return vlt_client, err
}

func main() {

	var config_file_path string
	flag.StringVar(&config_file_path, "config", "", "config file path")
	flag.Parse()

	vaultPort := os.Getenv("VAULT_PORT")
	portUp := util.CheckPortAvailability("localhost", vaultPort)
	if !portUp {
		glog.Error("VAULT server is not up, so exiting...")
		os.Exit(-1)
	}

	vlt_client, err := init_vault_eta()
	if err != nil {
		glog.Errorf("Failed to initialize vault, error: %s", err)
		os.Exit(-1)
	}
	glog.Infof("Initialized Vault successfully")

	// Read the certificate files and populate the vault
	cert_root_dir := "/etc/ssl/"

	cert_dirs, err := ioutil.ReadDir(cert_root_dir)
	if err != nil {
		glog.Errorf("Unable to read certificate dir: %s", cert_root_dir)
		os.Exit(-1)
	}
	//Read the certificates folder and update the vault
	logical_clnt := vlt_client.Logical()
	secret_to_write := make(map[string]interface{})
	for _, dir := range cert_dirs {
		dir_path := cert_root_dir + "/" + dir.Name()
		files, err := ioutil.ReadDir(dir_path)
		if err != nil {
			glog.Errorf("Unable to read certificate dir: %s", cert_root_dir)
			os.Exit(-1)
		}
		for _, file := range files {
			file_path := dir_path + "/" + file.Name()
			if file.Name() != "ca_key" {
				byteValue, err := ioutil.ReadFile(file_path)
				if err != nil {
					glog.Errorf("Provision: Error Reading file %s, error: %s", file, err)
					os.Exit(-1)
				}
				secret_to_write[file.Name()] = byteValue
			}
		}
	}

	_, err = logical_clnt.Write("secret/Certificates", secret_to_write)
	if err != nil {
		glog.Errorf("Failed to write secret to vault err:%s", err)
		os.Exit(-1)
	}
	glog.Infof("Written certificate to Vault successfully")
	// Write
	err = write_secret(config_file_path, vlt_client)
	if err != nil {
		glog.Errorf("Failed to write successfully to vault, error: %s", err)
		os.Exit(-1)
	}
	glog.Infof("Written credential secret to Vault succefully")
}
