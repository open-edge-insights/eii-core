from collections import namedtuple


defaults = {
    'ImageStore': 'InMemory',
    'url': 'localhost',
    'port': '8086',
    'username': 'root',
    'password': 'root',
    'db': 'datain'
}

value = namedtuple('Settings', defaults.keys())(**defaults)
