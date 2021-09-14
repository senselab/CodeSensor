with open('input.txt', 'r') as f, open('output.txt', 'w') as f2:
    cases = int(f.readline())
    for i in range(cases):
        buf = list(map(int, f.readline().split()))
        buf.sort()

        for x in buf:
            f2.write(str(x) + ' ')
        f2.write('\n')
