import streamlit as st
import pandas as pd
import plotly.graph_objects as go

st.set_page_config(
    page_title="Visualisation des capteurs",
    layout="wide"
)

st.title("Visualisation des mesures de séchage")

uploaded_file = st.file_uploader(
    "Choisir un fichier CSV",
    type="csv"
)

if uploaded_file is not None:

    # ==========================
    # Lecture de la première ligne
    # ==========================
    date_debut = uploaded_file.readline().decode("utf-8").strip()

    df = pd.read_csv(uploaded_file)

    # Temps en heures
    df["Temps_h"] = df["temps_ms"] / 1000 / 3600

    st.info(date_debut)

    st.sidebar.header("Options")

    mode = st.sidebar.radio(
        "Données à afficher",
        ["Brutes", "Filtrées", "Les deux"]
    )

    courbes = st.sidebar.multiselect(
        "Capteurs",
        [
            "Capteur 1",
            "Capteur 2",
            "Référence"
        ],
        default=[
            "Capteur 1",
            "Capteur 2",
            "Référence"
        ]
    )

    fig = go.Figure()

    # =============================
    # CAPTEUR 1
    # =============================
    if "Capteur 1" in courbes:

        if mode in ["Brutes", "Les deux"]:
            fig.add_trace(
                go.Scatter(
                    x=df["Temps_h"],
                    y=df["CapteurBrut1"],
                    mode="lines",
                    name="Capteur 1 brut",
                    line=dict(dash="dot")
                )
            )

        if mode in ["Filtrées", "Les deux"]:
            fig.add_trace(
                go.Scatter(
                    x=df["Temps_h"],
                    y=df["CapteurFiltre1"],
                    mode="lines",
                    name="Capteur 1 filtré"
                )
            )

    # =============================
    # CAPTEUR 2
    # =============================
    if "Capteur 2" in courbes:

        if mode in ["Brutes", "Les deux"]:
            fig.add_trace(
                go.Scatter(
                    x=df["Temps_h"],
                    y=df["CapteurBrut2"],
                    mode="lines",
                    name="Capteur 2 brut",
                    line=dict(dash="dot")
                )
            )

        if mode in ["Filtrées", "Les deux"]:
            fig.add_trace(
                go.Scatter(
                    x=df["Temps_h"],
                    y=df["CapteurFiltre2"],
                    mode="lines",
                    name="Capteur 2 filtré"
                )
            )

    # =============================
    # REFERENCE
    # =============================
    if "Référence" in courbes:

        if mode in ["Brutes", "Les deux"]:
            fig.add_trace(
                go.Scatter(
                    x=df["Temps_h"],
                    y=df["Reference45K"],
                    mode="lines",
                    name="Référence brute",
                    line=dict(dash="dot")
                )
            )

        if mode in ["Filtrées", "Les deux"]:
            fig.add_trace(
                go.Scatter(
                    x=df["Temps_h"],
                    y=df["Reference45KFiltre"],
                    mode="lines",
                    name="Référence filtrée"
                )
            )

    fig.update_layout(
        title=date_debut,
        xaxis_title="Temps (heures)",
        yaxis_title="Résistance (Ω)",
        hovermode="x unified",
        height=700,
        template="plotly_white"
    )

    st.plotly_chart(
        fig,
        use_container_width=True
    )

    st.subheader("Données")

    st.dataframe(df)