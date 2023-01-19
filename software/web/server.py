#!/usr/bin/env python3

import sys
import os
import argparse
from flask import Flask, Response
from flask_sock import Sock # pip install flask-sock
from simple_websocket import ws as WS
import websocket # pip install websocket-client
from urllib.request import urlopen

app = Flask(__name__)
sock = Sock(app)
host = None

def make_absolute_path(path):
    return os.path.join(os.path.dirname(os.path.realpath(__file__)), path)

@app.route('/')
def index():
    with open(make_absolute_path('build/index.standalone.html'), 'r') as f:
        return f.read()

@app.route('/<path:path>')
def forward_html(path):
    with urlopen(f'http://{host}/{path}') as f:
        # Exclude Transfer-Encoding: chunked header.  The chunked response is already reassembled.
        return Response(f.read(), headers={x: f.headers[x] for x in f.headers if x != "Transfer-Encoding"})

@sock.route('/ws')
def forward_ws(sock):
    try:
        ws = websocket.create_connection(f'ws://{host}/ws')
    except:
        print('create_connection failed')
        sock.close()
        return

    ws.settimeout(0.01)

    while True:
        try:
            data = sock.receive(timeout=0.01)
        except WS.ConnectionClosed:
            print('sock.receive failed')
            ws.close()
            return

        if data != None:
            ws.send(data)

        try:
            data = ws.recv()
        except:
            pass
        else:
            sock.send(data)

parser = argparse.ArgumentParser()
parser.add_argument('host')
args = parser.parse_args(sys.argv[1:])
host = args.host

app.run(host="0.0.0.0")
