import os
import sys
import time
import logging
import argparse
import datetime
import json
from crontab import CronTab
from sqlalchemy import create_engine, update, or_
from algos.etr_utils.log import configure_logging, LOG_LEVELS


class RemoveVisualHmiEntries:
    def removeFiles(self, path):
        """
            Remove the file or directory
        """
        try:
            if os.path.exists(path):
                os.remove(path)
            else:
                self.logger.info("File Not Exists : {}".format(path))
        except OSError as e:
            self.logger.error("Exception Occured : {}".format(e))

    def cleanVisuaHmiImages(self, path, retentiondays):
        """
            Removes files from the passed in path that are older than or equal
            to the number_of_days
        """
        retention_time_in_secs = time.time() - (retentiondays * 24 * 60 * 60)
        for root, dirs, files in os.walk(path, topdown=False):
            if files != []:
                for filename in files:
                    full_path = os.path.join(root, filename)
                    stat = os.stat(full_path)
                    file_lastmodtime = stat.st_mtime
                    if file_lastmodtime <= retention_time_in_secs:
                        self.removeFiles(full_path)
                    else:
                        lastmodtimestamp =\
                            datetime.datetime.fromtimestamp(file_lastmodtime)
                        self.logger.info("This File is not Applicable for\
                        deletion, As Retention policy Says to delete {0}\
                        Days.LastModified TimeStamp is\
                        : {1}".format(retentiondays, lastmodtimestamp))
                self.logger.info("Files Removed Successfully : {}".format(path))
            else:
                self.logger.info("No File Exists in Dir : {}".format(root))

    def cleanVisuaHmiDatabaseEntries(self, retentiondays):
        """
            This helps to remove the database Entries from VisualHmi
        """
        try:
            self.initializePostgreSql(self.dbConfig)
            select_query =\
                "select * from image where timestamp < NOW() - INTERVAL '"\
                + "{} days'".format(retentiondays)
            result_set = self.engine.execute(select_query)
            self.logger.info("Executed Selecting Image Id Entires")
            imgList = []
            for row in result_set:
                imgList.append(row["id"])

            if imgList != []:
                for imgId in imgList:
                    remove_defects_query = "delete from defect where "\
                                            + "image_id = '{}'".format(imgId)
                    self.engine.execute(remove_defects_query)

                self.logger.info("Removed Defects for the retrieved Images")
            else:
                self.logger.info("No Entires found for the given retention \
                                policy : {}".format(retentiondays))

            remove_images_entries =\
                "delete from image where timestamp < NOW() - INTERVAL '{} days'\
                ".format(retentiondays)
            self.engine.execute(remove_images_entries)
            self.logger.info("Removed Image Entires from postgresql")
        except Exception as e:
            self.logger.info('Exception Occured: {}'.format(e))

    def initializePostgreSql(self, config):
        try:
            db_url = 'postgresql://{0}:{1}@{2}:{3}/{4}'.format(\
                    config['username'], config['password'],\
                    config['host'], config['port'],
                    config['database_name'])
        except KeyError as e:
            self.logger.info('Database configuration missing key: {}'.format(e))

        self.engine = create_engine(db_url)

    def startSystemService(self, servicename):
        service_status_cmd = "service " + servicename + " status"
        service_start_cmd = "service " + servicename + " start"
        status_code = os.system(service_status_cmd)
        if status_code != 0:
            self.logger.info("Service is Not Running , Service will be started ")
            stat = os.system(service_start_cmd)
            if stat == 0:
                self.logger.info("Service Started Succesfuly")
            else:
                self.logger.info("Issue in Starting Service")
        else:
            self.logger.info("CronJob Service is running fine")

    def enableCrontabJob(self, path, config):
        try:
            command = "export PYTHONPATH=$PYTHONPATH:{0};/usr/bin/python2.7\
             {1}/VisualHmiCleaner.py -c {1}/config.json -log-dir {1}/logs"\
             .format(config["docker_eta_home"], path)
            cron = CronTab(user='root')
            cron.remove_all(comment="VisualHmiCleaner")
            job = cron.new(command=command, comment='VisualHmiCleaner')
            hrs = config["dailyjobscedulehrs"]
            mins = config["dailyjobscedulemins"]
            job.hour.on(hrs)
            job.minutes.on(mins)
            job.enable()
            cron.write()
            self.logger.info("Job Runs at Every Day (hours:mins) : {0}:{1} "\
                             .format(hrs, mins))
            self.logger.info("Cron Job Enabled Successfully !!")
        except Exception as e:
            self.logger.info("Exception Occured : {}".format(e))

    def main(self, args):
        logging.basicConfig(level=logging.DEBUG,
                            format='%(asctime)s : %(levelname)s : \
                            %(name)s : [%(filename)s] :' +
                            '%(funcName)s : in line : \
                            [%(lineno)d] : %(message)s')

        logging.getLogger("VisualHmiCleaner").setLevel(logging.WARNING)
        self.logger = logging.getLogger(__name__)
        self.args = args
        self.logger.info("VisualHmiCleaner Started")
        with open(self.args.config, 'r') as f:
            self.config = json.load(f)
        hmi_image_folder = self.config["hmi_image_folder"]
        self.dbConfig = self.config["database"]
        eta_dir = self.config["docker_eta_home"]
        self.logger.info("HMI ImageFolder : {}".format(hmi_image_folder))
        self.logger.info("ETA Home Dir for CronJob : {}".format(eta_dir))

        if self.args.enableCron is True:
            self.logger.info("Enabling CronJob")
            self.startSystemService("cron")
            path = str(os.getcwd())+"/VisualHmiClient/VisualHmiCleaner"
            self.enableCrontabJob(path, self.config)
        else:
            retentiondays = self.config["retention_days"]
            self.logger.info("Retention Policy in Days : {}"\
                             .format(retentiondays))
            self.logger.info("VisualHmi Images Folder : {}"\
                             .format(hmi_image_folder))
            self.cleanVisuaHmiImages(hmi_image_folder, retentiondays)
            self.logger.info("Removing VisualHmi Entries from "\
                             + "postgresql Started")
            self.cleanVisuaHmiDatabaseEntries(retentiondays)
            self.logger.info("Removing VisualHmi Entries from "\
                             + "postgresql Finished")
            self.logger.info("HMI ImageFolder : {}".format(hmi_image_folder))
            self.logger.info("Running CronJob Mode")
            self.logger.info("VisualHmiCleaner Finished")


def parse_arg():
    """
        Parsing the Commandline Arguments
    """
    ap = argparse.ArgumentParser()
    ap.add_argument("-c", "--config", default="config.json",
                    help="Please give the config file localtion")
    ap.add_argument("-cron", "--enableCron",
                    action='store_true',
                    help="To Enable CronJob - Please give the path of \
                    ViusalHmiCleaner by default : Current Directory")
    ap.add_argument("-m", "--mode", default="host",
                    help="Please set the running Mode host/docker")
    ap.add_argument('-log', choices=LOG_LEVELS.keys(), default='INFO',
                    help='Logging level (df: INFO)')

    ap.add_argument('-log-dir', dest='log_dir', default='logs',
                    help='Directory to for log files')
    return ap.parse_args()


if __name__ == "__main__":
    args = parse_arg()

    currentDateTime = str(datetime.datetime.now())
    listDateTime = currentDateTime.split(" ")
    currentDateTime = "_".join(listDateTime)
    logFileName = 'visual_hmi_cleaner_' + currentDateTime + '.log'

    if not os.path.exists(args.log_dir):
        os.mkdir(args.log_dir)

    configure_logging(args.log.upper(), logFileName, args.log_dir)

    visualHmiCleaner = RemoveVisualHmiEntries().main(args)

    if args.mode == 'docker':
        while True:
            time.sleep(10000)
            print("Running in CronJobMode Inside Docker")
    else:
        print("Running in Host Machine")
