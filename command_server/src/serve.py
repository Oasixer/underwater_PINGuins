from pathlib import Path
from flask import Flask
from flask import json

position_data_file = "blah"

def get_coords():
    with open(path) as f:
        return f.read()

app = Flask(__name__)

@app.route('/get_coords')
def get_coords():
    coords_data = get_coords()
    coords_json_obj = json.loads(coords_data)
    coords_json_str = json.dumps()
    return coords_json_str