import requests
url = "https://stageird.onrender.com/api/execute_sql"
##envoier de requette sql verse le server 
deletePic = "DELETE FROM mesures WHERE raw_value >= 1020 OR filtered_value >= 1020 and capteur_id not in ('a4', 'a5' , 'a6', 'a7')"
netoiyer = "DELETE FROM mesures WHERE capteur_id not in ('a0', 'a1', 'a2', 'a3', 'a4', 'a5' , 'a6', 'a7')"
#supprimer les donner fltreé supierieur a 600 SAUF POUR LE CAPEUR a6
req = "DELETE FROM mesures WHERE filtered_value > 600 AND capteur_id != 'a6'"
#supprimer les donner brute supierieur a 390 SAUF POUR LE CAPEUR a6
req2 = "DELETE FROM mesures WHERE raw_value > 390 AND capteur_id != 'a6'"
#recalculer des donner filter : si |filtered_value-filtered_valuePrecedent| > 100 et que filterPrecedent > 0 alors filter = raw * 0.8 + filterPrecedent * 0.2
req3 = """
WITH ordered_mesures AS (
    SELECT
        id,
        capteur_id,
        raw_value,
        filtered_value,
        created_at,
        LAG(filtered_value) OVER (
            PARTITION BY capteur_id
            ORDER BY created_at
        ) AS prev_filtered
    FROM mesures
)

UPDATE mesures m
SET filtered_value =
    om.raw_value * 0.8 +
    om.prev_filtered * 0.2

FROM ordered_mesures om

WHERE m.id = om.id
AND ABS(om.filtered_value - om.prev_filtered) > 50
AND om.prev_filtered > 0;
"""


data = {"sql_query": req3}

response = requests.post(url, data=data)

print("Status Code:", response.status_code)
print("Response:", response.json())