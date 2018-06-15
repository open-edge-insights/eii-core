"""Camera position model
"""
from sqlalchemy import Integer, String, Column, Float
from sqlalchemy.orm import relationship
from . import Base


class CameraPos(Base):
    """Camera position database model
    """
    __tablename__ = 'camera_pos'
    id = Column(Integer, primary_key=True)
    zoom = Column(Float)
    tilt = Column(Float)
    fps = Column(Integer)

    # Relationship with the cameras which have this position
    cameras = relationship('Camera', back_populates='cam_pos')

    # Relationship with images captured by cameras with this position
    images = relationship('Image', back_populates='cam_pos')

    def __repr__(self):
        return '<CameraPos(id={0}, zoom={1}, tilt={2}, fps={3})>'.format(
                self.id, self.zoom, self.tilt, self.fps)

