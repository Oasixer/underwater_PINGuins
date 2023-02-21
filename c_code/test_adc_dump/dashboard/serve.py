import socket
import signal
import sys
from proto.tmp_data_pb2 import TmpData

HOST = "192.168.1.70"
PORT = 6969  # Port to listen on (non-privileged ports are > 1023)

while True:
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        def signal_handler(sig, frame):
            print('Closing socket...')
            s.close()
            sys.exit(0)
        signal.signal(signal.SIGINT, signal_handler)
        try:
            s.bind((HOST, PORT))
            print('listening')
            s.listen()
            conn, addr = s.accept()
            print(f"Connected by {addr}")
            while True:
                try:
                    data = conn.recv(5000)
                    if not data:
                        # Client has disconnected
                        print(f"Client {addr} has disconnected")
                        break
                    # Process the data
                    # print(len(data))
                    tmpData = TmpData()
                    if len(data) == 33:
                        thing = TmpData.ParseFromString(data)
                        print('data: ')
                        print(thing)
                        print(thing)
                except:
                    # Client has disconnected
                    print(f"Client {addr} has disconnected")
                    break
        except:
            # An exception occurred while listening/accepting
            pass
