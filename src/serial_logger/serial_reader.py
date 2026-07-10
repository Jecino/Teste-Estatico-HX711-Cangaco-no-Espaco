import os
import sys
import time
import queue
import serial
import fnmatch
import threading
import pyqtgraph as pg
from collections import deque
from pyqtgraph.Qt import QtCore, QtWidgets

# Destino do log
LOG_DIR = "./log/"


# Informação de conexão
# Serial port pode ser visto no canto inferior esquerdo na arduino ide
#
SERIAL_PORT = '/dev/ttyUSB0'
BAUD_RATE = 115200


#
#   Variáveis gráfico
#
MAX_PONTOS = 1000
max_forca = 0
duracao = 0
fila_dados = queue.Queue()
x_data = deque(maxlen=MAX_PONTOS)
y_data = deque(maxlen=MAX_PONTOS)


#
# A função verifica se o dado recebido está no formato '*,*'
#
def verifyData(data):
    if fnmatch.fnmatch(data, '*,*'):
        return True

    return False


#
# Inicia a conexão serial
#
def initSerial(comPort, bdrate):

    try:
        ser = serial.Serial(port=comPort, baudrate=bdrate, timeout=1)

        return ser

    except serial.serialutil.SerialException as e:
        print("[ERRO]: " + e.strerror)
        sys.exit(1)


#
# Função de atualização do gráfico
#
def atualizar():
    global max_forca

    while not fila_dados.empty():
        string = str(fila_dados.get())

        try:
            forca, tempo = map(float, string.split(","))
            x_data.append(tempo)
            y_data.append(forca)

            if forca > max_forca:
                max_forca = forca
                label_pico.setText(f"Força Máx: {max_forca:.2f} N")

            duracao = tempo / 1000
            label_duracao.setText(f"Duração: {duracao} s")

        except Exception as e:
            print(f"[ERRO]: {e}")
            continue

    if len(x_data) > 0:

        curva.setData(x_data, y_data)



#
# Leitura e tratamento dos dados do serial
#
def readSerial(serialConn):
    contador = 0

    while True:
        try:
            data = serialConn.readline().decode().strip()

        except Exception as e:
            print(f"[ERRO]: {e}")
            continue

        # Caso não haja dados, pula iteração
        if not data:
            continue

        # Apenas escreve no arquivo os dados úteis
        if verifyData(data):
            contador += 1

            # Escreve os dados no arquivo de saída
            file.write(data + '\n')

            # Limite para não fazer flush a todo momento
            if contador >= 80:
                file.flush()
                contador = 0

            # Armazena os dados para plot
            fila_dados.put(data)
        

#
# Função de início da janela
#
def windowInit():

    app = QtWidgets.QApplication(sys.argv)
    
    # Define fundo branco e letras pretas (padrão de artigos/relatórios)
    pg.setConfigOption('background', 'w')
    pg.setConfigOption('foreground', 'k')

    janela = pg.GraphicsLayoutWidget(show=True, title="Banco Estatico")
    janela.resize(800, 600)

    return app,janela


#
# Função de início do gráfico
#
def graphInit(app, janela):

    # Adiciona o plot na janela e configura rótulos
    grafico = janela.addPlot(row=1, col=0, colspan=2, title="Força vs Tempo")
    grafico.setLabel('bottom', "Tempo", units="ms")
    grafico.setLabel('left', "Força", units="N")

    # Remove prefixo dos eixos (m, k, M, etc)
    grafico.getAxis('bottom').enableAutoSIPrefix(False)
    grafico.getAxis('left').enableAutoSIPrefix(False)

    grafico.showGrid(x=True, y=True)

    return grafico



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

    # Inicia a thread de leitura do arduino
    thread_leitura = threading.Thread(target=readSerial, args=(serialConn,), daemon=True)
    thread_leitura.start()

    app,janela = windowInit()

    grafico = graphInit(app, janela)

    # Cria a curva (linha azul, com espessura 2)
    caneta = pg.mkPen(color='b', width=2)
    curva = grafico.plot(pen=caneta)

    label_pico = janela.addLabel(text="Força Máx: 0.00 N", row=0, col=0, size='20pt', color='k', bold=True)
    label_duracao = janela.addLabel(text="Duração: 0.00 s", row=0, col=1, size='20pt', color='k', bold=True)

    # Configuração do Timer para atualizar a tela
    timer = QtCore.QTimer()
    timer.timeout.connect(atualizar)

    # taxa de atualização (ms)
    timer.start(33)

    sys.exit(app.exec())
