import streamlit as st
import numpy as np
import pandas as pd
import plotly.graph_objects as go
from plotly.subplots import make_subplots

# =========================
# Lecture fichier
# =========================

fichier = r"C:\Users\djibril\Desktop\Stage_utiman\donneeCapteur\V5-3\collecte12_06_2026\DATA.CSV"

temps = []
mesures = []
pfs = []

offset = 0
last_time = None
pf = None

with open(fichier) as f:

    for ligne in f:

        ligne = ligne.strip()

        if not ligne:
            continue

        if ligne.lower().startswith("pf="):
            pf = float(ligne.split("=")[1])
            continue

        vals = [float(x) for x in ligne.split(",")]

        t = vals[0]

        # Gestion reboot horloge
        if last_time is not None and t < last_time:
            offset += last_time

        temps.append(offset + t)
        mesures.append(vals[1:])
        pfs.append(pf)

        last_time = t

temps = np.array(temps)
mesures = np.array(mesures)

nb_capteurs = mesures.shape[1]

# =========================
# Interface Streamlit
# =========================

st.set_page_config(
    page_title="Validation Capteurs",
    layout="wide"
)

st.title("📈 Validation des capteurs")

# =========================
# Couleurs PF
# =========================

pf_uniques = sorted(set(pfs))

palette = [
    "#1f77b4",
    "#ff7f0e",
    "#2ca02c",
    "#d62728",
    "#9467bd",
    "#8c564b",
    "#e377c2",
    "#7f7f7f",
]

couleurs_pf = {
    pf: palette[i % len(palette)]
    for i, pf in enumerate(pf_uniques)
}

# =========================
# Segments PF
# =========================

changements = [0]

for i in range(1, len(pfs)):
    if pfs[i] != pfs[i - 1]:
        changements.append(i)

changements.append(len(pfs))

# =========================
# Figure
# =========================

fig = make_subplots(
    rows=nb_capteurs,
    cols=1,
    shared_xaxes=True,
    vertical_spacing=0.01,
    subplot_titles=[
        f"Capteur {i+1}"
        for i in range(nb_capteurs)
    ]
)

for capteur in range(nb_capteurs):

    for k in range(len(changements)-1):

        d = changements[k]
        f = changements[k+1]

        pf_segment = pfs[d]

        fig.add_trace(
            go.Scatter(
                x=temps[d:f],
                y=mesures[d:f, capteur],
                mode="lines",
                line=dict(
                    color=couleurs_pf[pf_segment],
                    width=2
                ),
                name=f"PF={pf_segment}",
                legendgroup=f"PF={pf_segment}",
                showlegend=(capteur == 0)
            ),
            row=capteur+1,
            col=1
        )

# =========================
# Lignes verticales PF
# =========================

for idx in changements[1:-1]:

    fig.add_vline(
        x=temps[idx],
        line_width=1,
        line_dash="dash",
        line_color="white"
    )

# =========================
# Layout
# =========================

fig.update_layout(
    height=max(800, nb_capteurs * 220),
    template="plotly_dark",
    hovermode="x unified",
    title="Validation temporelle des capteurs",
    legend_title="PF"
)

fig.update_xaxes(title="Temps reconstruit")

st.plotly_chart(
    fig,
    use_container_width=True
)