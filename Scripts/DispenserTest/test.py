with open('data/readings.txt', 'r') as file:
    lines = file.readlines()

    totalTime = actTime = moveTime = 0
    for line in lines:
        split = line.strip().split(", ")
        actTime += int(split[2].split("=")[1])
        moveTime += int(split[1].split("=")[1])
        totalTime += int(split[3].split("=")[1])
    print(f"Avg Act Time: {actTime / len(lines) / 1000.0} seconds")
    print(f"Avg Move Time: {moveTime / len(lines) / 1000.0} seconds")
    print(f"Avg Total Time: {totalTime / len(lines) / 1000.0} seconds")

# Avg Act Time: 22.783048387096773 seconds
# Avg Move Time: 7.874637096774193 seconds
# Avg Total Time: 30.657693548387098 seconds
