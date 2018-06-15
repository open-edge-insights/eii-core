"""Dummy classifier module
"""


class Classifier:
    """Dummy classifier which never returns any defects. This is meant to aid
    pure data collection without running any classification on ingested video
    frames.
    """
    def __init__(self):
        """Constructor
        """
        # Constructor does nothing...
        pass

    def classify(self, frame_num, img, user_data):
        """Classify the given image.

        Parameters
        ----------
        frame_num : int
            Frame count since the start signal was received from the trigger
        img : NumPy Array
            Image to classify
        user_data : Object
            Extra data passed forward from the trigger

        Returns
        -------
            Always returns an empty list, since this is the dummy classifier
        """
        return []

