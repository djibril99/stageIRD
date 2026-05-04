from flask import Flask, request, jsonify , Response , render_template
import psycopg2
import os
import json
from datetime import datetime
import matplotlib.pyplot as plt
import io, base64
import csv


app = Flask(__name__)

# =========================
# POSTGRES RENDER
# =========================
DATABASE_URL = "postgresql://utinamstage_user:TXWcA6nGftlMT2ALzazPYk3oSZLHAFrb@dpg-d7pk6v0g4nts73b3jnn0-a/utinamstage"
def get_conn():
    return psycopg2.connect(DATABASE_URL)

# =========================
# INIT DB
# =========================
def init_db():
    conn = get_conn()
    cur = conn.cursor()

    cur.execute("""
    CREATE TABLE IF NOT EXISTS mesures (
        id SERIAL PRIMARY KEY,
        capteur_id TEXT,
        raw_value DOUBLE PRECISION,
        filtered_value DOUBLE PRECISION,
        created_at TIMESTAMP
    )
    """)

    conn.commit()
    conn.close()

# =========================
# INSERT DATA
# =========================
def insert_capteur(capteur_id, raw_val, filt_val):
    conn = get_conn()
    cur = conn.cursor()

    cur.execute("""
    INSERT INTO mesures (capteur_id, raw_value, filtered_value, created_at)
    VALUES (%s, %s, %s, %s)
    """, (
        capteur_id,
        raw_val,
        filt_val,
        datetime.now()
    ))

    conn.commit()
    conn.close()

# =========================
# RECEIVE DATA ESP32
# =========================
@app.route("/api/data", methods=["POST", "GET"])
def recevoir():

    if request.method == "POST":
        data = request.get_json(force=True)
    else:
        raw_json = request.args.get("json")
        data = json.loads(raw_json)

    for capteur_id, valeurs in data.items():

        raw_val = valeurs.get("raw", 0)
        filt_val = valeurs.get("f", 0)

        insert_capteur(capteur_id, raw_val, filt_val)

    return jsonify({"status": "ok", "saved": list(data.keys())})

# =========================
# ALL DATA API
# =========================
@app.route("/api/all")
def all_data():

    conn = get_conn()
    cur = conn.cursor()

    cur.execute("""
    SELECT id, capteur_id, raw_value, filtered_value, created_at
    FROM mesures
    ORDER BY id DESC
    """)

    rows = cur.fetchall()
    conn.close()

    return jsonify([
        {
            "id": r[0],
            "capteur": r[1],
            "raw": r[2],
            "filtered": r[3],
            "date": r[4]
        }
        for r in rows
    ])

# =========================
# ONE SENSOR
# =========================
@app.route("/api/capteur/<capteur_id>")
def one_capteur(capteur_id):

    conn = get_conn()
    cur = conn.cursor()

    cur.execute("""
    SELECT id, raw_value, filtered_value, created_at
    FROM mesures
    WHERE capteur_id=%s
    ORDER BY id DESC
    """, (capteur_id,))

    rows = cur.fetchall()
    conn.close()

    return jsonify([
        {
            "id": r[0],
            "raw": r[1],
            "filtered": r[2],
            "date": r[3]
        }
        for r in rows
    ])

# =========================
# DASHBOARD
# =========================
@app.route("/dashboard")
def dashboard():


    # ... reste de ton code pour générer le graphique ...


    capteur = request.args.get("capteur", "ALL")
    type_data = request.args.get("type", "raw")

    conn = get_conn()
    cur = conn.cursor()

    cur.execute("SELECT DISTINCT capteur_id FROM mesures ORDER BY capteur_id")
    capteurs_db = [c[0] for c in cur.fetchall()]

    plt.figure()

    if capteur == "ALL":

        for c in capteurs_db:

            cur.execute("""
                SELECT raw_value, filtered_value
                FROM mesures
                WHERE capteur_id=%s
                ORDER BY id ASC
            """, (c,))

            rows = cur.fetchall()

            if not rows:
                continue

            raw = [r[0] for r in rows]
            filt = [r[1] for r in rows]

            if type_data == "ALL":
                plt.plot(raw, label=f"{c} raw")
                plt.plot(filt, label=f"{c} filtered")
            elif type_data == "filtered":
                plt.plot(filt, label=f"{c} filtered")
            else:
                plt.plot(raw, label=f"{c} raw")

        plt.legend()

    else:

        cur.execute("""
            SELECT raw_value, filtered_value
            FROM mesures
            WHERE capteur_id=%s
            ORDER BY id ASC
        """, (capteur,))

        rows = cur.fetchall()

        raw = [r[0] for r in rows]
        filt = [r[1] for r in rows]

        if type_data == "ALL":
            plt.plot(raw, label="raw")
            plt.plot(filt, label="filtered")
            plt.legend()
        elif type_data == "filtered":
            plt.plot(filt)
        else:
            plt.plot(raw)

    conn.close()

    plt.title(f"Dashboard {capteur}")
    plt.xlabel("time")
    plt.ylabel("value")

    img = io.BytesIO()
    plt.savefig(img, format="png")
    plt.close()
    img.seek(0)

    graph = base64.b64encode(img.getvalue()).decode()

    return render_template(
            "dashboard.html",
            graph=graph,
            capteur=capteur,
            type_data=type_data,
            capteurs_db=capteurs_db
        )


# =========================
# EXPORT CSV
# =========================
@app.route("/api/export")
def export_csv():

    capteur = request.args.get("capteur", "ALL")

    conn = get_conn()
    cur = conn.cursor()

    if capteur == "ALL":
        cur.execute("""
            SELECT id, capteur_id, raw_value, filtered_value, created_at
            FROM mesures
            ORDER BY id ASC
        """)
    else:
        cur.execute("""
            SELECT id, capteur_id, raw_value, filtered_value, created_at
            FROM mesures
            WHERE capteur_id=%s
            ORDER BY id ASC
        """, (capteur,))

    rows = cur.fetchall()
    conn.close()

    # Création CSV en mémoire
    output = io.StringIO()
    writer = csv.writer(output)

    # Header
    writer.writerow(["id", "capteur", "raw", "filtered", "date"])

    # Données
    for r in rows:
        writer.writerow(r)

    csv_data = output.getvalue()
    output.close()

    return Response(
        csv_data,
        mimetype="text/csv",
        headers={
            "Content-Disposition": "attachment; filename=mesures.csv"
        }
    )
from flask import redirect, url_for

@app.route("/clear", methods=["POST"])
def clear():
    conn = get_conn()
    cur = conn.cursor()

    try:
        cur.execute("DELETE FROM mesures")
        cur.execute("ALTER SEQUENCE mesures_id_seq RESTART WITH 1")
        conn.commit()
    except Exception as e:
        conn.rollback()
        print(e)
    finally:
        conn.close()

    return redirect(url_for("dashboard"))


# =========================
# HOME
# =========================
@app.route("/")
def home():
    return "Flask + PostgreSQL Render OK"

# =========================
# START
# =========================
if __name__ == "__main__":
    init_db()
    app.run(host="0.0.0.0", port=5000)