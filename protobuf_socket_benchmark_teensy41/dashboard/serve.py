import socket
import signal
import sys
from proto.tmp_data_pb2 import TmpData

# tmp = TmpData()
# for i in range(64):
#     tmp.data.append(4095)

# print(tmp.SerializeToString())
# print(len(tmp.SerializeToString()))


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
                    data = conn.recv(15000)
                    if not data:
                        # Client has disconnected
                        print(f"Client {addr} has disconnected")
                        break
                    # Process the data
                    # print(len(data))
                    # tmpData = TmpData()
                    # print(f'len: {len(data)}')
                    # if len(data) == 131:
                    #     print('data: ')
                    #     print(data)
                    #     thing = tmpData.ParseFromString(data)
                    #     print(thing)
                    #     print(tmpData.data)
                except Exception as e:
                    print(f'error OR disconnect: {e}')
                    # Client has disconnected
                    print(f"Client {addr} has disconnected")
                    break
        except Exception as e:
            # An exception occurred while listening/accepting
            print(f'exception: {e}')
            import time
            time.sleep(1)
            pass
