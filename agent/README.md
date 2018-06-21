Alibaba March PoC
===============
> **NOTE:** This code uses Python 3, not 2.7.x

Code base for simple lab setup to showcase feasibility of end-to-end factory
automation system.

## Setup
> **NOTE:** For Clear Linux, see [these](https://github.intel.com/Alibaba-Pathfinding/march-poc/wiki/ETR-Clear-Linux-Step-by-Step-Guide) step-by-step instructions

The setup for this code base requires the following steps:

1. Install OpenCV 3 with Python 3 support
2. Install Python dependencies
3. Install and configure PostgreSQL

### OpenCV

#### Linux
Follow the instructions at [this link](https://docs.opencv.org/trunk/d7/d9f/tutorial_linux_install.html)
for instructions on installing OpenCV on Linux.

#### MacOS

To install OpenCV 3 with Python support Python 3 support execute the following
command:

```sh
$ brew install opencv3 --with-python3
```

### Python Dependencies

The code depends upon a few Python dependencies, to install those dependencies
with pip execute the following command:

```sh
$ sudo -H pip3 install -r requirements.txt
```

### PostgreSQL
PostgreSQL is used as the underlying database used by the factory agent. To
install PostgreSQL run one of the following commands:

#### Linux

```sh
$ sudo apt install postgresql
```

#### MacOS

```sh
$ brew install postgres
```

To configure PostgreSQL, first switch over to the execute the following command
to create the database:

```sh
$ sudo -u postgres createdb ali
```
> **NOTE:** Using `ali` for the database name is just a suggestion, this could be
> any name you want as long as the JSON configuration file for the agent 
> reflects the name of the database correctly.

Next, create the user which the factory agent will use to connect to the 
database. To do this, execute the following command:

```sh
sudo -u postgres createuser -l -P ali 
```
> **NOTE:** Using `ali` as the username is just a recommendation, and is not
> required. Also, note that this command will only give the user access to
> log into database, not to create database (which is good, because it only
> gives the user the access that they need).

Next, in order to connect to the database you must create the `.pgpass` file
which PostgreSQL uses to store user credentials for connecting to databases. 
This file must exist in your home directory. To create this file execute the
following command:

```sh
$ touch ~/.pgpass
```

Once you have created this file, open it with you desired editor. Each line in
the `.pgpass` must follow the following format:

```
hostname:port:database:username:password
```

An example entry for the `.pgpass` file would be:

```
localhost:5432:ali:ali:intel123
```

For ETR, you should use `localhost` for the hostname, the other values depend
on your configuration.

Once you have created the `.pgpass` file and added the login credentials for 
your PostgreSQL database change the permissions for the file to `0600` with the
following command:

```sh
$ chmod 0600 ~/.pgpass
```

ETR will not be able to successfully connect to the database unless the 
permissions are set with command specified above.

Once you have completed all of the above steps, the setup of the factory agent
should be complete.

## Configuration 
The main entry point of the code base is the `factory.py` script. In order to 
use the `factory.py` script, first you must setup the JSON configuration file. 
By default, the script will look in it's current directory for a file
name `factory.json`, if it does not exist an error will occur. You can pass a
different file name through the command line arguments if you wish. The 
configuration file must have the following entries:

```
{
    "machine_id": <STRING: Unique string identifying the machine>,
    "trigger_threads": <INT: (OPTIONAL) Number of threads to use for classification>,
    "log_file_size": <INT: (OPTIONAL) Maximum size for rotating log files>,
    "data_ingestion_manager": {
        "ingestors": {
            "<INGESTOR>": <OBJECT: Ingestor configuration object>
        }
    },
    "storage": {
        "folder": <STRING: Folder to save images to>, 
        "image_format": {
            "type": <STRING: Image encoding type>,
            "config": <OBJECT: Config for the encoding> 
        }
    },
    "triggers": {
        "<TRIGGER>": <OBJECT: Trigger configuration> 
    },
    "classification": {
        "classifiers": {
            "<CLASSIFIER>": {
                "trigger": <STRING: Name of the trigger to use>,
                "config": <OBJECT: Classifier configuration> 
            }
        }
    },
    "database": {
        "username": <STRING: Database username>,
        "host": <STRING: Host running the database>,
        "database_name": <STRING: Name of the postgres db>,
        "port": <INT: Port for the database>
    }
}
```

Once you have your configuration file setup, the last step in configuring the
factory agent is to add your camera's to the database. This can be done through
the `factory.py` script. 

The first two structures which must be inserted into the database is the camera
location and position. The following two commands show case how to accomplish
this.

```sh
# Inserts the camera location
$ python3 factory.py db insert cam-loc example-location example-orientation example-direction

# Inserts the camera position
$ python3 factory.py db insert cam-pos 0 0 0
```

Both of the commands above will will print the ID of the database entry for the
location and position, which must be used when inserting the camera into the
database. In the example below it is assumed that the IDs are `0` and `3` for
the camera location and position respectively.

```sh
$ python3 factory.py db insert camera camera-serial-number 0 3
```

An example configuration named `example-factory.json` is provided in the code
base. For more information on using the example configuration, see 
[this](https://github.intel.com/Alibaba-Pathfinding/march-poc/wiki/Example-Setup) 
wiki page.

The following sections describe each of the major components of the configuration
structure specified above.

### Storage

> **IMPORTANT NOTE:** The `type` configuration value must be set to either `png`
> or `jpeg`.

The storage represents the configuration for the local storage component of the
agent. The important constraint here is that the `type` value must be set to
either `png` or `jpeg`. An error will result otherwise.

The `config` key's value must be another JSON object. The configuration for 
each supported image type are specified below.

#### PNG
The `png` image format type must have the following configuration object:

```
{
    "compression_level": <INTEGER: BETWEEN 0 AND 9>
}
```

Note that the higher the compression level, the smaller the PNG file will be,
however, this will also mean that the compression takes longer.

#### JPEG
The `jpeg` image format type must have the following configuration object:

```
{
    "quality": <INTEGER: BETWEEN 0 AND 100>
}
```

Note that the higher the quality the larger the file, the lower the quality the
smaller the image, however, the quality of the image will be significantly lower.

### Classifiers

Classifiers represent the different classification algorithms to use on the
video frames that are ingested into the agent. 

The the keys in the `classifiers` object in the configuration for the agent
represent the classifier you wish to have the agent load. The value associated
with that key is the configuration for that classifier.

It is important to note that if the specified classifier does not exist, then
the agent will fail to start. In addition, if the configuration for the
classifier is not correct, then the classifier will fail to be loaded.

#### Supported Classifiers

| Classifier | Documentation |
| :--------: | :-----------: |
| Tester     | [Link](https://github.intel.com/Alibaba-Pathfinding/march-poc/wiki/Tester-Classifier) |

### Ingestion

The ingestion portion of the configuration represents the different ingestors
for the agent to load. The `ingestors` key inside of this configuration
represents which ingestors to load into the ingestion pipeline. Each key in
the `ingestors` object represents an ingestor to load and the value associated
with that key is the configuration for that ingestor.

It is important to note that if the specified ingestor does not exist, then
the agent will fail to start. In addition, if the configuration for the
ingestor is not correct, then the classifier will fail to be loaded.

See the documentation for the ingestors you wish to have loaded for how to
configure them.

#### Supported Ingestors

| Ingestor | Documentation |
| :------: | :-----------: |
| Video    | [Link](https://github.intel.com/Alibaba-Pathfinding/march-poc/wiki/Video-Ingestor) |

### Database
The database section provides the needed values for the database adapter connect
to the database and store data.

## Usage

The `factory.py` script has the following command line interface:

```
usage: factory.py [-h] [--config CONFIG] [--log {DEBUG,INFO,ERROR,WARN}]
                  [--log-dir LOG_DIR]
                  {classifier,db,show-image} ...

positional arguments:
  {classifier,db,show-image}
    classifier          Test a classifier
    db                  Interface with the database
    show-image          Show an image with its classified defects

optional arguments:
  -h, --help            show this help message and exit
  --config CONFIG       JSON configuration file
  --log {DEBUG,INFO,ERROR,WARN}
                        Logging level (df: INFO)
  --log-dir LOG_DIR     Directory to for log files
```

## SystemD Service
To use ETR as a SystemD service you must first copy the code base into the
`/opt/etr/` directory. The service assumes that the `factory.py` script can be
found at `/opt/etr/factory.py`. It also assumes that your configuration is at
`/opt/etr/factory.json`. The service will also be ran as root, so you must 
setup the `.pgpass` file specified in the configuration for the root user on
your system.

Once you have completed these steps, you need to install the `etr.service` file 
into systemd. To do that, execute the following commands:

```sh
# Copy etr.service file to the proper etc location
$ sudo cp etr.service /etc/systemd/system

# Reload services into systemd
$ sudo systemctl daemon-reload

# Enable the ETR service
$ sudo systemctl enable etr

# Start the ETR service
$ sudo systemctl start etr
```

The log files for the ETR service will be stored at `/opt/etr/logs/`.

The commands above will have systemd start ETR on boot, if you do not wish to be
the case do not run the `enable` command, but if you did you can disable 
starting the ETR service on boot with the following command:

```sh
$ sudo systemctl disable etr
```

### Starting ETR SystemD Service
```sh
$ sudo systemctl start etr
```

### Stopping ETR SystemD Service
```sh
$ sudo systemctl stop etr
```

### Checking ETR SystemD Service Status
```sh
$ sudo systemctl status etr
```

## Development

For developing classifiers and ingestors see the following documentation:

* [Developing Classifiers](https://github.intel.com/Alibaba-Pathfinding/march-poc/wiki/Developing-Classifiers)
* [Developing Ingestors]()
