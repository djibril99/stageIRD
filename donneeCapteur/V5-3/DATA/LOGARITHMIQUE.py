import numpy as np
import matplotlib.pyplot as plt
from sklearn.linear_model import LinearRegression
from sklearn.metrics import r2_score

FICHIER = r"DATA_DIV.CSV"

R = []
PF = []

pf_courant = None

# =====================
# LECTURE DATA
# =====================
with open(FICHIER, "r") as f:

    for ligne in f:

        ligne = ligne.strip()

        if not ligne:
            continue

        if ligne.lower().startswith("pf="):
            pf_courant = float(ligne.split("=")[1])
            continue

        vals = [float(x) for x in ligne.split(",")]
        capteurs = vals[1:]

        indices_utiles = [2, 3, 4, 5, 8]

        for idx in indices_utiles:
            R.append(capteurs[idx - 1])
            PF.append(pf_courant)

R = np.array(R)
PF = np.array(PF)

print("Nombre de points :", len(R))

# =====================
# NETTOYAGE (important)
# =====================
R = np.clip(R, 1e-9, None)

# =====================
# MODELE LOG (MOINDRES CARRES)
# =====================
X = np.log10(R).reshape(-1, 1)
y = PF

model = LinearRegression()
model.fit(X, y)

a = model.coef_[0]
b = model.intercept_

PF_pred = model.predict(X)

# =====================
# METRICS
# =====================
r2 = r2_score(PF, PF_pred)
rmse = np.sqrt(np.mean((PF - PF_pred)**2))

print("\n===== MODELE PF = a log10(R) + b =====\n")
print(f"a = {a:.6f}")
print(f"b = {b:.6f}")
print("R² =", r2)
print("RMSE =", rmse)

# =====================
# AFFICHAGE
# =====================
plt.figure(figsize=(12,6))

plt.scatter(R, PF, s=5, alpha=0.3, label="Mesures")

R_lisse = np.linspace(np.min(R), np.max(R), 2000)
PF_lisse = a * np.log10(R_lisse) + b

plt.plot(R_lisse, PF_lisse, 'r', linewidth=3, label="Modele log")

plt.xlabel("Resistance")
plt.ylabel("PF")
plt.grid(True)
plt.legend()
plt.show()

# =====================
# FONCTION UTILISATEUR
# =====================
def calculerPf(R_value):
    R_value = max(R_value, 1e-9)
    return a * np.log10(R_value) + b

# =====================
# TEST
# =====================
while True:
    R_input = input("R (q pour quitter) : ")

    if R_input.lower() == "q":
        break

    try:
        R_value = float(R_input)
        print("PF =", calculerPf(R_value))

    except:
        print("Erreur valeur")