#!/usr/bin/python3 -u

import sys
import os
import argparse
from flask import Flask, Response, request
from flask_sock import Sock  # pip install flask-sock
import websocket  # pip install websocket-client
from urllib.request import urlopen, Request
from urllib.error import HTTPError
import queue
import json
import threading
import time

app = Flask(__name__)
sock = Sock(app)
host = None

ws_cache_lock = threading.RLock()
ws_cache = {}
ws_queues = []
ws_thread_queue = queue.Queue()


def ws_thread_fn(q: queue.Queue):
    while True:
        time.sleep(0.1)
        try:
            ws = websocket.create_connection(f'ws://{host}/ws')
        except Exception as e:
            print('create_connection failed:', e)
            continue

        ws.settimeout(1)

        while True:
            try:
                to_send = q.get(timeout=0.1)
                if to_send is None:
                    return
                ws.send(to_send)
            except queue.Empty:
                pass
            except Exception:
                break

            try:
                text = ws.recv()
                lines = text.strip().split("\n")
                msgs = [json.loads(line) for line in lines]
                with ws_cache_lock:
                    for msg in msgs:
                        ws_cache[msg["topic"]] = msg["payload"]

                    for ws_queue in ws_queues:
                        ws_queue.put(text)
            except:
                break


def make_absolute_path(path):
    return os.path.join(os.path.dirname(os.path.realpath(__file__)), path)


@app.route('/')
def index():
    with open(make_absolute_path('build/index.standalone.html'), 'r', encoding='utf-8') as f:
        return f.read()


@app.route('/<path:path>', methods=['GET', 'PUT'])
def forward_html(path):
    try:
        with urlopen(Request(f'http://{host}/{path}', data=request.data, method=request.method)) as f:
            # Exclude Transfer-Encoding: chunked header.  The chunked response is already reassembled.
            return Response(f.read(), headers={x: f.headers[x] for x in f.headers if x != "Transfer-Encoding"})
    except HTTPError as e:
        return Response(e.fp.read(), e.code)


@sock.route('/ws')
def forward_ws(sock):
    q = queue.Queue()
    try:
        with ws_cache_lock:
            initial_data = "\n".join(json.dumps({"topic":topic,"payload":payload}) for topic, payload in ws_cache.items()) + "\n\n"
            sock.send(initial_data)
            ws_queues.append(q)

        while True:
            data = sock.receive(timeout=0.01)

            if data != None:
                ws_thread_queue.send(data)

            try:
                data = q.get(timeout=0.01)
            except queue.Empty:
                pass
            else:
                sock.send(data)
    finally:
        with ws_cache_lock:
            ws_queues.remove(q)


parser = argparse.ArgumentParser()
parser.add_argument('host')

args = parser.parse_args(sys.argv[1:])
host = args.host

thread = threading.Thread(target=ws_thread_fn, args=[ws_thread_queue])
thread.start()

app.run(host="::")

ws_thread_queue.put(None)
