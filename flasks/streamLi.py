import streamlit as st
import pandas as pd
import plotly.graph_objects as go
import os


def convert_to_ohms(adc):
    if pd.isna(adc) or adc <= 0:
        return None

    voltage = (adc * 5.0) / 1023.0
    return 1000000.0 * ((5.0 / voltage) - 1.0)
def ____convert_to_ohms(adc):
    if pd.isna(adc):
        return None

    voltage = adc * 5.0 / 1023.0

    if voltage <= 0 or voltage >= 5.0:
        return None

    return 1000000.0 * voltage / (5.0 - voltage)

st.set_page_config(layout="wide")

# -------------------------
# DOSSIER CSV
# -------------------------
DOSSIER_DONNEES = r"C:\Users\djibril\Desktop\Stage_utiman\donneeCapteur\V5-3"

fichiers_csv = [
    f for f in os.listdir(DOSSIER_DONNEES)
    if f.endswith(".csv")
]

fichier_selectionne = st.sidebar.selectbox(
    "Fichier CSV",
    fichiers_csv
)

chemin_csv = os.path.join(DOSSIER_DONNEES, fichier_selectionne)

# -------------------------
# CHARGEMENT UNIQUE DU CSV
# -------------------------
df = pd.read_csv(chemin_csv, sep=";")

df["Date_Heure"] = pd.to_datetime(df["Date_Heure"])

capteurs = ["W1", "W2", "W3", "W4"]

df_adc = df.copy()
df_ohm = df.copy()

colonnes_mesures = [
    "W1_raw", "W1_f",
    "W2_raw", "W2_f",
    "W3_raw", "W3_f",
    "W4_raw", "W4_f"
]

for col in colonnes_mesures:
    if col in df_ohm.columns:
        df_ohm[col] = df_ohm[col].apply(convert_to_ohms)

# -------------------------
# SIDEBAR
# -------------------------
st.sidebar.header("Filtres")

unite = st.sidebar.radio(
    "Unité",
    ["ADC brut", "Résistance (Ω)"]
)

selected = st.sidebar.multiselect(
    "Capteurs",
    capteurs,
    default=capteurs
)

show_raw = st.sidebar.checkbox("Raw", value=True)
show_filtered = st.sidebar.checkbox("Filtré", value=True)

# -------------------------
# DATA CHOIX
# -------------------------
df_plot = df_adc if unite == "ADC brut" else df_ohm
y_label = "Valeur ADC" if unite == "ADC brut" else "Résistance (Ω)"

# -------------------------
# GRAPH
# -------------------------
fig = go.Figure()

for capteur in selected:

    if show_raw:
        fig.add_trace(go.Scatter(
            x=df_plot["Date_Heure"],
            y=df_plot[f"{capteur}_raw"],
            mode="lines",
            name=f"{capteur} Raw"
        ))

    if show_filtered:
        fig.add_trace(go.Scatter(
            x=df_plot["Date_Heure"],
            y=df_plot[f"{capteur}_f"],
            mode="lines",
            name=f"{capteur} Filtré"
        ))

fig.update_layout(
    title="Évolution des capteurs",
    xaxis_title="Date et heure",
    yaxis_title=y_label,
    hovermode="x unified",
    height=700
)

st.plotly_chart(fig, use_container_width=True)

with st.expander("Afficher les données"):
    st.dataframe(df_plot)