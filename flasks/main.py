from flask import Flask, request, jsonify
import sqlite3
import json
from datetime import datetime

app = Flask(__name__)

DB_NAME = "capteurs.db"


# =====================================
# INIT BASE SQLITE
# =====================================
def init_db():
    conn = sqlite3.connect(DB_NAME)
    cur = conn.cursor()

    cur.execute("""
    CREATE TABLE IF NOT EXISTS mesures (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        capteur_id TEXT,
        raw_value REAL,
        filtered_value REAL,
        created_at TEXT
    )
    """)

    conn.commit()
    conn.close()


# =====================================
# INSERT CAPTEUR
# =====================================
def insert_capteur(capteur_id, raw_val, filt_val):
    conn = sqlite3.connect(DB_NAME)
    cur = conn.cursor()

    cur.execute("""
    INSERT INTO mesures (capteur_id, raw_value, filtered_value, created_at)
    VALUES (?, ?, ?, ?)
    """, (
        capteur_id,
        raw_val,
        filt_val,
        datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    ))

    conn.commit()
    conn.close()


# =====================================
# RECEVOIR JSON ESP32 / ARDUINO
# =====================================
@app.route("/api/data", methods=["POST", "GET"])
def recevoir():

    # POST JSON
    if request.method == "POST":
        data = request.get_json(force=True)

    # GET ?json=
    else:
        raw_json = request.args.get("json")
        data = json.loads(raw_json)

    # Exemple reçu :
    # {
    #   "a6":{"raw":123,"f":120},
    #   "a0":{"raw":456,"f":450}
    # }

    for capteur_id, valeurs in data.items():

        raw_val = valeurs.get("raw", 0)
        filt_val = valeurs.get("f", 0)

        insert_capteur(capteur_id, raw_val, filt_val)

    return jsonify({
        "status": "ok",
        "saved": list(data.keys())
    })


# =====================================
# TOUTES LES DONNÉES
# =====================================
@app.route("/api/all")
def all_data():

    conn = sqlite3.connect(DB_NAME)
    cur = conn.cursor()

    cur.execute("""
    SELECT id, capteur_id, raw_value, filtered_value, created_at
    FROM mesures
    ORDER BY id DESC
    """)

    rows = cur.fetchall()
    conn.close()

    result = []

    for row in rows:
        result.append({
            "id": row[0],
            "capteur": row[1],
            "raw": row[2],
            "filtered": row[3],
            "date": row[4]
        })

    return jsonify(result)


# =====================================
# DONNÉES D'UN CAPTEUR
# =====================================
@app.route("/api/capteur/<capteur_id>")
def one_capteur(capteur_id):

    conn = sqlite3.connect(DB_NAME)
    cur = conn.cursor()

    cur.execute("""
    SELECT id, raw_value, filtered_value, created_at
    FROM mesures
    WHERE capteur_id=?
    ORDER BY id DESC
    """, (capteur_id,))

    rows = cur.fetchall()
    conn.close()

    result = []

    for row in rows:
        result.append({
            "id": row[0],
            "raw": row[1],
            "filtered": row[2],
            "date": row[3]
        })

    return jsonify(result)


@app.route("/dashboard")
def dashboard():

    capteur = request.args.get("capteur", "a6")
    type_data = request.args.get("type", "raw")

    conn = sqlite3.connect(DB_NAME)
    cur = conn.cursor()

    if type_data == "filtered":
        cur.execute("""
        SELECT filtered_value
        FROM mesures
        WHERE capteur_id=?
        ORDER BY id ASC
        """, (capteur,))
    else:
        cur.execute("""
        SELECT raw_value
        FROM mesures
        WHERE capteur_id=?
        ORDER BY id ASC
        """, (capteur,))

    rows = cur.fetchall()
    conn.close()

    values = [r[0] for r in rows]

    import matplotlib.pyplot as plt
    import io, base64

    plt.figure()
    plt.plot(values)
    plt.title(f"Capteur {capteur} - {type_data}")
    plt.xlabel("temps")
    plt.ylabel("valeur")

    img = io.BytesIO()
    plt.savefig(img, format="png")
    plt.close()
    img.seek(0)

    graph = base64.b64encode(img.getvalue()).decode()

    return f"""
<!DOCTYPE html>
<html>
<head>
    <title>Dashboard Capteurs</title>

    <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/css/bootstrap.min.css" rel="stylesheet">

    <meta http-equiv="refresh" content="2">
</head>

<body class="bg-dark text-light">

<div class="container mt-4">

    <h1 class="text-center mb-4">📊 Dashboard Capteurs</h1>

    <div class="card bg-secondary text-white p-3 mb-3">

        <form method="get" class="row g-3">

            <div class="col-md-6">
                <label class="form-label">Capteur</label>
                <input class="form-control" name="capteur" value="{capteur}">
            </div>

            <div class="col-md-6">
                <label class="form-label">Type</label>
                <select class="form-control" name="type">
                    <option value="raw" {"selected" if type_data=="raw" else ""}>Raw</option>
                    <option value="filtered" {"selected" if type_data=="filtered" else ""}>Filtered</option>
                </select>
            </div>

            <div class="col-12">
                <button class="btn btn-primary w-100">Afficher</button>
            </div>

        </form>
    </div>

    <div class="card bg-dark border-light p-3 text-center">

        <img src="data:image/png;base64,{graph}" class="img-fluid">

    </div>

</div>

</body>
</html>
"""



# =====================================
# PAGE TEST
# =====================================
@app.route("/")
def home():
    return "Serveur capteurs actif"


# =====================================
# MAIN
# =====================================
if __name__ == "__main__":
    init_db()
    app.run(host="0.0.0.0", port=5000)