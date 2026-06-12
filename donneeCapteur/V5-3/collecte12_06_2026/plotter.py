import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

fichier = r"C:\Users\djibril\Desktop\Stage_utiman\donneeCapteur\V5-3\collecte12_06_2026\DATA.CSV"

temps_global = []
mesures = []
pfs = []

offset = 0
last_time = None
pf_courant = None

with open(fichier, "r") as f:

    for ligne in f:

        ligne = ligne.strip()

        if not ligne:
            continue

        if ligne.startswith("pf="):
            pf_courant = float(ligne.split("=")[1])
            continue

        valeurs = [float(x) for x in ligne.split(",")]

        t = valeurs[0]

        # Détection retour arrière horloge
        if last_time is not None and t < last_time:
            offset += last_time

        t_corrige = offset + t

        temps_global.append(t_corrige)
        mesures.append(valeurs[1:])   # tous les capteurs
        pfs.append(pf_courant)

        last_time = t

mesures = np.array(mesures)

# -------------------------
# Couleurs PF
# -------------------------

liste_pf = sorted(set(pfs))
cmap = plt.cm.turbo

couleurs_pf = {
    pf: cmap(i/(len(liste_pf)-1 if len(liste_pf)>1 else 1))
    for i, pf in enumerate(liste_pf)
}

# -------------------------
# Affichage
# -------------------------

plt.figure(figsize=(18,8))

nb_capteurs = mesures.shape[1]

for capteur in range(nb_capteurs):

    for pf in liste_pf:

        idx = np.array(pfs) == pf

        plt.scatter(
            np.array(temps_global)[idx],
            mesures[idx, capteur],
            color=couleurs_pf[pf],
            s=2,
            alpha=0.25
        )

# Légende PF
for pf in liste_pf:
    plt.scatter([], [], color=couleurs_pf[pf], label=f"PF={pf}")

plt.xlabel("Temps reconstruit")
plt.ylabel("Valeur capteur")
plt.title("Tous les capteurs - Couleur selon PF")
plt.grid(True)
plt.legend()
plt.tight_layout()

plt.show()