import java.io.BufferedWriter;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;
import java.util.Scanner;

class BufferLimitado {
  private int[] buffer;
  private int N, count = 0, in = 0, out = 0, blocosBuffer = 10;
  Scanner entradaScanner;

  // Construtor
  BufferLimitado(int N, String entrada) {
    this.N = N;
    try {
      this.entradaScanner = new Scanner(new FileInputStream(entrada));
      this.entradaScanner.nextInt();
      this.buffer = new int[this.blocosBuffer*N];

    } catch (FileNotFoundException e) {
      e.printStackTrace();
    }
  }

  public int getN() { return this.N; }

  public synchronized boolean hasNextInt() {
    return this.entradaScanner.hasNextInt();
  }

  public synchronized void Insere () {
    try {
      while (count == buffer.length) { wait(); } //bloqueio condicão lógica
      int elemento = this.entradaScanner.nextInt();
      buffer[in] = elemento;
      in = (in + 1) % buffer.length; count++;
      System.out.println("Inserindo elemento: " + elemento);
      notifyAll();
    } catch (InterruptedException e) { }
  }

  public synchronized int[] Remove () {
    int[] elementos = new int[this.N];
    try {
      while (count == 0 || (count % this.N) != 0) { wait(); } //bloqueio condição lógica
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

class Produtor extends Thread {
  int id;
  BufferLimitado buffer;
  Produtor(int id, BufferLimitado buffer) {
    this.id = id;
    this.buffer = buffer;
  }

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

class Consumidor extends Thread {
  int id;
  int[] bloco;
  BufferLimitado buffer;
  Monitor monitor;
  String saida;

  Consumidor(int id, BufferLimitado buffer, Monitor monitor, String saida) {
    this.id = id;
    this.buffer = buffer;
    this.bloco = new int[buffer.getN()];
    this.monitor = monitor;
    this.saida = saida;

    this.criaArquivoVazio();
  }

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

  public void run () {
    for (;;) {
      try {
        this.bloco = this.ordenaBloco(this.buffer.getN(), this.buffer.Remove().clone());
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

class Monitor {
  private int leit, escr;  
  
  Monitor() { 
    this.leit = 0; 
    this.escr = 0;
  } 
  
  public synchronized void EntraLeitor () {
    try { 
      while (this.escr > 0) { wait(); }
      this.leit++;
    } catch (InterruptedException e) { }
  }
  
  public synchronized void SaiLeitor () {
    this.leit--; 
    if (this.leit == 0) this.notify(); 
  }
  
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
  
  public synchronized void SaiEscritor (int id) {
    this.escr--; 
    notifyAll(); 
    // System.out.println ("le.escritorSaindo("+id+")");
  }
}

class Main {
  public static void main (String[] args) {
    int i;
    Produtor prod;
    Consumidor[] cons;
    Monitor monitor = new Monitor();

    if(args.length < 4) {
      System.out.print("Falta(m) argumento(s). Escreva: ");
      System.out.println("java Main <numero_threads> <tamanho_bloco> <caminho_arquivo_entrada> <nome_arquivo_entrada>");
      System.out.println();
      return;
    }

    int C = Integer.parseInt(args[0]);
    int N = Integer.parseInt(args[1]);
    String entrada = args[2];
    String saida = args[3];
    
    BufferLimitado buffer = new BufferLimitado(N, entrada);

    cons = new Consumidor[C];
    prod = new Produtor(1, buffer);
    prod.start();

    for (i=0; i<C; i++) {
      cons[i] = new Consumidor(i+2, buffer, monitor, saida);
      cons[i].start();
    }
  }
}