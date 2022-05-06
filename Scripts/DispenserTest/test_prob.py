with open('data/readings.txt', 'r') as file:
    lines = file.readlines()

    # [occurrences, accumulated time, multiplier]
    arr = [[0, 0, 2], [0, 0, 4], [0, 0, 3]]
    for line in lines:
        split = line.strip().split(", ")
        moves = int(split[0].split("=")[1])
        arr[moves][0] += 1
        arr[moves][1] += int(split[3].split("=")[1])
    avg_time = sum([x[1] / x[0] * x[2] for x in arr]) / 9 / 1000
    print(f"Actual average: {avg_time} seconds")

# Actual average: 32.89694814814815 seconds
