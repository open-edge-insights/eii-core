/*
Copyright (c) 2018 Intel Corporation.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

Explicit permissions are required to publish, distribute, sublicense, and/or sell copies of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

package config

import (
	"github.com/golang/glog"
	"github.com/hashicorp/vault/api"
	"io/ioutil"
	"encoding/json"
	"errors"
)

// DAConfig type exports the config data
type DAConfig struct {
	InfluxDB   influxDBCfg
	Redis      redisCfg
	OutStreams map[string]outStreamCfg
	Opcua      opcuaCfg
}

type influxDBCfg struct {
	Host      string
	Port      string
	DBName    string
	Retention string //use time.ParseDuration to convert this to Duration in golang
	UserName  string
	Password  string
	Ssl       string
	VerifySsl string
}

type redisCfg struct {
	Host      string
	Port      string
	Password  string
	Retention string //use time.ParseDuration to convert this to Duration in golang
}

type outStreamCfg struct {
	DatabusFormat string
}

type opcuaCfg struct {
	Port string
}

// InitVault initializes and read secrets from vault.
func (cfg *DAConfig) InitVault() error {

	VAULT_SECRET_FILE_PATH := "DataAgent/vault_secret_file"

	// Enable the TLS config.
	vlt_config := api.DefaultConfig()

	// Read the secrets and populate the internal data structure.PANI
	vlt_client, err := api.NewClient(vlt_config)

	if err != nil {
		glog.Errorf("DataAgent: failed to create vault client: %s", err)
		return err
	}

	vlt_client.SetAddress("http://localhost:8200")

	//Check for vault is initialized or not.
	sys_obj := vlt_client.Sys()
	vault_initialized, err := sys_obj.InitStatus()
	if (!vault_initialized) || err != nil {
		glog.Errorf("DataAgent: Vault is not initialized yet, please provision it: %s", err)
		return errors.New("Vault Not Initialized")
	} else {

		//Unseal the vault & access using the root token
		//TODO READ FROM tpm HARDWARE.

		var data map[string]interface{}

		byteVal, err := ioutil.ReadFile(VAULT_SECRET_FILE_PATH)
		if err != nil {
			glog.Errorf("dataAgent: failed to open secret file")
			return err
		}

		json.Unmarshal([]byte(byteVal), &data)
		inter := data["keys"].([]interface{})
		unseal_keys := make([]string, len(inter))

		for i, v := range inter {
			unseal_keys[i] = v.(string)
		}

		sys_obj.Unseal(unseal_keys[0])
		vlt_client.SetToken(data["root_token"].(string))

		logical_clnt := vlt_client.Logical()

		//Read the secrets and populate DA-DS :-)
		inflx_secret, err := logical_clnt.Read("secret/influxdb")
		if err != nil || inflx_secret == nil {
			glog.Errorf("DataAgent: Failed to read vault secrets for influxDB: %s", err)
			return errors.New("Failed to read secrets")
		}

		cfg.InfluxDB.Host = inflx_secret.Data["host"].(string)
		cfg.InfluxDB.Port = inflx_secret.Data["port"].(string)
		cfg.InfluxDB.Retention = inflx_secret.Data["retention"].(string)
		cfg.InfluxDB.UserName = inflx_secret.Data["username"].(string)
		cfg.InfluxDB.Password = inflx_secret.Data["password"].(string)
		cfg.InfluxDB.DBName = inflx_secret.Data["dbname"].(string)
		cfg.InfluxDB.Ssl = inflx_secret.Data["ssl"].(string)
		cfg.InfluxDB.VerifySsl = inflx_secret.Data["verify_ssl"].(string)

		//Read Redis secret
		redis_secret, err := logical_clnt.Read("secret/redis")
		if err != nil || redis_secret == nil {
			glog.Errorf("DataAgent: Failed to read vault secrets for Redis: %s", err)
			return errors.New("Failed to read secrets")
		}
		cfg.Redis.Host = redis_secret.Data["host"].(string)
		cfg.Redis.Port = redis_secret.Data["port"].(string)
		cfg.Redis.Retention = redis_secret.Data["retention"].(string)
		cfg.Redis.Password = redis_secret.Data["password"].(string)

		opcua_secret, err := logical_clnt.Read("secret/opcua")
		if err != nil || opcua_secret == nil {
			glog.Errorf("DataAgent: Failed to read vault secrets for OPCUA: %s", err)
			return errors.New("Failed to read secrets")
		}
		cfg.Opcua.Port = opcua_secret.Data["port"].(string)

		stream_secret, err := logical_clnt.Read("secret/OutStreams")
		if err != nil || stream_secret == nil {
			glog.Errorf("DataAgent: Failed to read vault secrets for Out-stream's detail: %s", err)
			return errors.New("Failed to read secrets")
		}

		if stream_secret.Data != nil {
			cfg.OutStreams = make(map[string]outStreamCfg)
			for k, v := range stream_secret.Data {
				a := outStreamCfg{DatabusFormat: v.(string)}
				cfg.OutStreams[k] = a
			}
		}

		//Read all data, Seal the vault.
		err = sys_obj.Seal()
		if err != nil {
			glog.Errorf("Failed to Seal after reading secret, Error: %s", err)
			return errors.New("Failed to Seal")
		}
	}

	return err
}
