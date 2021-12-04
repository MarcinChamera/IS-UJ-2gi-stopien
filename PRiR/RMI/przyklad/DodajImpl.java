import java.rmi.*;
import java.rmi.server.*;

public class DodajImpl extends UnicastRemoteObject implements DodajInterface
{
  private int suma = 0;

  public Dodawacz dodaj( Dodawacz d, int ile ) throws RemoteException
  {
    d.add( ile );
    return d;
  }

  public DodajImpl()  throws RemoteException
  { 
    super();
  }
}