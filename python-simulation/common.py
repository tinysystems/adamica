import json

def init():
    global json_data
    with open('config.json') as config_file:
        json_data = json.load(config_file)