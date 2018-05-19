import numpy as np

data = []
filepath = "C:/Users/Stefano/Desktop/MUSIC_HACK/Code/output_2018-05-19_09-15-29.log"

with open(filepath, 'r') as file:
    big_string = str(file.read())
    data = big_string.split('MF')
    print(data)

    
    
