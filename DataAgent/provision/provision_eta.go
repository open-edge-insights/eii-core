package main

import (
	"encoding/json"
	"errors"
	"flag"
	"github.com/golang/glog"
	"github.com/hashicorp/vault/api"
	"io/ioutil"
	"os"
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

	CERT_FILE_PATH := "./vault_certs/"

	VAULT_SECRET_FILE := "./vault_secret_file"

	// Enable the TLS config.
	vlt_config := api.DefaultConfig()
	//TODO: Read it from TPM
	ca_cert_file_path := CERT_FILE_PATH + "ca_certificate.pem"

	vlt_config.ConfigureTLS(&api.TLSConfig{
		CACert: ca_cert_file_path,
	})

	// Read the secrets and populate the internal data structure.PANI
	vlt_client, err := api.NewClient(vlt_config)

	if err != nil {
		glog.Errorf("Provision: failed to create vault client: %s", err)
		return nil, err
	}

	vlt_client.SetAddress("https://ia_vault:8200")

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
			glog.Errorf("Failed to write secreet to file %s, err: %s", VAULT_SECRET_FILE, err)
		}
		glog.Infof("The secret struct is %+v, %s",resp, secret_blob)


		sys_obj.Unseal(resp.Keys[0])
		vlt_client.SetToken(resp.RootToken)
	}

	return vlt_client, err
}

func main() {

	var config_file_path string
	flag.StringVar(&config_file_path, "config", "", "config file path")
	flag.Parse()

	vlt_client, err := init_vault_eta()
	if err != nil {
		glog.Errorf("Failed to initialize vault, error: %s", err)
		os.Exit(-1)
	}
	glog.Infof("Initialized Vault succefully")
	err = write_secret(config_file_path, vlt_client)
	if err != nil {
		glog.Errorf("Failed to write successfully to vault, error: %s", err)
		os.Exit(-1)
	}
	glog.Infof("Write secret to Vault succefully")


}
