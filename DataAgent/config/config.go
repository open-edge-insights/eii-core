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
