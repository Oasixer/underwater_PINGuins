import socket
import signal
import sys
import time
import datetime

HOST = "192.168.1.70"
PORT = 6969

debug = False
measure_speed_every_n_packets = 100

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
            n=0
            speed_measure_bytes=0
            begin_speed_measure_ts = datetime.datetime.now()
            while True:
                try:
                    data = conn.recv(10000)
                    if not data:
                        # Client has disconnected
                        print(f"Client {addr} has disconnected")
                        break

                    if measure_speed_every_n_packets is not None:
                        speed_measure_bytes += len(data)
                        if n % measure_speed_every_n_packets == 0 and n > 0:
                            cur = datetime.datetime.now()
                            elapsed = cur - begin_speed_measure_ts
                            rate = speed_measure_bytes / ((elapsed.total_seconds()+0.0000001) * 1000)
                            print(f'rate({measure_speed_every_n_packets}): {rate} kB/s')
                            begin_speed_measure_ts = datetime.datetime.now()
                            speed_measure_bytes = 0

                    n += 1
                    if debug:
                        print(len(data))
                except Exception as e:
                    print(f'error (disconnect?): {e}')
                    break
        except Exception as e:
            # An exception occurred while listening/accepting
            print(f'Exception: {e}')
            time.sleep(1)
            pass
