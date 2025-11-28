import numpy as np
import matplotlib.pyplot as plt

# 1. Cargar archivo .txt (formato: "tiempo_ms,valor" con encabezado)
def cargar_ecg(path):
    """
    Devuelve:
        t : vector de tiempo (ms)
        x : señal de ECG
    """
    data = np.loadtxt(path, delimiter=",", skiprows=1)
    t = data[:, 0]
    x = data[:, 1]
    return t, x

# 2. Re-muestrear ambas señales en la misma rejilla de tiempo
def alinear_y_normalizar(t1, x1, t2, x2, dt=4.0):
    """
    dt: paso de tiempo en ms para la rejilla común (4 ms ~ 250 Hz)
    """
    t_ini = max(t1[0], t2[0])
    t_fin = min(t1[-1], t2[-1])

    # Rejilla común
    t = np.arange(t_ini, t_fin, dt)

    # Interpolación lineal sobre la rejilla común
    x1i = np.interp(t, t1, x1)
    x2i = np.interp(t, t2, x2)

    # Normalización (media 0, varianza 1)
    x1n = (x1i - np.mean(x1i)) / np.std(x1i)
    x2n = (x2i - np.mean(x2i)) / np.std(x2i)

    return t, x1n, x2n

# 3. Calcular correlación y similitud
def similitud_por_correlacion(x1, x2):
    r = np.corrcoef(x1, x2)[0, 1]      # coeficiente de correlación de Pearson
    similitud = abs(r) * 100.0         # porcentaje
    return r, similitud

# 4. Función principal para comparar y (opcional) graficar
def comparar_ecg(archivo_comercial, archivo_serigrafiado, dt=4.0, graficar=True):
    t1, x1 = cargar_ecg(archivo_comercial)
    t2, x2 = cargar_ecg(archivo_serigrafiado)

    t, x1n, x2n = alinear_y_normalizar(t1, x1, t2, x2, dt)

    r, sim = similitud_por_correlacion(x1n, x2n)
    print(f"Archivo comercial:     {archivo_comercial}")
    print(f"Archivo serigrafiado:  {archivo_serigrafiado}")
    print(f"Coeficiente de correlación r = {r:.3f}")
    print(f"Similitud = {sim:.1f} %")

    if graficar:
        plt.figure(figsize=(10, 4))
        plt.plot(t, x1n, label="Comercial (normalizado)")
        plt.plot(t, x2n, label="Serigrafiado (normalizado)", alpha=0.7)
        plt.xlabel("Tiempo (ms)")
        plt.ylabel("Amplitud normalizada")
        plt.title("Comparación ECG comercial vs serigrafiado")
        plt.legend()
        plt.tight_layout()
        plt.show()

    return r, sim

# ===== Ejemplos de uso =====
# Echado
comparar_ecg("ecg_echado_ec.txt", "ecg_echado_es.txt")

# Sentado
# comparar_ecg("ecg_sentado_ec.txt", "ecg_sentado_es.txt")

# Parado
# comparar_ecg("ecg_parado_ec.txt", "ecg_parado_es.txt")

# Caminando
# comparar_ecg("ecg_caminando_ec.txt", "ecg_caminando_es.txt")

# Corriendo
# comparar_ecg("ecg_corriendo_ec.txt", "ecg_corriendo_es.txt")
