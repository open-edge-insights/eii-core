"""Defect database model
"""
from sqlalchemy import Integer, String, Column, ForeignKey
from sqlalchemy.orm import relationship
from . import Base


class Defect(Base):
    """Defect database model
    """
    __tablename__ = 'defect'

    id = Column(Integer, primary_key=True)
    defect_class = Column(Integer)
    
    # Top left (x, y) of defect bounding box
    tl_x = Column(Integer)
    tl_y = Column(Integer)

    # Bottom right (x, y) of the defect bounding box
    br_x = Column(Integer)
    br_y = Column(Integer)

    # Image on which the defect exists
    image_id = Column(String, ForeignKey('image.id'))

    def __init__(self, defect_class, tl, br):
        """Constructor

        Arguments:
            defect_class - String representation of the defect
            tl           - Top left (x, y) tuple for bounding box
            br           - Bottom right (x, y) tuple for bounding box
        """
        self.defect_class = int(defect_class)
        self.tl_x = int(tl[0])
        self.tl_y = int(tl[1])
        self.br_x = int(br[0])
        self.br_y = int(br[1])

    @property
    def tl(self):
        """Helper property for top left (x, y) tuple of the bounding box
        """
        return (self.tl_x, self.tl_y)

    @property
    def br(self):
        """Helper property for bottom right (x, y) tuple of the bounding box
        """
        return (self.br_x, self.br_y)

    def __repr__(self):
        return '<Defect(defect_class={0}, tl={1}, br={2})>'.format(
                self.defect_class, self.tl, self.br)

