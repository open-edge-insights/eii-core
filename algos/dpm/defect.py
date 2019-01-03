class Defect:
    """Defect class
    """
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