"""
Copyright (c) 2018 Intel Corporation.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
"""

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

