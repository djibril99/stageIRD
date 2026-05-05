from flask import Flask, request, jsonify , Response , render_template
import psycopg2
from datetime import datetime
import io
import csv
from flask import redirect, url_for


app = Flask(__name__)

# =========================
# POSTGRES RENDER
# =========================
DATABASE_URL = "postgresql://utinamstage_user:TXWcA6nGftlMT2ALzazPYk3oSZLHAFrb@dpg-d7pk6v0g4nts73b3jnn0-a/utinamstage"
##external url 

DATABASE_URL_EXTERNAL = "postgresql://utinamstage_user:TXWcA6nGftlMT2ALzazPYk3oSZLHAFrb@dpg-d7pk6v0g4nts73b3jnn0-a.oregon-postgres.render.com/utinamstage"

DATABASE_URL = DATABASE_URL_EXTERNAL

def get_conn():
    return psycopg2.connect(DATABASE_URL, sslmode="require")

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
@app.route("/api/data", methods=["POST"]) #attention utisliser par l'esp32 pour envoyer les données
def recevoir():
    data = request.get_json()

    if not data:
        return jsonify({"error": "No JSON received"}), 400

    for capteur_id, valeurs in data.items():
        raw_val = valeurs.get("raw", 0)
        filt_val = valeurs.get("f", 0)

        insert_capteur(capteur_id, raw_val, filt_val)

    return jsonify({"status": "ok", "saved": list(data.keys())})


@app.route("/api/data2" , methods=["GET"])
def api_data():
    capteur = request.args.get("capteur", "ALL")

    conn = get_conn()
    cur = conn.cursor()

    if capteur == "ALL":
        cur.execute("""
            SELECT capteur_id, raw_value, filtered_value, created_at
            FROM mesures
            ORDER BY created_at ASC
            LIMIT 1000
        """)
    else:
        cur.execute("""
            SELECT capteur_id, raw_value, filtered_value, created_at
            FROM mesures
            WHERE capteur_id=%s
            ORDER BY created_at ASC
            LIMIT 1000
        """, (capteur,))

    rows = cur.fetchall()
    conn.close()

    data = []
    for r in rows:
        data.append({
            "capteur": r[0],
            "raw": r[1],
            "filtered": r[2],
            "time": r[3].isoformat()   # 🔥 important pour Plotly
        })

    return {"data": data}
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
@app.route("/dashboard2")
def dashboard2():
    return render_template("dashboard2.html")
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

    return redirect(url_for("dashboard2"))


# =========================
# HOME
# =========================
@app.route("/")
def home():
    #url de dashboard
    return redirect(url_for("dashboard2"))
# =========================
# START
# =========================
if __name__ == "__main__":
    init_db()
    app.run(host="0.0.0.0", port=5000 , debug=True)