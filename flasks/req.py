import requests
url = "https://stageird.onrender.com/api/execute_sql"
##envoier de requette sql verse le server 
deletePic = "DELETE FROM mesures WHERE raw_value > 1023 OR filtered_value > 1023"
detete = "DELETE FROM mesures WHERE capteur_id not in ('a0', 'a1', 'a2', 'a3', 'a4', 'a5' , 'a6', 'a7')"

data = {"sql_query": detete}

response = requests.post(url, data=data)

print("Status Code:", response.status_code)
print("Response:", response.json())

