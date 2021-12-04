import java.rmi.*;

public class DodajClient 
{
  public static void main( String[] argv ) throws Exception
  { 
     DodajInterface dodaj = 
          (DodajInterface) Naming.lookup( "rmi://localhost/dodawacz");
          
     Dodawacz d = new Dodawacz( 0 );
          
     for ( int i = 0; i < 10; i++ )
     {
        int wynik = ( dodaj.dodaj( d, i ) ).get();
        System.out.println( "I : " + i + " wynik : " + wynik );     
     }
     
     System.out.println( "A teraz lokalnie" );

     DodajImpl di = new DodajImpl();

     for ( int i = 0; i < 10; i++ )
     {
        int wynik = ( di.dodaj( d, i ) ).get();
        System.out.println( "I : " + i + " wynik : " + wynik );     
     }

     System.exit(0);
  }
}