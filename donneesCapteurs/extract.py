import pandas as pd
import matplotlib.pyplot as plt
from io import StringIO

# ===== Chargement fichier =====
file = "C:\\Users\\djibril\\Desktop\\Stage_utiman\\donneesCapteurs\\W3_WM_W1_WM___WM_2026-04-21.csv"

with open(file, "r", encoding="utf-8") as f:
    data = f.read()

df = pd.read_csv(StringIO(data), low_memory=False)

# ===== Conversion date =====
df["date_mes"] = pd.to_datetime(df["date_mes"], errors="coerce")
df = df.dropna(subset=["date_mes"])
df = df.sort_values("date_mes")

# ===== Colonnes numériques =====
numeric_cols = ["pp_mmjour", "tair_max"] + \
               [col for col in df.columns if col == "S_t" or col.startswith("S_")]

for col in numeric_cols:
    if col in df.columns:
        df[col] = pd.to_numeric(df[col], errors="coerce")

# ===== Affichage capteurs =====
st_cols = [col for col in df.columns if col == "S_t" or col.startswith("S_")]

print("Colonnes capteurs :", st_cols)

if st_cols:
    print(df[["date_mes"] + st_cols].head(20).to_string(index=False))
else:
    print("Aucune colonne capteur trouvée")

# ===== Plot météo =====
plot_df = df.dropna(subset=["date_mes", "pp_mmjour", "tair_max"])

plt.figure(figsize=(12, 6))
plt.plot(plot_df["date_mes"], plot_df["pp_mmjour"], label="Pluie (mm/jour)")
plt.plot(plot_df["date_mes"], plot_df["tair_max"], label="Température max")

# ===== Plot capteurs =====
for col in st_cols:
    if col in df.columns:
        plt.plot(df["date_mes"], df[col], label=col)

plt.xlabel("Date")
plt.ylabel("Valeurs")
plt.title("Évolution pluie, température et capteurs")
plt.legend()
plt.xticks(rotation=45)
plt.tight_layout()

plt.savefig("graphique.png", dpi=100)
plt.show()