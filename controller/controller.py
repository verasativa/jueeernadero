import json, serial


class growEnviroment:
    def __init__(self, config, rules):
        with open(config) as file:
            self.config = json.loads(file.read())
        self.rules = rules

