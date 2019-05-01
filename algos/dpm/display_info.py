class DisplayInfo:
    """DisplayInfo class
    """
    def __init__(self, info, priority):
        """Constructor

        Arguments:
            info         - Information string to be displayed with the frame
            priority     - Priority of the information [0: low, 1: medium,
                           2: high]
        """

        self.info = info
        self.priority = priority

    def __repr__(self):
        return '<DisplayInfo(info={0}, priority={1})>'.format(
                self.info, self.priority)
