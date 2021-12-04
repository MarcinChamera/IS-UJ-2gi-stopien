import java.io.*;

class Dodawacz implements Serializable 
{
   private int value;
   public Dodawacz( int ile )
   {
      value = ile;
   }
   public void add( int ile )
   {
      value += ile;
   }
   public int get( )
   {
      return value;
   }
}