import random
import sys

letters = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
try:
    filename, n_tuples = sys.argv[1:3]
    n = int(n_tuples)
except:
    print ('Usage: %s <outfile> <number of tuples>' % sys.argv[0])
    exit()

f = open(filename, 'w')
attributes = []

for _ in range(n):
    for a in range(100):
    	attr = [random.choice(letters) for i in range(10)]
    	attributes.append(''.join(attr))

f.write( ','.join(attributes) )
    
print ("Generated %d random tuples in %s." % (n, filename))   