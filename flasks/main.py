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