"""Main script for the lab-poc
"""
import os
import time
import signal
import logging
import argparse
import tempfile
import datetime
import cv2
import importlib
import pkgutil
import traceback as tb

from agent.dpm.classification import load_classifier
from agent.dpm.config import Configuration
from agent.db import DatabaseAdapter
from agent.dpm.storage import LocalStorage
from agent.dpm import DataPipelineManager
from agent.rsync_service import RsyncService
from agent.etr_utils.log import configure_logging, LOG_LEVELS


def parse_args():
    """Parse command line arguments
    """
    parser = argparse.ArgumentParser()
    parser.add_argument('--config', default='factory.json', 
            help='JSON configuration file')
    parser.add_argument('--log', choices=LOG_LEVELS.keys(), default='INFO',
             help='Logging level (df: INFO)')
    parser.add_argument('--log-dir', dest='log_dir', default='logs',
            help='Directory to for log files')
    parser.set_defaults(func=main_agent)

    subparsers = parser.add_subparsers()

    test_classifier = subparsers.add_parser(
            'classifier', help='Test a classifier')

    rs = subparsers.add_parser('rsync-service', help='Run rsync service')
    rs.set_defaults(func=main_etr_sync)

    # Setting up command line parameters for classifier testing
    loader = pkgutil.get_loader('agent.dpm.classification')
    path = [os.path.dirname(loader.get_filename())]
    classifiers = [n for (m, n, ispkg) in pkgutil.walk_packages(path=path)]

    test_classifier.add_argument('classifier', choices=classifiers,
            help='Classifier to test')

    test_classifier.add_argument('input', help='Input image')
    test_classifier.add_argument(
            '--write', default=False, 
            help='Write the result to the specified file name')
    test_classifier.add_argument(
            '--db', default=False, action='store_true',
            help='Insert the results into the database')
    

    test_classifier.set_defaults(func=main_test_classifier)

    # Setting up command line for interfacing with the database
    db_parser = subparsers.add_parser('db', help='Interface with the database')
    db_subparsers = db_parser.add_subparsers()

    # Adding command line for inserting into the database
    ins_parser = db_subparsers.add_parser('insert', 
            help='Insert into the database')
    ins_subparsers = ins_parser.add_subparsers()

    # Parameters for inserting a camera location config
    ins_cam_loc = ins_subparsers.add_parser(
            'cam-loc', help='Insert camera location')
    ins_cam_loc.add_argument('location', help='Location of the camera')
    ins_cam_loc.add_argument('orientation', help='Camera orientation')
    ins_cam_loc.add_argument('direction', help='Directing the camera is facing')
    ins_cam_loc.set_defaults(func=insert_cam_loc)

    # Parameters for inserting a camera position config
    ins_cam_pos = ins_subparsers.add_parser(
            'cam-pos', help='Insert camera position')
    ins_cam_pos.add_argument('zoom', type=float, help='Camera zoom')
    ins_cam_pos.add_argument('tilt', type=float, help='Camera tilt')
    ins_cam_pos.add_argument('fps', type=int, help='Camera FPS')
    ins_cam_pos.set_defaults(func=insert_cam_pos)

    # Parameters for isnert a camera
    ins_cam = ins_subparsers.add_parser('camera', help='Insert a camera')
    ins_cam.add_argument('sn', help='Serial number')
    ins_cam.add_argument('cam_loc_id', type=int, 
            help='ID of the camera location entry')
    ins_cam.add_argument('cam_pos_id', type=int, 
            help='ID of the camera position entry')
    ins_cam.set_defaults(func=insert_camera)

    # Parameters for removing items from the database
    del_parser = db_subparsers.add_parser('delete', help='Remove from database')
    del_parser.add_argument('table', choices=('camera', 'cam-loc', 'cam-pos'),
            help='Table in the database to remove from')
    del_parser.add_argument('id', help='Primary key of the item to delete')
    del_parser.set_defaults(func=delete_from_db)

    # Parameters for listing items in the database
    list_parser = db_subparsers.add_parser('list', help='List items in database')
    list_parser.add_argument('table', choices=('images',))
    list_parser.set_defaults(func=list_db)

    # Parameters for showing an image with its defects from the database
    show_image_parser = subparsers.add_parser(
            'show-image', help='Show an image with its classified defects')
    show_image_parser.add_argument('image_id', help='ID of the image to show')
    show_image_parser.add_argument('--output', default=None, 
            help=('Write the resulting image to the specified file, note this '
                  'will override the file if it exists'))
    show_image_parser.set_defaults(func=show_image)
    
    return parser.parse_args()


def insert_cam_loc(args, log, config):
    """Insert camera location into the database
    """
    log.info('Adding camera location to the database')
    log.info('Inititializing database')
    db = DatabaseAdapter(config.machine_id, config.database)
    log.info('Adding camera location database')
    cam_loc = db.add_camera_location(
            args.location, args.orientation, args.direction)
    log.info('Camera location added:\n%s', cam_loc)


def insert_cam_pos(args, log, config):
    """Insert camera position into the database
    """
    log.info('Adding camera position to the database')
    log.info('Inititializing database')
    db = DatabaseAdapter(config.machine_id, config.database)
    log.info('Adding camera position database')
    cam_pos = db.add_camera_position(args.zoom, args.tilt, args.fps)
    log.info('Camera position added:\n%s', cam_pos)


def insert_camera(args, log, config):
    """Insert camera into the database
    """
    log.info('Adding camera to the database')
    log.info('Inititializing database')
    db = DatabaseAdapter(config.machine_id, config.database)
    log.info('Adding camera database')
    cam = db.add_camera(args.sn, args.cam_loc_id, args.cam_pos_id)
    log.info('Camera added:\n%s', cam)


def delete_from_db(args, log, config):
    """Remove entry from the database
    """
    log.info('Removing entry from the database')
    log.info('Inititializing database')
    db = DatabaseAdapter(config.machine_id, config.database)
    if args.table == 'cam-loc':
        log.info('Removing camera location: %s', args.id)
        db.remove_camera_location(int(args.id))
    elif args.table == 'cam-pos':
        log.info('Removing camera position: %s', args.id)
        db.remove_camera_position(int(args.id))
    elif args.table == 'camera':
        log.info('Removing camera: %s', args.id)
        db.remove_camera(args.id)
    else:
        # This should never happen...
        raise RuntimeError('Unknown table: {}'.format(args.id))


def list_db(args, log, config):
    """List entries in the database.
    """
    log.info('Listing entries in the database')
    log.info('Initializing the database')
    db = DatabaseAdapter(config.machine_id, config.database)
    if args.table == 'images':
        log.info('Listing images in the database')
        images = db.get_all_images()
        if len(images) == 0:
            self.log.info('No images in the database')
        else:
            print('Images:')
            for img in images:
                print('\t{0} : {1}'.format(img.timestamp, img.id))
    else:
        # This should never happen...
        log.error('Known table to list: %s', args.table)


def show_image(args, log, config):
    """Show a given image with its defects written on it.
    """
    log.info('Showing an image')

    log.info('Initializing the database')
    db = DatabaseAdapter(config.machine_id, config.database)

    log.info('Initializing local storage')
    storage = LocalStorage(config.storage)

    log.info('Finding image %s', args.image_id)
    image = db.find_image(args.image_id)

    if image is None:
        log.error('Unable to fine image: %s', args.image_id)
        return -1
    
    log.info('Finding image file')
    img_file = storage.get_image_path(image) 
    storage.stop()
    
    if os.path.exists(img_file):
        log.info('Reading image file: %s', img_file)
        img = cv2.imread(img_file)

        log.info('Drawing defects on the image')
        for defect in image.defects:
            cv2.rectangle(img, defect.tl, defect.br, (0, 255, 0))

        if args.output is not None:
            log.info('Writing the image to: %s', args.output)
            cv2.imwrite(args.output, img)
            log.info('Done writing image')
        else:     
            log.info('Showing the image')
            cv2.namedWindow('Factory Agent', cv2.WINDOW_NORMAL)
            cv2.resizeWindow('Factory Agent', 600, 480)
            cv2.imshow('Factory Agent', img)
            cv2.waitKey(0)
    else:
        log.error('Unable to find image file for image %s', args.image_id)


def main_test_classifier(args, log, config):
    """Main method when testing a classifier.
    """
    log.info('Initializing classifier: %s', args.classifier)

    if args.classifier not in config.classifiers:
        log.error('Classifier configuration missing in config')
        sys.exit(-1)

    classifier = load_classifier(
            args.classifier, 
            config.classification['classifiers'][args.classifier])

    log.info('Loading input image: %s', args.input)
    img = cv2.imread(args.input)
    
    log.info('Classifying the input image')
    defects = classifier.classify(0, img)

    log.info('Drawing results')
    for defect in defects:
        cv2.rectangle(img, defect.tl, defect.br, (0, 255, 0), 2)

    if args.db:
        log.info('Inititializing database')
        db = DatabaseAdapter(config.machine_id, config.database)

        # Getting the test camera that this was "taken" from, creating if it
        # does not exist in the database
        camera = db.find_camera('test-camera')
        if camera is None:
            # Adding dummy location and position
            cam_loc = db.add_camera_location(
                    'on your computer', 'where you are right now', 
                    'pointed at you')
            cam_pos = db.add_camera_position(42, 42, 42)
            camera = db.add_camera('test-camera', cam_loc.id, cam_pos.id)

        log.info('Adding part for the image')
        part = db.add_part()
        log.info('Added part: %s', part)

        log.info('Adding image and defects into the database')
        image = db.add_image(
                part.id, camera.serial_number, img.shape[0], img.shape[1], 
                'png', defects)
        log.info('Added image: %s', image)
    
    if args.write:
        log.info('Saving resulting image to: %s', args.write)
        cv2.imwrite(args.write, img)
    else:
        cv2.imshow(args.classifier, img)
        cv2.waitKey(0)
        cv2.destroyAllWindows()


def main_agent(args, log, config):
    """Main method when the factory agent should fully run.
    """
    log.info('Initializing the factory agent')
    agent = None

    try:
        agent = DataPipelineManager(config)

        def handle_signal(signum, frame):
            log.info('ETR killed...')
            agent.stop()

        signal.signal(signal.SIGTERM, handle_signal)
        agent.run()
    except KeyboardInterrupt:
        log.info('Quitting...')
    except Exception:
        log.error('Error during execution:\n%s', tb.format_exc())
    finally:
        if agent is not None:
            agent.stop()


def main_etr_sync(args, log, config):
    """Main method for running the syncronization process for ETR.
    Note that this is a long running process, which subscribes to the ETR
    MQTT messages, so ETR must be running separately and have MQTT enabled.
    """
    rsync = RsyncService(config)
    try:
        rsync.run()
    except KeyboardInterrupt:
        log.info('Quitting...')
    except Exception:
        log.error('Error during execution:\n%s', tb.format_exc())
    finally:
        rsync.stop()


def main():
    """Main method
    """
    # Parse command line arguments
    args = parse_args()

    # Creating log directory if it does not exist
    if not os.path.exists(args.log_dir):
        os.mkdir(args.log_dir)

    try:
        # Read in the configuration file and initialize needed objects
        config = Configuration(args.config)
    except KeyError as e:
        print('!!! ERROR: Configuration missing key: {}'.format(str(e)))
        return -1

    # Configuring logging
    if config.log_file_size is not None:
        configure_logging(args.log.upper(), 'etr.log', args.log_dir, 
                max_bytes=config.log_file_size)
    else:
        configure_logging(args.log.upper(), 'etr.log', args.log_dir)
    log = logging.getLogger('MAIN')

    args.func(args, log, config)


if __name__ == '__main__':
    main()

