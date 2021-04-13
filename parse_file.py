import pandas as pd
import numpy as np
import re

data = pd.read_csv('Курсы v.3.0 - Курсы.csv')

data.columns

def parse(line):
    #     print (line)

    #     for sep in ['[', ']']:
    line = line.replace(']', '[')

    for scope in ['(', ')']:
        for i in range(len(line)):
            if line[i] == scope:
                line = line[:i] + '[' + line[i+1:]
                break

    answ = line.split('[')
    if answ[0] == '':
        answ = answ[1:]

    if answ[1][0] == ' ':
        answ[1] = answ[1][1:]

    if answ[1][-1] == ' ':
        answ[1] = answ[1][:-1]
    #         print(line)

    return answ[:3]

def make_dict(vec, level):
    dict_ = {}
    dict_['level'] = level
    dict_['num'] = int(vec[0])
    dict_['title'] = vec[1]

    if len(vec) < 3:
        vec.append('')

    dict_['requirments'] = vec[2]

    return dict_


courses = []

for i in range(1, 6):
    for line in data[f'Уровень {i}']:
        if line is np.nan:
            break

        courses.append(make_dict(parse(line), i))


import json

json_data = json.dumps(courses)

json_data

obj = open('data.txt', 'wb')
obj.write(json_data.encode('utf8'))
obj.close