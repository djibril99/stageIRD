import requests
url = "https://stageird.onrender.com/api/execute_sql"
##envoier de requette sql verse le server 
requete = "DELETE FROM mesures WHERE raw_value > 1000 OR filtered_value > 1000"
data = {"sql_query": requete}
headers = {
   "Content-Type": "application/json",
   "Authorization": "Bearer YOUR_TOKEN"
}
response = requests.post(url, json=data, headers=headers)
print("Status Code:", response.status_code)
print("Response:", response.json())


