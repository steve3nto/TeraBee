import numpy as np

data = []
filepath = "C:/Users/Stefano/Desktop/Terabee/Code/output_2018-05-19_09-59-22_raw.txt"

with open(filepath, 'r') as file:
    content = file.read()

lines = content.split("\n")
numstr = [line.split("\t")[1:] for line in lines if line != ""]
numbers = np.array(numstr, dtype=np.int32)

print(numbers)

    
    
