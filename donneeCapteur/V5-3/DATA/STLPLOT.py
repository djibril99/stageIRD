import streamlit as st
import numpy as np
import plotly.graph_objects as go
from plotly.subplots import make_subplots
import pandas as pd

# =========================
# Configuration page
# =========================

st.set_page_config(layout="wide")

# =========================
# Paramètres
# =========================

PERIODE_S = 30  # période d'acquisition en secondes

# Noms personnalisés pour les 8 capteurs (indices de 1 à 8)
NOMS_CAPTEURS = (
    "Réference ",
    "Capteur 1",
    "Capteur 2",
    "Capteur 3",
    "Capteur 4",
    "Capteur 5",
    "Capteur 6",
    "WaterMark"
)

# =========================
# Lecture du fichier
# =========================

#fichier = r"C:\Users\djibril\Desktop\Stage_utiman\donneeCapteur\V5-3\DATA\DATA_DIV.csv"
fichier = r"C:\Users\djibril\Desktop\Stage_utiman\donneeCapteur/clonedu08072026/DATAajuster.csv"

mesures = []
pfs = []

pf_courant = None

with open(fichier, "r") as f:

    for ligne in f:

        ligne = ligne.strip()

        if not ligne:
            continue

        if ligne.lower().startswith("pf="):
            pf_courant = float(ligne.split("=")[1])
            continue

        

        valeurs = [float(x) for x in ligne.split(",")]

        for i in range(1, len(valeurs)):
            #valeurs[i] = valeurs[i] / 10.0
            valeurs[i] = valeurs[i] 
        mesures.append(valeurs[1:])
        pfs.append(pf_courant)

mesures = np.array(mesures)
nb_capteurs = mesures.shape[1]

# =========================
# Interface
# =========================

st.title("Analyse des capteurs")

mode = st.radio(
    "Mode d'affichage",
    [
        "Tous les capteurs",
        "Un graphe par capteur"
    ]
)

# Par défaut, on exclut les capteurs 6 et 7 (indices 5 et 6 dans une liste commençant à 0)
options_capteurs = list(range(nb_capteurs))
default_capteurs = [i for i in options_capteurs if i not in (5, 6)]

capteurs_selectionnes = st.multiselect(
    "Capteurs à afficher",
    options=options_capteurs,
    default=default_capteurs,
    format_func=lambda x: NOMS_CAPTEURS[x] if x < len(NOMS_CAPTEURS) else f"Capteur {x+1}"
)

decimation = st.slider(
    "Réduction du nombre de points",
    min_value=1,
    max_value=100,
    value=79
)

window = st.slider(
    "Lissage (moyenne glissante)",
    min_value=1,
    max_value=100,
    value=18
)



# =========================
# Décimation & Exclusion des 10 derniers points
# =========================

mesures_decimees = mesures[::decimation]
pfs_decimes = np.array(pfs)[::decimation]

# On retire les 10 derniers points des courbes
dernierpoint =      5
mesures_aff = mesures_decimees[:-dernierpoint]
pfs_aff = pfs_decimes[:-dernierpoint]




temps_aff = (
    np.arange(len(mesures_aff))
    * decimation
    * PERIODE_S
    / 3600.0
)

# =========================
# Couleurs PF
# =========================

palette = [
    "#1f77b4",
    "#ff7f0e",
    "#2ca02c",
    "#d62728",
    "#9467bd",
    "#8c564b",
]

pf_uniques = sorted(set(pfs_aff))

couleur_pf = {
    pf: palette[i % len(palette)]
    for i, pf in enumerate(pf_uniques)
}

# =========================
# Détection changements PF
# =========================

changements = [0]

for i in range(1, len(pfs_aff)):
    if pfs_aff[i] != pfs_aff[i - 1]:
        changements.append(i)

changements.append(len(pfs_aff))

# =========================
# MODE 1
# =========================


if mode == "Tous les capteurs":

    fig = go.Figure()

    for capteur in capteurs_selectionnes:

        y = mesures_aff[:, capteur]

        if window > 1:
            y = np.convolve(
                y,
                np.ones(window) / window,
                mode="same"
            )

        ##supprimer les 10 derniers points pour éviter les valeurs aberrantes
        y = y[:-dernierpoint]
        temps_aff = temps_aff[:-dernierpoint]

        nom_trace = NOMS_CAPTEURS[capteur] if capteur < len(NOMS_CAPTEURS) else f"Capteur {capteur+1}"

        fig.add_trace(
            go.Scatter(
                x=temps_aff,
                y=y,
                mode="lines",
                name=nom_trace,
                line=dict(width=2)
            )
        )

    for k in range(len(changements) - 1):

        d = changements[k]
        f = changements[k + 1]

        # Gestion de la sécurité sur l'index de fin si moins de données disponibles
        idx_f = min(f - 1, len(temps_aff) - 1)
        idx_d = min(d, len(temps_aff) - 1)

        if idx_d < idx_f:
            fig.add_vrect(
                x0=temps_aff[idx_d],
                x1=temps_aff[idx_f],
                fillcolor=couleur_pf[pfs_aff[idx_d]],
                opacity=0.08,
                line_width=0,
                annotation_text=f"PF={pfs_aff[idx_d]}",
                annotation_position="top left",
                annotation_textangle=-90
            )

    fig.update_layout(
        title="Evolution des résistances capteurs dans le temps",
        height=800,
        hovermode="x unified",
        template="plotly_dark",
        xaxis_title="Temps (heures)"
    )

    st.plotly_chart(fig, use_container_width=True)

# =========================
# MODE 2
# =========================

else:

    if len(capteurs_selectionnes) == 0:
        st.warning("Sélectionnez au moins un capteur.")
        st.stop()

    titres_subplots = [
        NOMS_CAPTEURS[c] if c < len(NOMS_CAPTEURS) else f"Capteur {c+1}"
        for c in capteurs_selectionnes
    ]

    fig = make_subplots(
        rows=len(capteurs_selectionnes),
        cols=1,
        shared_xaxes=True,
        vertical_spacing=0.02,
        subplot_titles=titres_subplots
    )

    for row, capteur in enumerate(capteurs_selectionnes, start=1):

        y = mesures_aff[:, capteur]

        if window > 1:
            y = np.convolve(
                y,
                np.ones(window) / window,
                mode="same"
            )

        fig.add_trace(
            go.Scatter(
                x=temps_aff,
                y=y,
                mode="lines",
                showlegend=False
            ),
            row=row,
            col=1
        )

    for idx in changements[1:-1]:
        if idx < len(temps_aff):
            fig.add_vline(
                x=temps_aff[idx],
                line_dash="dash",
                line_width=1
            )

    fig.update_layout(
        title="Un graphe par capteur",
        height=250 * len(capteurs_selectionnes),
        template="plotly_dark"
    )

    fig.update_xaxes(title_text="Temps (heures)")

    st.plotly_chart(fig, use_container_width=True)







    # =========================
# Création du DataFrame CSV
# =========================

from io import StringIO
from datetime import timedelta

buffer = StringIO()

pf_prec = None

for i in range(len(mesures_aff)):

    # Changement de PF
    if pfs_aff[i] != pf_prec:
        buffer.write(f"PF={pfs_aff[i]}\n")
        pf_prec = pfs_aff[i]

    # Temps au format HH:MM:SS
    temps = str(timedelta(seconds=i * decimation * PERIODE_S))

    # Ligne de données
    ligne = [temps] + [str(v) for v in mesures_aff[i]]

    buffer.write(",".join(ligne) + "\n")

st.download_button(
    label="💾 Sauvegarder les données (CSV)",
    data=buffer.getvalue(),
    file_name="mesures_capteurs.csv",
    mime="text/csv"
)