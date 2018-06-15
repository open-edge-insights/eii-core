"""Camera database model
"""
from sqlalchemy import Table, Integer, String, Column, ForeignKey
from sqlalchemy.orm import relationship
from . import Base


class Camera(Base):
    """Camera database model
    """
    __tablename__ = 'camera'
    serial_number = Column(String, primary_key=True)

    # Camera location relationship
    cam_loc_id = Column(Integer, ForeignKey('camera_location.id'))
    cam_loc = relationship('CameraLocation')

    # Camera position relationship
    cam_pos_id = Column(Integer, ForeignKey('camera_pos.id'))
    cam_pos = relationship('CameraPos')

    # Relationship with it images
    images = relationship('Image', back_populates='camera')

    # ID of the gateway to which the camera is connected
    gateway_id = Column(String)

    def __repr__(self):
        return '<Camera(serial_number={0}, gateway_id={1})>'.format(
                self.serial_number, self.gateway_id)

