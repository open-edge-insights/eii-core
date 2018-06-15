"""Part database object
"""
from sqlalchemy import Integer, String, Column, DateTime, ForeignKey
from sqlalchemy.orm import relationship
from . import Base


class Part(Base):
    """Part database object
    """
    __tablename__ = 'part'

    # Must be a unique UUID assigned to the part
    id = Column(String, primary_key=True)
    timestamp = Column(DateTime)

    # ID of the gateway which processed the part
    gateway_id = Column(String)

    # Relationship with the images of the part
    images = relationship('Image')

    def __repr__(self):
        return '<Part(id={0}, gateway_id={1})>'.format(self.id, self.gateway_id)

