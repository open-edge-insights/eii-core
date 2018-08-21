"""
Copyright (c) 2018 Intel Corporation.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
"""

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

