package lab6;

// Contagem sequencial
class ContagemSeq {
	int pares = 0;
	int[] ints;
	
	public ContagemSeq(int[] ints) {
		this.ints = ints;
	}
	
	public void contaPares() {
		for(int i = 0; i < ints.length; i++) {
			if(ints[i]%2 == 0) this.pares++;
		}
	}
	
	public int getPares() {
		return this.pares;
	}
}

// Dados globais concorrentes
class Dados {
	int index = 0;
	int pares = 0;
	
	public synchronized int getNext() {
		return this.index++;
	}
	
	public synchronized void incPares() {
		this.pares++;
	}
	
	public int getPares() {
		return this.pares;
	}
}

// Task de cada thread
class Contador extends Thread {
	int id;
	int[] ints;
	Dados pares;

	public Contador(int tid, int[] ints, Dados pares) {
		this.id = tid;
		this.ints = ints;
		this.pares = pares;
	}
	
	public void run() {
		for(int i = pares.getNext(); i < ints.length; i = pares.getNext()) {
			if(ints[i]%2 == 0) {
				pares.incPares();
			}
		}
	}
}

public class Main {
	public static void main(String[] args) {
		
		// Constantes
		final int n_threads = 4;
		final int n_ints = 1000;
		
		// Criacao e preenchimento das estruturas de controle
		Thread[] threadList = new Thread[n_threads];
		int[] intList = new int[n_ints];
		
		
		for(int i = 0; i < n_ints; i++) {
			intList[i] = (int) (Math.random()*10);
		}
		
		// Instanciacao das classes
		Dados dados = new Dados();
		ContagemSeq paresSeq = new ContagemSeq(intList);
		
		for (int i = 0; i < n_threads; i++) {
			threadList[i] = new Contador(i, intList, dados);
			threadList[i].start();
			
		}
		
		// Aguardo ao fim de todas as threads
		for (int i = 0; i < n_threads; i++) {
	    	try { 
	    		threadList[i].join(); 
    		} catch (InterruptedException e) {
    			System.out.println("e");
			}
	    }
		
		// Execucao sequencial para verificacao
	    paresSeq.contaPares();

	    // Verificacao de corretude
	    System.out.println("Quantidade de inteiros pares (CONCORRENTE): " + dados.getPares()); 
	    System.out.println("Quantidade de inteiros pares (SEQUENCIAL): " + paresSeq.getPares()); 
	    
	    System.out.printf("Contagem %s\n", dados.getPares() == paresSeq.getPares() ? "ok" : "errada");
	}
}
