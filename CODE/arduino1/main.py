import serial
import matplotlib.pyplot as plt

# =========================
# SERIAL
# =========================

arduino = serial.Serial("COM6", 115200, timeout=1)

# =========================
# DATA
# =========================

raw_data = []
filtered_data = []

# =========================
# PLOT
# =========================

plt.ion()
fig, ax = plt.subplots()

line1, = ax.plot([], [], label="Raw")
line2, = ax.plot([], [], label="Filtered")

ax.set_title("Capteur en temps réel")
ax.set_xlabel("Temps")
ax.set_ylabel("resistance en ohms")
ax.legend()

# =========================
# LOOP
# =========================

print("Lecture en cours...")

while True:
    line = arduino.readline().decode(errors="ignore").strip()

    if not line or "," not in line:
        continue

    try:
        raw, filtered = line.split(",")

        raw = float(raw)
        filtered = float(filtered)

        raw_data.append(raw)
        filtered_data.append(filtered)

        # limite mémoire
        if len(raw_data) > 200:
            raw_data.pop(0)
            filtered_data.pop(0)

        # update plot
        line1.set_xdata(range(len(raw_data)))
        line1.set_ydata(raw_data)

        line2.set_xdata(range(len(filtered_data)))
        line2.set_ydata(filtered_data)

        ax.relim()
        ax.autoscale_view()

        plt.pause(0.01)

    except:
        continue