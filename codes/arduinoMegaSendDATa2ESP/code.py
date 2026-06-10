import serial
import time
import json

# =====================================================
# CONFIGURATION DU PORT COM (FTDI)
# =====================================================
# Remplace 'COM4' par le vrai numéro de port de ton FTDI (ex: '/dev/ttyUSB0' sur Linux)
# La vitesse DOIT être la même que celle configurée dans RF-Setting (généralement 9600)
port_ftdi = 'COM12' 
baud_rate = 115200

try:
    ser = serial.Serial(port_ftdi, baud_rate, timeout=1)
    print(f"[-] Connecté au FTDI sur le port {port_ftdi} (9600 bauds)")
    print("[-] En attente de messages du LoRa distant...\n")
except Exception as e:
    print(f"[Erreur] Impossible d'ouvrir le port {port_ftdi} : {e}")
    exit()

try:
    while True:
        # 1. LECTURE : Si le LoRa de l'ESP32 envoie quelque chose, on l'affiche
        if ser.in_waiting > 0:
            ligne_recue = ser.readline().decode('utf-8', errors='ignore').strip()
            if ligne_recue:
                print(f"[REÇU du Drone] : {ligne_recue}")

        # 2. EXEMPLE D'ENVOI : On envoie un ordre structuré en JSON toutes les X secondes
        # (Dans ton vrai projet, ce sera déclenché par OpenCV)
        # On simule ici l'envoi d'une commande toutes les 10 secondes
        time.sleep(0.1) # Petite pause pour ne pas surcharger le processeur
        
except KeyboardInterrupt:
    print("\n[-] Arrêt du programme PC.")
    ser.close()