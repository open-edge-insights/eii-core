"""
Copyright (c) 2018 Intel Corporation.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
"""

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

