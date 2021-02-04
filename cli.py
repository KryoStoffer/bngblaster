#!/usr/bin/env python3
import sys
import socket
import os
import json

def usage():
    print("""
BNG Blaster Control Socket Client

{c} <socket> <command> [arguments]

Examples: 
    {c} run.sock session-info outer-vlan 1 inner-vlan 1
    {c} run.sock igmp-join outer-vlan 1 inner-vlan 1 group 239.0.0.1 source1 1.1.1.1 source2 2.2.2.2 source3 3.3.3.3
    {c} run.sock igmp-info outer-vlan 1 inner-vlan 1
""".format(c=sys.argv[0]))
    sys.exit(1)

def error(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)
    sys.exit(1)


def main():
    request = {}
    if(len(sys.argv)) < 3:
        usage()

    socket_path = sys.argv[1]

    request["command"] = sys.argv[2]
    if(len(sys.argv)) > 4:
        request["arguments"] = {} 
        for i in range(3, len(sys.argv), 2):
            try: 
                request["arguments"][sys.argv[i]] = int(sys.argv[i+1])
            except:
                request["arguments"][sys.argv[i]] = sys.argv[i+1]

    if os.path.exists(socket_path):
        client = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        try:
            client.connect(socket_path)
            client.send(json.dumps(request).encode('utf-8'))

            data = ""
            while True:
                junk = client.recv(1024)
                if junk:
                    data += junk.decode('utf-8')
                else:
                    break
            print(json.dumps(json.loads(data), indent=4))
        except Exception as e:
            error(e)
        finally:
            client.close()
    else:
        error("socket %s not found" % socket_path)

if __name__ == "__main__":
    main()
