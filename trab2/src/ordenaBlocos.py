import sys

class OrdenaBlocos:
  def __init__(self):
    self.blocos = []
    self.blocosOrdenados = []
  
  def preencheBlocos(self, arquivo):
    f = open(arquivo, 'r')
    self.blocos = f.read().split('\n')
    self.blocos.pop(0)

    if '' in self.blocos:
      self.blocos.remove('')

    f.close()
  
  def ordenaBlocos(self):
    for bloco in range(len(self.blocos)):
      blocoAux = self.blocos[bloco].split(' ')
      for i in range(len(blocoAux)):
        blocoAux[i] = int(blocoAux[i])
      blocoAux.sort()
      self.blocos[bloco] = blocoAux
      self.blocos[bloco]

  def preencheBlocosOrdenados(self, arquivo):
    f = open(arquivo, 'r')
    self.blocosOrdenados = f.read().split('\n')

    if '' in self.blocosOrdenados:
      self.blocosOrdenados.remove('')

    f.close()
    for bloco in range(len(self.blocosOrdenados)):
      blocoAux = self.blocosOrdenados[bloco].split(' ')

      if '' in blocoAux:
        blocoAux.remove('')

      for i in range(len(blocoAux)):
        if blocoAux[i] != '':
          blocoAux[i] = int(blocoAux[i])
      self.blocosOrdenados[bloco] = blocoAux
      self.blocosOrdenados[bloco]

  def verificaCorretude(self):
    try:
      for bloco in self.blocos:
        i = self.blocosOrdenados.index(bloco)
    except:
      print('Os blocos no arquivo de saida nao estao ordenados corretamente...', file=sys.stderr)
    else:
      print('Os blocos no arquivo de saida estao ordenados corretamente!!')

  def printBlocos(self):
    print(self.blocos)
    print(self.blocosOrdenados)