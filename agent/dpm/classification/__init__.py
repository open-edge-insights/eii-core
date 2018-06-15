"""Classification module
"""
import inspect
import importlib


class ClassifierLoadError(Exception):
    """Exception raised by classifiers
    """
    pass


class ClassifierConfigError(Exception):
    """Exception raised if there is an error in the configurtion for a
    classifier.
    """
    pass


def load_classifier(classifier, classifier_config):
    """Load the specified classifier.

    Parameters
    ----------
    classifier : str
        Name of the classifier to attempt to load
    classifier_config : dict
        Configuration obejct for the classifier

    Returns
    -------
    Classifier object, or None if the classifier does not exist

    Exceptions
    ----------
    ClassifierConfigError
        If config keys are missing
    ClassifierLoadError
        If the classifier does not exist or if the classifier implementation is
        missing required components
    """
    try:
        lib = importlib.import_module(
                '.{}'.format(classifier), 
                package='agent.dpm.classification.classifiers')

        arg_names = inspect.getargspec(lib.Classifier.__init__).args
        if len(arg_names) > 0:
            # Skipping the first argument since it is the self argument
            args = [classifier_config[arg] for arg in arg_names[1:]] 
        else:
            args = []

        return lib.Classifier(*args)
    except AttributeError:
        raise ClassifierLoadError(
                '{} module is missing the Classifier class'.format(classifier))
    except ImportError:
        raise ClassifierLoadError(
                'Failed to load classifier: {}'.format(classifier))
    except KeyError as e:
        raise ClassifierConfigError(
                'Classifier config missing key: {}'.format(e))

