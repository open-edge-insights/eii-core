import etcd3
import json

def main():
    etcd = etcd3.client()
    with open("etcd_pre_load.json", 'r') as f:
        config = json.load(f)

    print("=======Adding key/values to etcd========")
    for key, value in config.items():
        if isinstance(value, str):
            etcd.put(key, bytes(value.encode()))
        elif isinstance(value, dict):
            etcd.put(key, bytes(json.dumps(value, indent=4).encode()))

    print("=======Reading key/values to etcd========")
    for key in config.keys():
        value = etcd.get(key)
        print(key, '->', value[0].decode())

main()
