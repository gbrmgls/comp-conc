import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.util.InputMismatchException;
import java.util.Scanner;

class BufferLimitado {
  private int[] buffer, entrada;
  private int N, elementos, count = 0, in = 0, out = 0;

  // Construtor
  BufferLimitado(int N, String entrada) {
    this.N = N;
    Scanner s;

    try {
      s = new Scanner(new FileInputStream(entrada));
      this.elementos = s.nextInt();
      this.buffer = new int[N];
      this.entrada = new int[this.elementos];

      s.close();
    } catch (FileNotFoundException e) {
      e.printStackTrace();
    }
  }

  public int getN() { return this.N; }

  public void printBuffer() {
    for(int i = 0; i < this.elementos; i++) {
      if((i+1) % this.N == 0 && i != 0) System.out.println(this.entrada[i]);
      else if(i == this.elementos - 1) System.out.print(this.entrada[i]);
      else System.out.print(this.entrada[i] + " ");
    }
  }

  public synchronized void Insere (int elemento) {
    try {
      while (count == this.N) { wait(); } //bloqueio condicão lógica
      // System.out.println("Inserindo elemento: " + elemento);
      buffer[in] = elemento;
      in = (in + 1) % this.N; count++;
      notifyAll();
    } catch (InterruptedException e) { }
  }

  public synchronized int[] Remove () {
    int[] elementos = new int[this.N];
    try {
      while (count == 0 || count != this.N) { wait(); } //bloqueio condição lógica

      while(count > 0) {
        elementos[out] = buffer[out % this.N];
        out = (out + 1) % this.N; count--;
      }

      notifyAll();
      return elementos;
    } catch(InterruptedException e) { return new int[1]; }
  }
}

class Produtor extends Thread {
  int id, elementos;
  Scanner entrada;
  BufferLimitado buffer;
  Produtor(int id, String entrada, BufferLimitado buffer) {
    this.id = id;
    this.buffer = buffer;
    try {
      this.entrada = new Scanner(new FileInputStream(entrada));
      this.elementos = this.entrada.nextInt();
    } catch (FileNotFoundException e) {
      e.printStackTrace();
    }
  }

  public void run () {
    for (;this.entrada.hasNextInt();) {
      try {
        this.buffer.Insere(entrada.nextInt());
        sleep(0);
      } catch (InterruptedException e) {
        return;
      }
    }
    // buffer.printBuffer();
  }
}

class Consumidor extends Thread {
  int id;
  int[] bloco;
  BufferLimitado buffer;
  Consumidor(int id, BufferLimitado buffer) {
    this.id = id;
    this.buffer = buffer;
    this.bloco = new int[buffer.getN()];
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

  public void run () {
    for (;;) {
      try {
        this.bloco = this.ordenaBloco(this.buffer.getN(), this.buffer.Remove().clone());
        
        for(int i = 0; i < bloco.length; i++) System.out.print(bloco[i] + " ");
        System.out.println();
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
  
  public synchronized void EntraLeitor (int id) {
    try { 
      while (this.escr > 0) {
        System.out.println ("le.leitorBloqueado("+id+")");
        wait();
      }
      this.leit++;
      System.out.println ("le.leitorLendo("+id+")");
    } catch (InterruptedException e) { }
  }
  
  public synchronized void SaiLeitor (int id) {
    this.leit--; 
    if (this.leit == 0) 
      this.notify(); 
    System.out.println ("le.leitorSaindo("+id+")");
  }
  
  public synchronized void EntraEscritor (int id) {
    try { 
      while ((this.leit > 0) || (this.escr > 0)) {
        System.out.println ("le.escritorBloqueado("+id+")");
        wait();  
      }
      this.escr++;
      System.out.println ("le.escritorEscrevendo("+id+")");
    } catch (InterruptedException e) { }
  }
  
  public synchronized void SaiEscritor (int id) {
    this.escr--; 
    notifyAll(); 
    System.out.println ("le.escritorSaindo("+id+")");
  }
}

class Leitor extends Thread {
  int id;
  Monitor monitor;

  Leitor (int id, Monitor m) {
    this.id = id;
    this.monitor = m;
  }

  public void run () {
    for (;;) {
      try {
        this.monitor.EntraLeitor(this.id);
        // Lê
        this.monitor.SaiLeitor(this.id);
        sleep(1500);
      } catch (InterruptedException e) {
        return;
      }
    }
  }
}

class Escritor extends Thread {
  int id;
  Monitor monitor;

  Escritor (int id, Monitor m) {
    this.id = id;
    this.monitor = m;
  }

  public void run () {
    for (;;) {
      try {
        this.monitor.EntraEscritor(this.id);
        // Escreve
        this.monitor.SaiEscritor(this.id);
        sleep(1500);
      } catch (InterruptedException e) {
        return;
      }
    }
  }
}

class Main {
  static final int Prod = 1;
  static final int Cons = 4;
  static final int L = 3;
  static final int E = 2;

  public static void main (String[] args) {
    int i;
    Produtor prod;
    Consumidor cons;
    Monitor monitor = new Monitor();
    Leitor[] l = new Leitor[L];
    Escritor[] e = new Escritor[E]; 

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

    prod = new Produtor(1, entrada, buffer);
    prod.start();

    cons = new Consumidor(2, buffer);
    cons.start();
    // for (i=0; i<L; i++) {
    //    l[i] = new Leitor(i+1, monitor, numero);
    //    l[i].start(); 
    // }
    
    // for (i=0; i<E; i++) {
    //    e[i] = new Escritor(i+1, monitor, numero);
    //    e[i].start(); 
    // }

  }
}