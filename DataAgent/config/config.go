/*
Copyright (c) 2018 Intel Corporation.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

Explicit permissions are required to publish, distribute, sublicense, and/or sell copies of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. 
*/

package config

import (
	"bytes"
	"io/ioutil"
	"os"
	"regexp"
	"strings"

	"github.com/BurntSushi/toml"
	"github.com/golang/glog"
)

// DAConfig type exports the config data
type DAConfig struct {
	InfluxDB   influxDBCfg
	Redis      redisCfg
	OutStreams map[string]outStreamCfg
}

type influxDBCfg struct {
	Host      string
	Port      string
	DBName    string
	Retention string //use time.ParseDuration to convert this to Duration in golang
	UserName  string
	Password  string
}

type redisCfg struct {
	Host      string
	Port      string
	Retention string //use time.ParseDuration to convert this to Duration in golang
}

type outStreamCfg struct {
	DatabusFormat string
}

// ParseConfig parses the DA config file and fills up the config structure.
// Also, takes care of parsing the env variables prefixed with $ and
// replacing them with the actual values.
func (cfg *DAConfig) ParseConfig(filepath string) error {

	envVarRe := regexp.MustCompile(`\$\w+`)
	envVarEscaper := strings.NewReplacer(
		`"`, `\"`,
		`\`, `\\`,
	)

	// Read contents of the file
	contents, err := ioutil.ReadFile(filepath)
	if err != nil {
		glog.Errorf("File not found: %s", filepath)
		return err
	}

	// Replace env varialbes with right values
	envVars := envVarRe.FindAll(contents, -1)
	for _, envVar := range envVars {
		envVal, ok := os.LookupEnv(strings.TrimPrefix(string(envVar), "$"))
		if ok {
			envVal = envVarEscaper.Replace(envVal)
			contents = bytes.Replace(contents, envVar, []byte(envVal), 1)
		}
	}

	// Decode the toml formatted config file
	_, err = toml.Decode(string(contents), &cfg)
	if err != nil {
		glog.Errorf("Config file: %s parse failed", filepath)
		return err
	}

	return err
}
