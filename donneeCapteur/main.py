import streamlit as st
import pandas as pd
import plotly.graph_objects as go

st.set_page_config(
    page_title="Dashboard Capteurs",
    layout="wide"
)
path =  r"C:\Users\djibril\Desktop\Stage_utiman\donneeCapteur\mesures13052026.csv"
df = pd.read_csv(
    path,
    names=["id", "capteur", "raw", "filtered", "date"]
)

df["date"] = pd.to_datetime(df["date"])

st.title("📊 Dashboard Capteurs")

capteurs = sorted(df["capteur"].unique())

selected_capteurs = st.sidebar.multiselect(
    "Capteurs",
    capteurs,
    default=capteurs
)

data_type = st.sidebar.radio(
    "Type de données",
    ["Raw", "Filtered", "Les deux"]
)

filtered_df = df[df["capteur"].isin(selected_capteurs)]

fig = go.Figure()

for capteur in selected_capteurs:

    cap_df = filtered_df[
        filtered_df["capteur"] == capteur
    ]

    if data_type in ["Raw", "Les deux"]:
        fig.add_trace(
            go.Scatter(
                x=cap_df["date"],
                y=cap_df["raw"],
                mode="lines",
                name=f"{capteur} Raw"
            )
        )

    if data_type in ["Filtered", "Les deux"]:
        fig.add_trace(
            go.Scatter(
                x=cap_df["date"],
                y=cap_df["filtered"],
                mode="lines",
                line=dict(dash="dash"),
                name=f"{capteur} Filtered"
            )
        )

fig.update_layout(
    height=800,
    template="plotly_dark",
    hovermode="x unified",
    legend=dict(
        orientation="h",
        y=1.05
    )
)

st.plotly_chart(
    fig,
    use_container_width=True
)