with open("README.md") as f: data = f.read()

lines = data.split("\n")
lines = [line.split() for line in lines]
lines = [line for line in lines if len(line) > 3]
lines = [line for line in lines if (line[0] == "#" and line[1].isnumeric())]
lines = [print(line[1], line[3], sep=":", end=" ") for line in lines]
