import java.rmi.RemoteException;
import java.rmi.server.UnicastRemoteObject;
import java.util.List;
import java.util.Random;

public class SerwisProcessor extends UnicastRemoteObject implements PolygonalChainProcessor {
    private int concurrentTasksLimit;

    protected SerwisProcessor() throws RemoteException {
        super();
        concurrentTasksLimit = 3;
    }

    @Override
    public int getConcurrentTasksLimit() throws RemoteException {
        return concurrentTasksLimit;
    }

    @Override
    public int process(String name, List<Position2D> polygonalChain) throws RemoteException {
        Random random = new Random();
        int returnValue = random.nextInt(10);
        try {
            System.out.println("Komunikat ze srodka metody process. Dla krzywej " + name + " otrzymano : " + polygonalChain.toString());
            Thread.sleep(2000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        return returnValue;
    }   
}