import statistics

for x in range(3):
    with open(f'data/moves/{x}.txt', 'r') as file:
        lines = file.readlines()
        try:
            move_sample = [int(line.strip().split(", ")[1].split("=")[1]) for line in lines]
            actuate_sample = [int(line.strip().split(", ")[2].split("=")[1]) for line in lines]
            print(f'Moves={x}')
            print(f'  Move SD: {statistics.stdev(move_sample)}')
            print(f'  Act SD: {statistics.stdev(actuate_sample)}')
        except statistics.StatisticsError:
            pass  # Ignore all zeros for 0 moves.

# Moves=0
#   Move SD: 0.0
#   Act SD: 0.15811388300841897
# Moves=1
#   Move SD: 1436.7722121296829
#   Act SD: 0.3600847357902755
# Moves=2
#   Move SD: 214.52108777798344
#   Act SD: 0.28232985128663995
