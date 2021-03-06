/* Disciplina: Computacao Concorrente */
/* Prof.: Silvana Rossetto */
/* */
/* Codigo: Segundo Trabalho de Implementação */
/* Aluno: Gabriel Magalhaes */ 
/* DRE: 118088665 */ 
/* Aluno: Raphael Mesquita */ 
/* DRE: 118020104 */ 

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;
import java.util.Scanner;

// Classe que realiza a solucao sequencial do problema e
// compara com a solucao concorrente
class VerificaCorretude {
  private int[][] blocos;
  private int N, elementos;
  Scanner entradaScanner;

  // Construtor
  VerificaCorretude(int N, String entrada, String saida) {
    this.N = N;
    try {
      this.entradaScanner = new Scanner(new FileInputStream(entrada)); // Guarda os dados do arquivo para ler posteriormente
      this.elementos = this.entradaScanner.nextInt(); // Recebe o valor que representa o numero elementos no arquivo de entrada
      this.blocos = new int[this.elementos/N][N]; // Inicializa o buffer MxN

      // Preenche os blocos
      for(int i = 0; i < blocos.length && this.entradaScanner.hasNextInt(); i++) {
        for(int j = 0; j < blocos[i].length && this.entradaScanner.hasNextInt(); j++) {
          blocos[i][j] = this.entradaScanner.nextInt();
        }
      }

      // Ordena os blocos
      for(int i = 0; i < blocos.length; i++) {
        int[] bloco = new int[blocos[i].length];
        bloco = ordenaBloco(blocos[i].length, blocos[i]);
        blocos[i] = bloco.clone();
      }
    } catch (FileNotFoundException e) {
      e.printStackTrace();
    }
  }

  // Verifica se os blocos de blocosConc foram ordenados corretamente
  public boolean verifica(int[][] blocosConc) {
    // Verifica se ha um bloco da solucao sequencial igual bloco da solucao concorrente
    for(int i = 0; i < blocosConc.length; i++)  {
      boolean blocoCerto = false;
      for(int j = 0; j < this.blocos.length; j++) {
        boolean certo = true;
        for(int k = 0; k < this.N; k++) {
          if(blocosConc[i][k] != this.blocos[j][k]) { certo = false; break; }
        }
        if(certo) {
          blocoCerto = true;
          break;
        }
      }
      if(!blocoCerto) return false;
    }
    return true;
  }

  // Imprime os blocos
  public void printBlocos() {
    for(int i = 0; i < blocos.length; i++) {
      for(int j = 0; j < blocos[i].length; j++) {
        System.out.print(blocos[i][j] + " ");
      }
      System.out.println();
    }
  }

  // Recebe um bloco e o retorna ordenado crescentemente
  private int[] ordenaBloco(int tamBloco, int[] bloco) {
    int[] blocoOrdenado = bloco;
    int k, j, aux;

    for (k = 1; k < tamBloco; k++) {
      for (j = 0; j < tamBloco - k; j++) {
        if (bloco[j] > bloco[j + 1]) {
          aux = bloco[j];
          bloco[j] = bloco[j + 1];
          bloco[j + 1] = aux;
        }
      }
    }
    return blocoOrdenado;
  }
}

// Classe que representa o buffer e os metodos a que ele eh relevante
class BufferLimitado {
  private int[][] blocosConc;
  private int[] buffer;
  private int N, elementos, iBloco = 0, escritos = 0, count = 0, in = 0, out = 0, blocosBuffer = 10;
  Scanner entradaScanner;

  // Construtor
  BufferLimitado(int N, String entrada, String saida) {
    this.N = N;
    try {
      this.entradaScanner = new Scanner(new FileInputStream(entrada)); // Guarda os dados do arquivo para ler posteriormente
      this.elementos = this.entradaScanner.nextInt(); // Recebe o valor que representa o numero elementos no arquivo de entrada
      this.buffer = new int[this.blocosBuffer*N]; // Inicializa o buffer 10xN
      this.blocosConc = new int[this.elementos/N][N];
    } catch (FileNotFoundException e) {
      e.printStackTrace();
    }
  }

  // Imprime os blocos
  public void printBlocosConc() {
    for(int i = 0; i < blocosConc.length; i++) {
      for(int j = 0; j < blocosConc[i].length; j++) {
        System.out.print(blocosConc[i][j] + " ");
      }
      System.out.println();
    }
  }

  // Retorna os blocos
  public int[][] getBlocosConc() {
    return this.blocosConc.clone();
  }

  // Insere um bloco na solucao concorrente
  public synchronized void insereBlocoConc(int[] bloco) {
    blocosConc[iBloco++] = bloco.clone();
  }

  // Retorna o tamanho dos blocos
  public int getN() { return this.N; }

  // Retorna se o numero de elementos escritos eh menor que o numero de elementos, ou seja,
  // se ainda ha blocos para escrever. Caso nao haja, libera todas as threads.
  public synchronized boolean hasNextBloco() {
    boolean hasNext = this.escritos < this.elementos;
    if(!hasNext) this.notifyAll();
    return hasNext;
  }

  // Retorna se ainda ha elementos(numeros) a serem lidos no arquivo
  // Verifica com exclusao mutua, pois a leitura do arquivo pode ser feita com multiplas threads
  public synchronized boolean hasNextInt() {
    return this.entradaScanner.hasNextInt();
  }

  // Insere o proximo elemento do arquivo no buffer
  public synchronized void Insere () {
    try {
      while (count == buffer.length) { wait(); } // Bloqueio condicao logica
      int elemento = this.entradaScanner.nextInt();
      buffer[in] = elemento;
      in = (in + 1) % buffer.length; count++;
      // System.out.println("Inserindo elemento: " + elemento);
      notifyAll();
    } catch (InterruptedException e) { }
  }

  // Remove e retorna um bloco de elementos do buffer.
  // Primeiro aguarda preencher um bloco e depois o remove completamente
  public synchronized int[] Remove () {
    int[] elementos = new int[this.N];
    try {
      // Bloqueio condicao logica. Aguarda pelo menos um bloco preencher, caso haja elementos para escrever
      while ((count == 0 || (count % this.N) != 0) && escritos < this.elementos) { wait(); }

      if(escritos == this.elementos) return null;
      
      // Retira exatamente um bloco
      int aux = count - this.N;
      while(count > aux && count > 0) {
        elementos[out % this.N] = buffer[out % (buffer.length)];
        out = (out + 1) % (buffer.length); count--; escritos++;
      }

      // System.out.println("escritos = " + this.escritos);
      notifyAll();
      return elementos;
    } catch(InterruptedException e) { return new int[1]; }
  }
}

// Classe que representa um produtor e seus metodos
class Produtor extends Thread {
  int id;
  BufferLimitado buffer;

  // Construtor
  Produtor(int id, BufferLimitado buffer) {
    this.id = id;
    this.buffer = buffer;
  }

  // Execucao da thread
  public void run () {
    for (;this.buffer.hasNextInt();) {
      this.buffer.Insere();
    }
    System.out.println("Produtor terminou de ler o arquivo.");
  }
}

// Classe que representa um consumidor e seus metodos
class Consumidor extends Thread {
  int id, delay = 0;
  int[] bloco;
  BufferLimitado buffer;
  Monitor monitor;
  String saida;

  // Construtor
  Consumidor(int id, BufferLimitado buffer, Monitor monitor, String saida) {
    this.id = id;
    this.buffer = buffer;
    this.bloco = new int[buffer.getN()];
    this.monitor = monitor;
    this.saida = saida;

    this.criaArquivoVazio();
  }

  // Verifica se o arquivo de saida passado como argumento do programa ja existe.
  // Se existe, limpa o arquivo para poder escrever os blocos ordenados.
  // Caso contrario, cria o arquivo com mesmo nome.
  private void criaArquivoVazio() {
    File arquivoSaida = new File(saida);

    if(!arquivoSaida.exists()) {
			try {
				arquivoSaida.createNewFile();
			} catch (IOException e) {
				System.err.println("Nao foi possivel criar o arquivo " + saida);
			}
		} else {
      Writer arquivo;
      try {
        arquivo = new BufferedWriter(new FileWriter(this.saida, false));
        arquivo.append("");
        arquivo.close();
      } catch (IOException e) {
        System.err.println("Problema ao escrever arquivo. Verifique sua integridade.");
      }
    }
  }

  // Recebe um bloco e o retorna ordenado crescentemente
  private int[] ordenaBloco(int[] bloco) {
    int[] blocoOrdenado = bloco;
    int k, j, aux, tamBloco = bloco.length;

    for (k = 1; k < tamBloco; k++) {
      for (j = 0; j < tamBloco - k; j++) {
        if (bloco[j] > bloco[j + 1]) {
          aux = bloco[j];
          bloco[j] = bloco[j + 1];
          bloco[j + 1] = aux;
        }
      }
    }
    return blocoOrdenado;
  }

  // Escreve o bloco atual do consumidor no arquivo de saida
  private void escreveBloco() {
    Writer arquivo;
    try {
      arquivo = new BufferedWriter(new FileWriter(this.saida, true));
      for(int i = 0; i < bloco.length; i++) arquivo.append(bloco[i] + " ");
      arquivo.append("\n");
      arquivo.close();
    } catch (IOException e) {
      System.err.println("Problema ao escrever arquivo. Verifique sua integridade.");
    }
    this.buffer.insereBlocoConc(bloco);
  }

  // Execucao da thread
  public void run () {
    for (;this.buffer.hasNextBloco();) {
      try {
        int[] blocoAux = this.buffer.Remove();
        if(blocoAux == null) return;
        this.bloco = this.ordenaBloco(blocoAux.clone()); // Bloco atual em ordem crescente.
        this.monitor.EntraEscritor(id);
        this.escreveBloco();
        this.monitor.SaiEscritor(id);
        sleep(delay);
      } catch (InterruptedException e) {
        return;
      }
    }
  }
}

// Monitor que implementa a logica do padrao leitores/escritores
class Monitor {
  private int leit, escr;  
  
  // Construtor
  Monitor() { 
    this.leit = 0; // Leitores lendo (0 ou mais)
    this.escr = 0; // Escritor escrevendo (0 ou 1)
  } 
  
  // Entrada para leitores
  public synchronized void EntraLeitor () {
    try { 
      while (this.escr > 0) { wait(); }
      this.leit++;
    } catch (InterruptedException e) { }
  }
  
  // Saida para leitores
  public synchronized void SaiLeitor () {
    this.leit--; 
    if (this.leit == 0) this.notify(); 
  }
  
  // Entrada para escritores
  public synchronized void EntraEscritor (int id) {
    try { 
      while ((this.leit > 0) || (this.escr > 0)) {
        wait();  
      }
      this.escr++;
    } catch (InterruptedException e) { }
  }
  
  // Saida para escritores
  public synchronized void SaiEscritor (int id) {
    this.escr--; 
    notifyAll(); 
  }
}

// Classe principal do programa
class Main {
  public static void main (String[] args) {

    // Verifica se todas as informacoes necessarias para o programa foram passadas por argumento
    if(args.length < 4) {
      System.out.print("Falta(m) argumento(s). Escreva: ");
      System.out.println("java Main <numero_threads> <tamanho_bloco> <caminho_arquivo_entrada> <nome_arquivo_entrada>");
      System.out.println();
      return;
    }

    // Recebendo os argumentos do programa
    int C = Integer.parseInt(args[0]);
    int N = Integer.parseInt(args[1]);
    String entrada = args[2];
    String saida = args[3];

    // Inicialização de variaveis e threads do programa
    int i;
    Produtor prod;
    Consumidor[] cons = new Consumidor[C];
    Monitor monitor = new Monitor();

    // Variáveis para calculo de tempo de execucao
    long inicioConc, fimConc;
    double duracaoConc;
    
    BufferLimitado buffer = new BufferLimitado(N, entrada, saida);

    inicioConc = System.currentTimeMillis();

    // Iniciando a unica thread produtora do programa
    prod = new Produtor(1, buffer);
    prod.start();
    
    // Inicando as threads consumidoras
    for (i=0; i<C; i++) {
      cons[i] = new Consumidor(i+2, buffer, monitor, saida);
      cons[i].start();
    }

    // Aguarda as threads finalizarem suas execuções
    try {
      prod.join();
    } catch (InterruptedException e) {
      System.err.println("ERRO! --> Thread Produtora foi interrompida");
    }

    for (i=0; i<C; i++) {
      try {
        cons[i].join();
      } catch (InterruptedException e) {
        System.err.println("ERRO! --> Thread Consumidora "+ (i+2) + " foi interrompida");
      }
    }

    fimConc = System.currentTimeMillis();

    duracaoConc = (fimConc - inicioConc) / 1000.0;

    System.out.println();
    char s = C > 1 ? 's' : '\0';
    System.out.printf("Tempo de execucao concorrente com %d thread%c consumidora%c foi de %f segundos", C, s, s, duracaoConc);
    System.out.println();

    // Verifica corretude da implementacao concorrente
    VerificaCorretude v = new VerificaCorretude(N, entrada, saida);

    // Sinaliza fim do programa
    if(v.verifica(buffer.getBlocosConc())) System.out.println("Blocos escritos e ordenados corretamente em " + saida + " com sucesso!");
    else System.out.println("A solucao nao esta correta");
    System.out.println("Encerrando programa...\n");
  }
}