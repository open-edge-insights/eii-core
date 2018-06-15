"""Camera location database model
"""
from sqlalchemy import Integer, String, Column
from sqlalchemy.orm import relationship
from . import Base


class CameraLocation(Base):
    """CameraLocation database model
    """
    __tablename__ = 'camera_location'

    id = Column(Integer, primary_key=True)
    location = Column(String)
    orientation = Column(String)
    direction = Column(String)

    # Relationship with the cameras which have this location 
    cameras = relationship('Camera', back_populates='cam_loc')

    # Relationship with images captured by cameras from this location 
    images = relationship('Image', back_populates='cam_loc')

    def __repr__(self):
        return '<CameraLocation(id={0}, location={1}, orientation={2}, direction={3})>'\
                .format(self.id, self.location, self.orientation,
                        self.direction)

