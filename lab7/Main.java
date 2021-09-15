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
  Numero numero;

  Leitor (int id, Monitor m, Numero n) {
    this.id = id;
    this.monitor = m;
    this.numero = n;
  }

  public void run () {
	for (;;) {
		try {
		    this.monitor.EntraLeitor(this.id);
		    numero.ehPrimo();
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
  Numero numero;

  Escritor (int id, Monitor m, Numero n) {
    this.id = id;
    this.monitor = m;
    this.numero = n;
  }

  public void run () {
	for (;;) {
		try {
	    this.monitor.EntraEscritor(this.id); 
		    numero.setNumero(this.id);
		    this.monitor.SaiEscritor(this.id);
		    sleep(1500);
		} catch (InterruptedException e) {
			return;
		}
	}
  }
}

class LeitorEscritor extends Thread {
	  int id;
	  Monitor monitor;
	  Numero numero;

	  LeitorEscritor (int id, Monitor m, Numero n) {
	    this.id = id;
	    this.monitor = m;
	    this.numero = n;
	  }

	  public void run () {
		for (;;) {
			try {
				this.monitor.EntraLeitor(this.id); 
				numero.ehPar();
				this.monitor.SaiLeitor(this.id); 
				this.monitor.EntraEscritor(this.id); 
				numero.setNumero(numero.getNumero() * 2);
				this.monitor.SaiEscritor(this.id);
				sleep(1500);				
			} catch (InterruptedException e) {
				return;
			}
		}
	  }
	}

// Classe de dados compartilhados entre as threads
class Numero {
	private int numero;
	private boolean pyLog;
	
	Numero() {
		this.numero = 0;
	}
	
	public int getNumero() {
		return this.numero;
	}
	
	public void setNumero(int n) {
		System.out.println("#valor atual: " + n);
		this.numero = n;
	}
	
	public void ehPrimo() {
		if(this.numero <= 1) {
			System.out.println("#" + this.numero + " nao eh primo");
			return;
		}
		for(int i = 2; i <= this.numero; i++) {
			if(this.numero % i == 0 && this.numero != i) {
				System.out.println("#" + this.numero + " nao eh primo");
				return;
			}
		}
		System.out.println("#" + this.numero + " eh primo");
		return;
	}
	
	public void ehPar() {
		System.out.printf("#" + "%d eh %s\n", this.numero, this.numero % 2 == 0 ? "par" : "impar");
	}
}

class Main {
  static final int L = 3;
  static final int E = 2;
  static final int LE = 5;

  public static void main (String[] args) {
    int i;
    Monitor monitor = new Monitor();
    Leitor[] l = new Leitor[L];
    Escritor[] e = new Escritor[E]; 
    LeitorEscritor[] le = new LeitorEscritor[LE]; 
    Numero numero = new Numero();

    System.out.println ("import verificaLE");
    System.out.println ("le = verificaLE.LE()");
    
    for (i=0; i<L; i++) {
       l[i] = new Leitor(i+1, monitor, numero);
       l[i].start(); 
    }
    
    for (i=0; i<E; i++) {
       e[i] = new Escritor(i+1, monitor, numero);
       e[i].start(); 
    }
    
    for (i=0; i<LE; i++) {
        le[i] = new LeitorEscritor(i+1, monitor, numero);
        le[i].start(); 
     }
  }
}