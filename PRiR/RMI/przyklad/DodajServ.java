import javax.naming.*;

public class DodajServ 
{
  public static void main( String[] argv ) throws Exception
  { 
     DodajImpl di = new DodajImpl();
     Context namingContext = new InitialContext();
     namingContext.bind( "rmi:dodawacz", di );
  }
}