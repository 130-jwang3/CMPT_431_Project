# Read the data from the file
with open('roadNet-CA.txt', 'r') as f:
    lines = f.readlines()

# Remove the header and split the lines into tuples
data = [tuple(line.strip().split('\t')) for line in lines[1:]]

# Sort the data based on the first element of each tuple (FromNodeId)
sorted_data = sorted(data, key=lambda x: int(x[0]))

# Write the sorted data to a new file
with open('sorted.txt', 'w') as f:
    f.write("# FromNodeId\tToNodeId\n")
    for line in sorted_data:
        f.write('\t'.join(line) + '\n')

print("Sorting completed. Check sorted.txt for the sorted data.")