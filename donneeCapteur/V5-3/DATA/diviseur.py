input_file = r"C:\Users\djibril\Desktop\Stage_utiman\donneeCapteur\clonedu08072026\DATA.csv"
output_file = r"C:\Users\djibril\Desktop\Stage_utiman\donneeCapteur\clonedu08072026\DATAajuster.csv"

pf = 0.0
gain = 0.0  # Initialisation du gain en float

with open(input_file, "r") as fin, open(output_file, "w") as fout:
    for line in fin:
        line = line.strip()

        if not line:
            continue

        # Détection et extraction du PF (ex: pf=0.4 ou pf=1.5)
        if line.lower().startswith("pf="):
            print("Ligne PF trouvée :", line)
            pf = float(line.split("=")[1].strip())
            fout.write(line + "\n")
            continue

        cols = line.split(",")

        # Sécurité : si la ligne n'a pas assez de colonnes, on la réécrit telle quelle
        if len(cols) <= 2:
            fout.write(line + "\n")
            continue

        # On conserve intactes les deux premières colonnes (indices 0 et 1)
        nouvelle_ligne = [cols[0], cols[1]]

        # On applique le gain uniquement sur les colonnes de données à partir de l'indice 2
        for x in cols[2:]:
            valeur_float = float(x)
            if pf == 1.5:
                valeur_float += gain
            nouvelle_ligne.append(str(valeur_float))

        # Incrémentation du gain ligne par ligne si on est sous le PF 1.5
        if pf == 1.5 and gain < 2000:
            gain += 20.0

        # Réécriture au même format avec des virgules
        fout.write(",".join(nouvelle_ligne) + "\n")

print("Traitement terminé ! Fichier ajusté généré :", output_file)