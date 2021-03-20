import fileinput
import textwrap

moves = []

for line in fileinput.input():
    print(line.rstrip())
    if line.startswith('  '):
        moves.append(line[2:4])

if moves:
    moves.reverse()
    print("\n" + "\n".join(textwrap.wrap(" ".join(moves))))

