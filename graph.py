import matplotlib.pyplot as plt

# Read the log file
with open("build/bin/log.txt", "r") as f:
    data = f.readlines()

# Process the data
times = []
active_threads = []

for line in data:
    if line.startswith("#"):
        continue
    time, active = line.strip().split()
    times.append(float(time))
    active_threads.append(int(active))

# Create the plot
plt.plot(times, active_threads)
plt.xlabel("Time (s)")
plt.ylabel("Number of Active Threads")
plt.title("Active Threads vs Time")
plt.grid()

# Save the plot as an image file
plt.savefig("active_threads_vs_time.png")

# Show the plot
plt.show()
