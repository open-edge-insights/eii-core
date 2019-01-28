package main

import (
	util "IEdgeInsights/Util"
	tpmutil "IEdgeInsights/Util/Tpm"
	"encoding/json"
	"errors"
	"flag"
	"io/ioutil"
	"os"
	"strconv"

	"github.com/golang/glog"
	"github.com/hashicorp/vault/api"
)

const (
	vaultFilePerm = 0660
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
	tpmVaultSecretPath := "/vault/vault_secret_file"

	type vaultSecret struct {
		Keys      []string `json:"keys"`
		RootToken string   `json:"root_token"`
	}

	var vltSecrt vaultSecret

	tString := os.Getenv("TPM_ENABLE")
	TPM_ENABLED, err := strconv.ParseBool(tString)
	if err != nil {
		glog.Errorf("Fail to read TPM environment variable: %s", err)
	}

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
		glog.Errorf("The vault container found to be tampered. Kindly reprovision the system.")
		return nil, errors.New("Tampered Vault")
	}

	resp, err := sys_obj.Init(&api.InitRequest{
		SecretShares:    1,
		SecretThreshold: 1,
	})

	if err != nil {
		glog.Errorf("Provision: Initialization of VAULT failed: %s", err)
		return nil, err
	}

	vltSecrt.Keys = resp.Keys
	vltSecrt.RootToken = resp.RootToken

	secret_blob, err := json.Marshal(vltSecrt)
	if err != nil {
		glog.Errorf("json marshal of vault secret failed")
		return nil, err
	}

	if TPM_ENABLED {
		glog.Infof("*************SEALING VAULT CREDS TO TPM***********")
		err = ioutil.WriteFile(tpmVaultSecretPath, secret_blob, vaultFilePerm)
		if err != nil {
			glog.Errorf("Failed to write unseal-key to file %s, err: %s", tpmVaultSecretPath, err)
		}

		err = tpmutil.WriteToTpm(tpmVaultSecretPath)
		if err != nil {
			glog.Errorf("Failed to store vault credentials to TPM, Error: %s", err)
			os.Exit(-1)
		}

	} else {
		//This path is executed when we run in developement mode or non-TPM capable machine
		err = ioutil.WriteFile(VAULT_SECRET_FILE, secret_blob, vaultFilePerm)
		if err != nil {
			glog.Errorf("Failed to write unseal-key to file %s, err: %s", VAULT_SECRET_FILE, err)
			os.Exit(-1)
		}
	}

	sys_obj.Unseal(vltSecrt.Keys[0])
	vlt_client.SetToken(vltSecrt.RootToken)

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
