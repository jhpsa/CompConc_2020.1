//código de um programa concorrente em java para somar dois vetores


//importando as classes Vector e Random que serão utilizadas no programa
import java.util.Vector;
import java.util.Random;


//classe para conter um vetor compartilhado entre as threads
class V {
  
  //vetor que será compartilhado
  private Vector<Integer> v;
  
  //construtor
  public V(int n){
    this.v = new Vector<>(n); //inicializando o vetor de tamanho n
    for(int i = 0; i < n; i++){
      v.add(0); //enchendo o vetor de zeros
    }
  }
  
  //tamanho do vetor compartilhado
  public int tamanho(){
    return v.size();
  }
  
  //operação de alteração de um elemento do vetor compartilhado
  public void altera(int i, int e){
    v.set(i, e);
  }
  
  //elemento do vetor compartilhado na posição i
  public int pega(int i){
    return v.get(i);
  }
  
  //operação de impressão do vetor compartilhado
  public void imprime(){
    for(int i = 0; i < v.size(); i++){
      System.out.println(v.get(i));
    }
  }

}


//classe que estende Thread e implementa a tarefa de cada thread do programa
class T1 extends Thread{
  
  //identificador da thread
  private int id;
  
  //objetos que contém vetores que serão compartilhados com outras threads
  V v1, v2, v3;
  
  //construtor
  public T1(int tid, V v1, V v2, V v3){
    this.id = tid;
    this.v1 = v1;
    this.v2 = v2;
    this.v3 = v3;
  }
  
  //método main da thread
  public void run() {
      System.out.println("Thread " + this.id + " iniciou!");
      for (int i=0; i<v3.tamanho(); i++) {    //alterando os valores zerados do vetor contido em v3(C) pela
         v3.altera(i, v1.pega(i)+v2.pega(i)); //soma dos valores de v1(A) e v2(B) em cada posição i
      }
      System.out.println("Thread " + this.id + " terminou!"); 
   }

}


//classe da aplicação
class Soma{
  
  //número de elementos dos vetores contidos em A, B e C
  static final int num = 3;
  
  //número de threads utilizadas (menor ou igual a num)
  static final int N = 2;
  
  public static void main (String[] args) {
  
      //reserva espaço para um vetor de N threads
      Thread[] threads = new Thread[N];

      //cria três instâncias dos vetores compartilhados entre as threads
      V A = new V(num);
      V B = new V(num);
      V C = new V(num);
      
      //altera os valores zerados dos vetores contidos em A e B por valores aleatórios entre 0 e 99
      Random r = new Random();
      for (int i = 0; i < num; i++){
        A.altera(i, r.nextInt(100));
        B.altera(i, r.nextInt(100));
      }

      //cria as threads da aplicação
      for (int i=0; i<threads.length; i++) {
         threads[i] = new T1(i, A, B, C);
      }

      //inicia as threads
      for (int i=0; i<threads.length; i++) {
         threads[i].start();
      }

      //espera pelo término de todas as threads
      for (int i=0; i<threads.length; i++) {
         try { threads[i].join(); } catch (InterruptedException e) { return; }
      }
      
      //imprime os valores atualizados (soma dos valores de A e B) do vetor contido em C
      System.out.println("Valores finais do vetor C:");
      C.imprime();
   }

}
