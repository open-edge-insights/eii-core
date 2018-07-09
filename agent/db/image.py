"""Image database model
"""
from sqlalchemy import Integer, String, Column, DateTime, ForeignKey
from sqlalchemy.orm import relationship
from . import Base


class Image(Base):
    """Image database model
    """
    __tablename__ = 'image'

    id = Column(String, primary_key=True)
    filename = Column(String)

    # Size of the image, i.e. 128x128
    cols = Column(Integer)
    rows = Column(Integer)

    format = Column(String)
    timestamp = Column(DateTime)
    
    # List of defects relationship
    defects = relationship('Defect', cascade='all,delete')

    # Relationship with the camera that captured the image
    cam_sn = Column(String, ForeignKey('camera.serial_number'))
    camera = relationship('Camera')

    # The relationship with the camera location and position are separated out
    # since the relation ship between them and the camera may change as time
    # progresses

    # Camera location relationship
    cam_loc_id = Column(Integer, ForeignKey('camera_location.id'))
    cam_loc = relationship('CameraLocation')

    # Camera position relationship
    cam_pos_id = Column(Integer, ForeignKey('camera_pos.id'))
    cam_pos = relationship('CameraPos')

    # Relationship with the part that this image is a picture of
    part_id = Column(String, ForeignKey('part.id'))
    part = relationship('Part')

    def __repr__(self):
        return ('<Image(id={0}, filename={1}, size=({2},{3}), format={4}, '
                'timestamp={5}, defects={6}, cam_sn={7}, cam_loc_id={8}, '
                'cam_pos_id={9}, part_id={10}>').format(
                        self.id, self.filename, self.rows, self.cols,
                        self.format, self.timestamp, self.defects, self.cam_sn,
                        self.cam_loc_id, self.cam_pos_id, self.part_id)

