import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.util.InputMismatchException;
import java.util.Scanner;

class BufferLimitado {
  private int[] buffer;
  private int N, count = 0, in = 0, out = 0;

  // Construtor
  BufferLimitado(int N, String entrada) {
    this.N = N;
    Scanner s;

    try {
      s = new Scanner(new FileInputStream(entrada));
      int elementos = s.nextInt();
      int blocos = elementos/N;
      this.buffer = new int[elementos];

      while(s.hasNextInt()) {
        System.out.println(s.nextLine());
      }
      s.close();
    } catch (FileNotFoundException e) {
      e.printStackTrace();
    }

    for (int i = 0; i < N; i++) {
      //inicia o buffer
    }
  }
  
  public synchronized void Insere (int elemento) {
    try {
      while (count == N) { wait(); } //bloqueio condicão lógica
      buffer[in] = elemento;
      in = (in + 1) % N; count++;
      notifyAll();
    } catch (InterruptedException e) { }
  }

  public synchronized int Remove () {
    int elemento;
    try {
      while (count == 0) { wait(); } //bloqueio condição lógica
      elemento = buffer[out % N];
      out = (out + 1) % N; count--;
      notifyAll();
      return elemento;
    } catch(InterruptedException e) {return -1;}
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
      this.entrada.close();
    } catch (FileNotFoundException e) {
      e.printStackTrace();
    }
  }

  public void run () {
    for (;this.entrada.hasNextInt();) {
      try {
        this.buffer.Insere(entrada.nextInt());
        sleep(500);
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
  static final int L = 3;
  static final int E = 2;

  public static void main (String[] args) {
    int i;
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