"""Agent database module
"""
import logging
from datetime import datetime
from uuid import uuid4
from sqlalchemy import create_engine 
from sqlalchemy.orm import sessionmaker, joinedload
from sqlalchemy.ext.declarative import declarative_base

Base = declarative_base()

from .defect import Defect
from .camera_location import CameraLocation
from .camera_pos import CameraPos 
from .camera import Camera
from .image import Image
from .part import Part


class DatabaseConfigError(Exception):
    """Exception raise when there is an error in the database configuration.
    """
    pass


class DatabaseAdapter:
    """Database access object used for interfacing with the database.

    The database configuration object must be a dictionary with the following
    (key, value) pairs:

        - host: Host on which the database is running
        - port: Port on the host to connect to the database on
        - username: Username to use when logging into the database
        - password: User's password
        - database_name: Name of the database to use
    """
    def __init__(self, gateway_id, config):
        """Constructor

        Parameters
        ----------
        gateway_id : str
            ID of the gateway the adapter is currently running on
        config : dict
            Database configuration object
        """
        self.log = logging.getLogger(__name__)
        self.gateway_id = gateway_id
        try:
            db_url = 'postgresql://{0}@{1}:{2}/{3}'.format(
                    config['username'], config['host'], config['port'], 
                    config['database_name'])
        except KeyError as e:
            raise DatabaseConfigError(
                    'Database configuration missing key: {}'.format(e))
        self.engine = create_engine(db_url)
        self.db_conn = self.engine.connect()

        Base.metadata.create_all(bind=self.engine)
        
        self.session_maker = sessionmaker(expire_on_commit=False)
        self.session_maker.configure(bind=self.engine)

    def add_part(self):
        """Add a part to the database. This will automatically generate a UUID
        for the part using the uuid4 standard.

        Returns
        ------
        Part
            Part object inserted into the database
        """
        s = self.session_maker()
        try:
            part = Part(id=str(uuid4()), timestamp=datetime.utcnow(),
                    gateway_id=self.gateway_id)
            s.add(part)
            s.commit()
            return part
        except:
            s.rollback()
            raise
        finally:
            s.close()


    def add_image(self, part_id, cam_sn, rows, cols, fmt, defects):
        """Add an image's meta-data to the database with the given defects.
        Note that this method will associate the given defects with the image.

        Parameters
        ----------
        part_id : str
            Unique ID of the part which the image represents
        cam_sn : str
            Serial number of the originating camera 
        row : int
            Height of the image
        cols : int
            Width of the image
        fmt : str
            Format of the saved image (PNG or JPEG)
        defects : list
            List of agent.db.defect.Defect objects

        Returns
        -------
        Image
            Image object inserted into the database.

        Exceptions
        ----------
        AssertionError
            If a camera with the given serian number cannot be found
        """
        # Get a database session
        s = self.session_maker()
        try:
            camera = self.find_camera(cam_sn)
            assert camera is not None, 'No camera with SN: {}'.format(cam_sn)
            img_id = str(uuid4())
            ts = datetime.utcnow()
            filename = '{0}_{1}_{2}_{3}.{4}'.format(
                    self.gateway_id,part_id, img_id, ts, fmt)
            img = Image(id=img_id, filename=filename, rows=int(rows), 
                    cols=int(cols), format=fmt, timestamp=ts, cam_sn=cam_sn, 
                    cam_loc_id=camera.cam_loc_id, cam_pos_id=camera.cam_pos_id,
                    part_id=part_id)
            s.add(img)
            for defect in defects:
                defect.image_id = img.id
            # Attempt to add all defects and the image and commit the changes
            s.add_all(defects)
            img.defects += defects
            s.commit()
            return img
        except:
            s.rollback()  # If an error occurs, rollback all changes
            raise
        finally:
            s.close()  # Always close the session, if an error occurs

    def add_camera_location(self, location, orientation, direction):
        """Add a camera location configuration to the database.

        Parameters
        ----------
        location : str
            String description of the location of the camera
        orientation : str
            String description of the orientation of the camera
        direction : str
            String description of the direction the camera is facing

        Returns
        -------
        CameraLocation
            Database object created while inserting into the database
        """
        s = self.session_maker()
        try:
            cam_loc = CameraLocation(
                    location=location, 
                    orientation=orientation, 
                    direction=direction)
            s.add(cam_loc)
            s.commit()
            return cam_loc 
        except:
            s.rollback()
            raise
        finally:
            s.close()

    def add_camera_position(self, zoom, tilt, fps):
        """Add camera position configuration to the database.

        Parameters
        ----------
        zoom : float 
            Zoom of the camera
        tilt : float
            Tilt angle of the camera
        fps : int
            Frames per second of the camera

        Returns
        -------
        CameraPos
            Databsae object created while inserting into the database
        """
        s = self.session_maker()
        try:
            cam_pos = CameraPos(zoom=zoom, tilt=tilt, fps=fps) 
            s.add(cam_pos)
            s.commit()
            return cam_pos
        except:
            s.rollback()
            raise
        finally:
            s.close()

    def add_camera(self, serial_number, cam_loc_id, cam_pos_id):
        """Add a camera to the database.
        
        Parameters
        ----------
        serial_number : str
            Unique serial number of the camera
        cam_loc_id : int 
            Location of the camera
        cam_pos_id : int 
            Position of the camera

        Returns
        -------
        Camera
            Database object created while inserting into the database
        """
        s = self.session_maker()
        try:
            camera = Camera(
                    serial_number=serial_number, 
                    cam_loc_id=cam_loc_id, 
                    cam_pos_id=cam_pos_id,
                    gateway_id=self.gateway_id)
            s.add(camera)
            s.commit()
            return camera
        except:
            s.rollback()
            raise
        finally:
            s.close()

    def remove_camera_location(self, cam_loc_id):
        """Remove a camera location from the database.
        
        Parameters
        ----------
        cam_loc_id : int
            ID of the camera location entry in the datbase

        Exceptions
        ----------
        AssertionError
            If a camera location with the given ID does not exist
        """
        s = self.session_maker()
        try:
            cam_loc = self.find_camera_location(cam_loc_id)
            assert cam_loc is not None
            s.delete(cam_loc)
            s.commit()
        except:
            s.rollback()
            raise
        finally:
            s.close()

    def remove_camera_position(self, cam_pos_id):
        """Remove a camera position from the database.
        
        Parameters
        ----------
        cam_pos_id : int
            ID of the camera position entry in the datbase

        Exceptions
        ----------
        AssertionError
            If a camera position with the given ID does not exist
        """
        s = self.session_maker()
        try:
            cam_pos = self.find_camera_position(cam_pos_id)
            assert cam_pos is not None
            s.delete(cam_pos)
            s.commit()
        except:
            s.rollback()
            raise
        finally:
            s.close()

    def remove_camera(self, serial_number):
        """Remove a camera from the database.
        
        Parameters
        ----------
        serial_number : str 
            Serial number of the camera entry in the datbase

        Exceptions
        ----------
        AssertionError
            If a camera with the given serial number does not exist
        """
        s = self.session_maker()
        try:
            cam = self.find_camera(serial_number)
            assert cam is not None
            s.delete(cam)
            s.commit()
        except:
            s.rollback()
            raise
        finally:
            s.close()

    def find_camera_location(self, cam_loc_id):
        """Find the camera location entry with the given ID.

        Parameters
        ----------
        cam_loc_id : int
            Camera location's ID

        Returns
        -------
        CameraLocation
            Camera location object, or None if it does not exist
        """
        s.self.session_maker()
        cam_loc = s.query(CameraLocation).filter_by(id=cam_loc_id).first()
        s.close()
        return cam_loc

    def find_camera_position(self, cam_pos_id):
        """Find the camera position entry with the given ID.

        Parameters
        ----------
        cam_pos_id : int
            Camera position's ID

        Returns
        -------
        CameraPos
            Camera position object, or None if it does not exist
        """
        s.self.session_maker()
        cam_pos = s.query(CameraPos).filter_by(id=cam_pos_id).first()
        s.close()
        return cam_pos
    
    def find_camera(self, serial_number):
        """Find the camera in the database with the given serial number.

        Parameters
        ----------
        serial_number : str
            Serial number of the camera to find
        
        Returns
        -------
        Camera
            Camera object, or None if it doesn't exist.
        """
        s = self.session_maker()
        cam = s.query(Camera).filter_by(serial_number=serial_number).first()
        s.close()
        return cam

    def get_all_images(self):
        """Get all of the images in the database.
        """
        s = self.session_maker()
        images = s.query(Image).all()
        s.close()
        return images

    def find_image(self, image_id):
        """Get the database entry for the specified image.

        Parameters
        ----------
        image_id : str
            String ID of the image

        Returns
        -------
        Image
            Image datbase object or None if it does not exist
        """
        s = self.session_maker()
        image = s.query(Image).filter_by(id=image_id).options(joinedload('defects')).first()
        s.close()
        return image

