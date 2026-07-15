import numpy as np
import matplotlib.pyplot as plt

# Constantes
Rmesure = 990000.0
RcapteurMax = 101800000.0

# Valeurs de x
x = np.arange(0, RcapteurMax, 100000.0)

# Calcul de y
y = 5**2 / (Rmesure + x)

# Tracé
plt.plot(x, y)
plt.title("Puissance consommée en fonction de la résistance du capteur")
plt.xlabel("Résistance du capteur (Ω)")
plt.ylabel("Puissance consommée (W)")
plt.grid(True)

# Affichage
plt.show()