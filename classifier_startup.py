import subprocess
import argparse
import time

def parse_args():
    """Parse command line arguments
    """
    parser = argparse.ArgumentParser()

    parser.add_argument('--config', dest='config', default='factory.json',
                        help='JSON configuration file')

    parser.add_argument('--log-dir', dest='log_dir', default='logs',
                        help='Directory to for log files')

    return parser.parse_args()

args = parse_args()
subprocess.Popen(["python3.6", "classifier.py", "--config", args.config, "--log-dir", args.log_dir])

print("starting kapacitor...")
subprocess.Popen(["./run_kapacitord.sh"])

print("enabling task")
subprocess.Popen(["./enable_kapacitor_task.sh"])

while(True):
    time.sleep(10)