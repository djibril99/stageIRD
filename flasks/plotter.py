import serial
import json
import matplotlib.pyplot as plt
from collections import deque

PORT = "COM6"   # adapte
BAUD = 115200
MAX_LEN = 500

ser = serial.Serial(PORT, BAUD, timeout=1)

plt.ion()
fig, ax = plt.subplots()

data_store = {}   # { "a0": deque, "a1": deque, ... }
lines = {}        # { "a0": line_obj, ... }

while True:
    try:
        line = ser.readline().decode(errors="ignore").strip()

        if "{" not in line:
            continue

        json_str = line[line.find("{"):]

        data = json.loads(json_str)

        # Parcourir toutes les clés dynamiquement
        for key, value in data.items():

            # init si nouvelle clé
            if key not in data_store:
                data_store[key] = deque(maxlen=MAX_LEN)
                line_obj, = ax.plot([], [], label=key)
                lines[key] = line_obj
                ax.legend()

            # choisir quoi afficher (raw ou f)
            val = value.get("raw", None)

            if val is not None:
                data_store[key].append(val)

        # update toutes les courbes
        for key in data_store:
            y = data_store[key]
            x = range(len(y))
            lines[key].set_data(x, y)

        ax.set_xlim(0, MAX_LEN)

        # autoscale dynamique
        ax.relim()
        ax.autoscale_view()

        plt.pause(0.01)

    except json.JSONDecodeError:
        continue
    except KeyboardInterrupt:
        break

ser.close()