import serial
import csv
import matplotlib.pyplot as plt

porta = "COM3"
baud = 115200
arquivo = "logger.csv"

ser = serial.Serial(porta,baud)
ser.flush()
print("Abrindo Porta Serial")

amostra = 100
linha = 0
sinal_coletado = []

file = open(arquivo, mode='w')


while linha <= amostra:

    data = ser.readline().decode('utf-8', errors='ignore')
    sinal_coletado.append(float(data))

    if linha<5:
        print(data[:-2])
    
    with open(arquivo, 'a', newline='') as f:
        writer = csv.writer(f)
        writer.writerows(data)
    
    linha = linha+1


file.close()
ser.close()
print("\nFinalizado\n")


# Verifica se dados foram coletados
if sinal_coletado:
    # Plotando o sinal coletado
    plt.figure(figsize=(6, 4)) 
    plt.plot(sinal_coletado, label="Sinal", marker='o')
    plt.xlabel('Índice da Amostra')
    plt.ylabel('Valor do Sinal')
    plt.title('Gráfico do Sinal Coletado')
    plt.legend()
    plt.grid(True)
    plt.show()