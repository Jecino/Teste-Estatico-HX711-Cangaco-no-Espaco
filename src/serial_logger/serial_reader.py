import serial
import time
import fnmatch
import os
import sys

LOG_DIR = "./log/"
# Informação de conexão
# Serial port pode ser visto no canto inferior esquerdo na arduino ide
SERIAL_PORT = '/dev/ttyUSB0'
BAUD_RATE = 115200


# A função verifica se o dado recebido está no formato '*,*'
def verifyData(data):
    if fnmatch.fnmatch(data, '*,*'):
        return True

    return False


def initSerial(comPort, bdrate):

    try:
        ser = serial.Serial(port=comPort, baudrate=bdrate, timeout=0.001)

        return ser

    except serial.serialutil.SerialException as e:
        print("[ERRO]: " + e.strerror)
        sys.exit(1)


def readSerial(serialConn):
    
    while True:
        data = serialConn.readline().decode().strip()
        
        # Caso não haja dados, pula o loop
        if not data:
            continue

        print(data + '\n');

        # Apenas escreve no arquivo os dados úteis
        if verifyData(data):
            file.write(data + '\n')


if __name__ == '__main__':

    # Cria a pasta de destino se não existir
    if not os.path.exists(LOG_DIR):
        os.makedirs(LOG_DIR)

    # Abre a conexão serial
    serialConn = initSerial(SERIAL_PORT, BAUD_RATE)

    # Cria o arquivo de log
    timestamp = time.strftime('%d_%m_%Y_%H:%M:%S')
    filename = LOG_DIR + "teste_estatico_" + timestamp + ".csv"

    file = open(filename, "w")
    file.write("Força,Tempo" + '\n')

    # Inicia a leitura da conexão serial
    readSerial(serialConn)