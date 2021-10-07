import java.io.BufferedWriter;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;
import java.util.Scanner;

/*  TO DO

    - Finalizar a execucao do programa apropriadamente:
      -> Ha threads consumidoras que nao finalizam apos nao haver mais blocos para escrever
    
    - Log/feedback informando que os arquivos ja foram lidos/escritos completamente
*/

// Classe que representa o buffer e os metodos a que ele eh relevante
class BufferLimitado {
  private int[] buffer;
  private int N, count = 0, in = 0, out = 0, blocosBuffer = 10;
  Scanner entradaScanner;

  // Construtor
  BufferLimitado(int N, String entrada) {
    this.N = N;
    try {
      this.entradaScanner = new Scanner(new FileInputStream(entrada)); // Guarda os dados do arquivo para ler posteriormente
      this.entradaScanner.nextInt(); // Consome o valor que representa o numero elementos no arquivo de entrada
      this.buffer = new int[this.blocosBuffer*N]; // Inicializa o buffer 10xN
    } catch (FileNotFoundException e) {
      e.printStackTrace();
    }
  }

  // Retorna o tamanho dos blocos
  public int getN() { return this.N; }

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
      System.out.println("Inserindo elemento: " + elemento);
      notifyAll();
    } catch (InterruptedException e) { }
  }

  // Remove e retorna um bloco de elementos do buffer.
  // Primeiro aguarda preencher um bloco e depois o remove completamente
  public synchronized int[] Remove () {
    int[] elementos = new int[this.N];
    try {
      while (count == 0 || (count % this.N) != 0) { wait(); } // Bloqueio condicao logica. Aguarda pelo menos um bloco preencher.

      // Retira exatamente um bloco
      int aux = count - this.N;
      while(count > aux) {
        elementos[out % this.N] = buffer[out % (buffer.length)];
        out = (out + 1) % (buffer.length); count--;
      }

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
      try {
        this.buffer.Insere();
        sleep(0);
      } catch (InterruptedException e) {
        return;
      }
    }
  }
}

// Classe que representa um consumidor e seus metodos
class Consumidor extends Thread {
  int id;
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
  }

  // Execucao da thread
  public void run () {
    for (;;) {
      try {
        this.bloco = this.ordenaBloco(this.buffer.getN(), this.buffer.Remove().clone()); // Bloco atual em ordem crescente.
        this.monitor.EntraEscritor(id);
        this.escreveBloco();
        this.monitor.SaiEscritor(id);
        sleep(0);
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
        // System.out.println ("le.escritorBloqueado("+id+")");
        wait();  
      }
      this.escr++;
      // System.out.println ("le.escritorEscrevendo("+id+")");
    } catch (InterruptedException e) { }
  }
  
  // Saida para escritores
  public synchronized void SaiEscritor (int id) {
    this.escr--; 
    notifyAll(); 
    // System.out.println ("le.escritorSaindo("+id+")");
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
    
    BufferLimitado buffer = new BufferLimitado(N, entrada);

    // Iniciando a unica thread produtora do programa
    prod = new Produtor(1, buffer);
    prod.start();
    
    // Inicando as threads consumidoras
    for (i=0; i<C; i++) {
      cons[i] = new Consumidor(i+2, buffer, monitor, saida);
      cons[i].start();
    }
  }
}