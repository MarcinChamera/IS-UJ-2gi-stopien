import java.rmi.*;

interface DodajInterface extends Remote
{
   Dodawacz dodaj( Dodawacz d, int ile ) throws RemoteException; 
}