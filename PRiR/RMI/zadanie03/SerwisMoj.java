import java.rmi.*;
import java.rmi.server.*;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.concurrent.SynchronousQueue;

public class SerwisMoj extends UnicastRemoteObject implements PolygonalChain {
    private String polygonalChainProcessorUri;
    private HashMap<String, ArrayList<LineSegment>> receivedLineSegments;
    private HashMap<String, ArrayList<Position2D>> polygonalChains;
    private SynchronousQueue<PolygonalChainWithName> queue;
    private HashMap<String, Integer> processedPolygonalChains;
    private final Object monitor = new Object();


    private class LineSegment {
        private Position2D firstPoint;
        private Position2D lastPoint;

        private LineSegment(Position2D firstPoint, Position2D lastPoint) {
            this.firstPoint = firstPoint;
            this.lastPoint = lastPoint;
        }
    }

    protected class PolygonalChainWithName {
        private String name;
        private ArrayList<Position2D> polygonalChain;

        private PolygonalChainWithName(String name, ArrayList<Position2D> polygonalChain) {
            this.name = name;
            this.polygonalChain = new ArrayList<Position2D>(polygonalChain);
        }

        protected String getName() {
            return name;
        }

        protected ArrayList<Position2D> getPolygonalChain() {
            return polygonalChain;
        }
    }

    protected SerwisMoj() throws RemoteException {
        super();
        receivedLineSegments = new HashMap<String, ArrayList<LineSegment>>();
        polygonalChains = new HashMap<String, ArrayList<Position2D>>();
        queue = new SynchronousQueue<PolygonalChainWithName>();
        processedPolygonalChains = new HashMap<String, Integer>();
    }

    @Override
    public void newPolygonalChain(String name, Position2D firstPoint, Position2D lastPoint) throws RemoteException {
        polygonalChains.put(name, new ArrayList<Position2D>());
        receivedLineSegments.put(name, new ArrayList<LineSegment>());
        polygonalChains.get(name).add(firstPoint);
        polygonalChains.get(name).add(lastPoint);
    }

    private void updateOrderedLineSegments(String name) {
        boolean search = true;
        while (search) {
            search = false;
            for (int i = 0; i < receivedLineSegments.get(name).size(); i++) {
                for (int j = 0; j < polygonalChains.get(name).size(); j++) {
                    // jesli pierwszy punkt odcinka jest rowny punktowi w kolekcji uporzadkowanych punktow obecnie przetwarzanej linii lamanej
                    if (receivedLineSegments.get(name).get(i).firstPoint == polygonalChains.get(name).get(j)) {
                        // to dodaj drugi punkt odcinka do tej kolekcji, w miejscu tuz po tym punkcie z powyzszego warunku
                        polygonalChains.get(name).add(j + 1, receivedLineSegments.get(name).get(i).lastPoint);
                        // jesli dodano nowy punkt to moze bedzie mozna wstawic jeszcze kolejne - szukaj jeszcze raz od poczatku
                        search = true;
                    }
                    // jesli drugi punkt odcinka jest rowny punktowi w kolekcji uporzadkowanych punktow obecnie przetwarzanej linii lamanej
                    if (receivedLineSegments.get(name).get(i).lastPoint == polygonalChains.get(name).get(j)) {
                        // to dodaj pierwszy punkt odcinka do tej kolekcji, w miejscu tuz przed tym punkcie z powyzszego warunku
                        polygonalChains.get(name).add(j, receivedLineSegments.get(name).get(i).firstPoint);
                        // jesli dodano nowy punkt to moze bedzie mozna wstawic jeszcze kolejne - szukaj jeszcze raz od poczatku
                        search = true;
                    }
                }
            }
            // mozliwe ze trzeba bedzie dodac usuwanie lineSegmentow dodanych juz do polygonalChain, zeby przeszukiwanie w petli nie trwalo za dlugo
        }
        
    }

    private void checkIfOrderedLineSegmentsReady(String name) {
        // odcinkow bedzie o jeden mniej niz punktow dla linii lamanej
        if (receivedLineSegments.get(name).size() == polygonalChains.get(name).size() - 1) {
            try {
                queue.put(new PolygonalChainWithName(name, new ArrayList<Position2D>(polygonalChains.get(name))));
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
    }

    @Override
    public void addLineSegment(String name, Position2D firstPoint, Position2D lastPoint) throws RemoteException {
        receivedLineSegments.get(name).add(0, new LineSegment(firstPoint, lastPoint));
        synchronized (monitor) {
            updateOrderedLineSegments(name);
        }
        checkIfOrderedLineSegmentsReady(name);
    }

    @Override
    public Integer getResult(String name) throws RemoteException {
        if (processedPolygonalChains.get(name) != null) {
            return processedPolygonalChains.get(name);
        }
        return null;
    }

    @Override
    public void setPolygonalChainProcessorName(String uri) {
        this.polygonalChainProcessorUri = uri;
    }

    public String getPolygonalChainProcessorName() {
        return polygonalChainProcessorUri;
    }

    public PolygonalChainWithName getFromQueue() throws InterruptedException {
        return queue.take();
    }
    
    protected void addProcessedPolygonalChain(String name, Integer result) {
        processedPolygonalChains.put(name, result);
    }
}
