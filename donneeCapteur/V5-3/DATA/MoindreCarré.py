import numpy as np
from sklearn.metrics import r2_score
import matplotlib.pyplot as plt

FICHIER = r"DATA_DIV.CSV"

R = []
PF = []

pf_courant = None

with open(FICHIER, "r") as f:

    for ligne in f:

        ligne = ligne.strip()

        if not ligne:
            continue

        if ligne.lower().startswith("pf="):

            pf_courant = float(
                ligne.split("=")[1]
            )

            continue

        vals = [float(x) for x in ligne.split(",")]

        # suppression temps
        capteurs = vals[1:]

        # capteurs utilisés :
        # capteur1 = référence -> ignoré
        # capteur6 et 7 -> ignorés
        indices_utiles = [ 2, 3, 4,5, 8]

        for idx in indices_utiles:
            R.append(capteurs[idx-1])
            PF.append(pf_courant)

R = np.array(R)
PF = np.array(PF)

print("Nombre de points :", len(R))

# =====================
# Ajustement polynomial
# =====================

DEGRE = 3

coef = np.polyfit(
    R,
    PF,
    DEGRE
)

poly = np.poly1d(coef)

PF_pred = poly(R)

r2 = r2_score(
    PF,
    PF_pred
)

rmse = np.sqrt(
    np.mean(
        (PF - PF_pred)**2
    )
)

print("\n===== MODELE =====\n")

print(poly)

print("\nR² =", r2)
print("RMSE =", rmse)

print("\nCoefficients :")

for i, c in enumerate(coef):

    print(
        f"a{i} = {c:.12e}"
    )

# =====================
# Affichage
# =====================

plt.figure(figsize=(12,6))

plt.scatter(
    R,
    PF,
    s=5,
    alpha=0.3,
    label="Mesures"
)

R_lisse = np.linspace(
    0,
    np.max(R),
    2000
)

plt.plot(
    R_lisse,
    poly(R_lisse),
    'r',
    linewidth=3,
    label="Modele"
)

plt.xlabel("Resistance")
plt.ylabel("PF")
plt.grid(True)
plt.legend()

plt.show()