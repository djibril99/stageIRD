
import streamlit as st
import pandas as pd
import plotly.graph_objects as go
import os

# --------------------------------------------------
# Conversion ADC -> Ohms
# --------------------------------------------------

def convert_to_ohms(adc):
    if pd.isna(adc) or adc <= 0:
        return None

    voltage = (adc * 5.0) / 1023.0

    if voltage <= 0 or voltage >= 5.0:
        return None

    return 1000000.0 * ((5.0 / voltage) - 1.0)


# --------------------------------------------------
# Configuration Streamlit
# --------------------------------------------------

st.set_page_config(layout="wide")

st.title("Visualisation des capteurs")

# --------------------------------------------------
# Dossier des données
# --------------------------------------------------

DOSSIER_DONNEES = r"C:\Users\djibril\Desktop\Stage_utiman\donneeCapteur\V5-3"

# --------------------------------------------------
# Recherche des fichiers CSV
# --------------------------------------------------

fichiers_csv = [
    f for f in os.listdir(DOSSIER_DONNEES)
    if f.endswith(".csv") and f.upper() != "DATA.CSV"
]

if len(fichiers_csv) == 0:
    st.error("Aucun fichier CSV trouvé")
    st.stop()

fichier_selectionne = st.sidebar.selectbox(
    "Fichier CSV principal",
    fichiers_csv
)

chemin_csv = os.path.join(DOSSIER_DONNEES, fichier_selectionne)

# --------------------------------------------------
# Chargement du CSV principal
# --------------------------------------------------

df = pd.read_csv(chemin_csv, sep=";")

if "Date_Heure" in df.columns:
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

# --------------------------------------------------
# Sidebar
# --------------------------------------------------

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

show_raw = st.sidebar.checkbox(
    "Raw",
    value=True
)

show_filtered = st.sidebar.checkbox(
    "Filtré",
    value=True
)

# --------------------------------------------------
# Choix des données
# --------------------------------------------------

df_plot = df_adc if unite == "ADC brut" else df_ohm

y_label = (
    "Valeur ADC"
    if unite == "ADC brut"
    else "Résistance (Ω)"
)

# --------------------------------------------------
# Graphique principal
# --------------------------------------------------

st.header("Fichier principal")

fig = go.Figure()

for capteur in selected:

    if show_raw:
        col = f"{capteur}_raw"

        if col in df_plot.columns:
            fig.add_trace(
                go.Scatter(
                    x=df_plot["Date_Heure"],
                    y=df_plot[col],
                    mode="lines",
                    name=f"{capteur} Raw"
                )
            )

    if show_filtered:
        col = f"{capteur}_f"

        if col in df_plot.columns:
            fig.add_trace(
                go.Scatter(
                    x=df_plot["Date_Heure"],
                    y=df_plot[col],
                    mode="lines",
                    name=f"{capteur} Filtré"
                )
            )

fig.update_layout(
    title="Évolution des capteurs",
    xaxis_title="Date et heure",
    yaxis_title=y_label,
    hovermode="x unified",
    height=700
)

st.plotly_chart(
    fig,
    use_container_width=True
)

with st.expander("Afficher les données du fichier principal"):
    st.dataframe(df_plot)

# --------------------------------------------------
# Chargement DATA.csv
# --------------------------------------------------

st.header("DATA.csv")

chemin_data = os.path.join(
    DOSSIER_DONNEES,
    "DATA.csv"
)

if os.path.exists(chemin_data):

    df_data = pd.read_csv(
        chemin_data,
        header=None,
        names=[
            "Temps",
            "C1",
            "C2",
            "C3",
            "C4",
            "C5",
            "C6",
            "C7",
            "C8"
        ]
    )

    st.subheader("Tableau DATA.csv")

    st.dataframe(
        df_data,
        use_container_width=True
    )

    fig_data = go.Figure()

    for col in [
        "C1",
        "C2",
        "C3",
        "C4",
        "C5",
        "C6",
        "C7",
        "C8"
    ]:

        fig_data.add_trace(
            go.Scatter(
                x=df_data["Temps"],
                y=df_data[col],
                mode="lines",
                name=col
            )
        )

    fig_data.update_layout(
        title="Capteurs C1 à C8",
        xaxis_title="Temps",
        yaxis_title="Résistance (Ω)",
        hovermode="x unified",
        height=700
    )

    st.plotly_chart(
        fig_data,
        use_container_width=True
    )

else:
    st.warning(
        "Le fichier DATA.csv n'existe pas dans le dossier."
    )