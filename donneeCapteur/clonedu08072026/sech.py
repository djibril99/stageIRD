import pandas as pd
import matplotlib.pyplot as plt


# fichier CSV
fichier = r"C:\Users\djibril\Desktop\Stage_utiman\donneeCapteur\clonedu08072026\SEC26.CSV"

df = pd.read_csv(fichier)


# Temps : une ligne = 0.5 heure
temps = range(len(df))
temps = [i * 0.5 for i in temps]
            

# Récupération des colonnes
cap1 = df.iloc[:,1]
cap2 = df.iloc[:,2]


# =========================
# 1) Capteur 1 et Capteur 2
# =========================

plt.figure(figsize=(10,5))

plt.plot(temps, cap1, label="Capteur 1 brut")
plt.plot(temps, cap2, label="Capteur 2 brut")

plt.xlabel("Temps (h)")
plt.ylabel("Résistance")
plt.grid()
plt.legend()
plt.title("Capteur 1 et Capteur 2")
plt.show()


# =========================
# 2) Rapport Cap1 / Cap2
# =========================

rapport = cap1 / cap2

plt.figure(figsize=(10,5))

plt.plot(temps, rapport)

plt.xlabel("Temps (h)")
plt.ylabel("R Cap1 / R Cap2")
plt.grid()
plt.title("Rapport Capteur 1 / Capteur 2")
plt.show()


# =========================
# 3) Capteur 1 seul
# =========================

plt.figure(figsize=(10,5))

plt.plot(temps, cap1)

plt.xlabel("Temps (h)")
plt.ylabel("Résistance")
plt.grid()
plt.title("Capteur 1 brut")
plt.show()