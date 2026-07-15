"""
a0 = 3.805738886958e-16
a1 = -2.172250524797e-10
a2 = 4.035686112355e-05
a3 = -1.458619930980e+00
"""



def calculer_pression(valeur_capteur):
    """
    Calcule la pression en kPa à partir de la valeur du capteur.
    """
    a0 = 3.805738886957e-13
    a1 = -2.172250524797e-08
    a2 = 4.035686112355e-04
    a3 = -1.458619930979e+00

    pression = a0 * (valeur_capteur ** 3) + a1 * (valeur_capteur ** 2) + a2 * valeur_capteur + a3
    return pression

resistance = 17401  # Valeur de la résistance en ohms

print("Pression calculée pour la valeur du capteur 5000:", calculer_pression(resistance), "kPa")  