import requests
url = "https://stageird.onrender.com/api/execute_sql"
##envoier de requette sql verse le server 
deletePic = "DELETE FROM mesures WHERE raw_value > 1023 OR filtered_value > 1023"

data = {"sql_query": deletePic}

response = requests.post(url, data=data)

print("Status Code:", response.status_code)
print("Response:", response.json())


