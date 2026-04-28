import pandas as pd
import matplotlib.pyplot as plt

# Charger fichier texte exporté du moniteur série
# Exemple: data.txt
with open("capteureau1.txt", "r", encoding="utf-8") as f:
    lines = f.readlines()

times = []
raw_vals = []
filt_vals = []

for line in lines:
    line = line.strip()

    # ignorer ligne vide ou header
    if "Raw,Filtered" in line:
        continue

    # Exemple ligne:
    # 15:26:39.817 -> 1459134.62,1459134.62

    if "->" in line:
        left, right = line.split("->")

        t = left.strip()

        vals = right.strip().split(",")

        if len(vals) == 2:
            try:
                raw = float(vals[0])
                filt = float(vals[1])

                times.append(t)
                raw_vals.append(raw)
                filt_vals.append(filt)

            except:
                pass

# DataFrame
df = pd.DataFrame({
    "Time": times,
    "Raw": raw_vals,
    "Filtered": filt_vals
})

# Plot
plt.figure(figsize=(12,6))

plt.plot(df.index, df["Raw"], label="Raw")
plt.plot(df.index, df["Filtered"], label="Filtered", linewidth=2)

plt.title("Capteur - Données Moniteur Série")
plt.xlabel("Échantillon")
plt.ylabel("Valeur")
plt.grid(True)
plt.legend()

# Affichage temps allégé
step = max(1, len(df)//10)
plt.xticks(range(0, len(df), step), df["Time"][::step], rotation=45)

plt.tight_layout()
plt.show()