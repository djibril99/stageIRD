import serial
import json
import time
import matplotlib.pyplot as plt
from matplotlib.widgets import CheckButtons

# =========================
# SERIAL
# =========================

arduino = serial.Serial("COM6", 115200, timeout=1)

# =========================
# DATA (HISTORIQUE COMPLET)
# =========================

t_data = []

a6_raw, a6_f = [], []
a0_raw, a0_f = [], []

# =========================
# PLOT
# =========================

plt.ion()
fig, ax = plt.subplots()

plt.subplots_adjust(left=0.3)

line_a6_raw, = ax.plot([], [], label="A6 Raw")
line_a6_f, = ax.plot([], [], label="A6 Filtré")
line_a0_raw, = ax.plot([], [], label="A0 Raw")
line_a0_f, = ax.plot([], [], label="A0 Filtré")

ax.set_title("Capteurs résistifs temps réel")
ax.set_xlabel("Temps (s)")
ax.set_ylabel("Résistance (Ω)")
ax.legend()

# =========================
# CHECKBOXES
# =========================

rax = plt.axes([0.02, 0.4, 0.2, 0.2])
labels = ["A6 Raw", "A6 Filtré", "A0 Raw", "A0 Filtré"]
visibility = [True, True, True, True]

check = CheckButtons(rax, labels, visibility)

lines = [line_a6_raw, line_a6_f, line_a0_raw, line_a0_f]

def toggle(label):
    index = labels.index(label)
    lines[index].set_visible(not lines[index].get_visible())
    plt.draw()

check.on_clicked(toggle)

# =========================
# LOOP
# =========================

print("Dashboard actif...")


start = time.time()

while True:

    line = arduino.readline().decode(errors="ignore").strip()

    if not line:
        continue

    try:
        js = json.loads(line)

        t = time.time() - start

        t_data.append(t)

        # ================= A6 =================
        if "a6" in js:
            a6_raw.append(js["a6"]["raw"])
            a6_f.append(js["a6"]["f"])

        # ================= A0 =================
        if "a0" in js:
            a0_raw.append(js["a0"]["raw"])
            a0_f.append(js["a0"]["f"])

        # ================= UPDATE =================

        line_a6_raw.set_data(t_data, a6_raw)
        line_a6_f.set_data(t_data, a6_f)

        line_a0_raw.set_data(t_data, a0_raw)
        line_a0_f.set_data(t_data, a0_f)

        ax.relim()
        ax.autoscale_view()
        

        with open("data_history.txt", "w") as f:
            for i in range(len(t_data)):
                f.write(f"{t_data[i]},{a6_raw[i] if i < len(a6_raw) else ''},{a6_f[i] if i < len(a6_f) else ''},{a0_raw[i] if i < len(a0_raw) else ''},{a0_f[i] if i < len(a0_f) else ''}\n")

        

        plt.pause(0.01)
        #pause de 1mn 

        
    except:
        continue