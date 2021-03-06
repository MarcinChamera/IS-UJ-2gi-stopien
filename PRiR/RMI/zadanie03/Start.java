import java.rmi.*;
import java.util.ArrayList;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import javax.naming.*;
import java.rmi.server.*;
import java.util.HashMap;
import java.util.LinkedHashSet;
import java.util.concurrent.SynchronousQueue;

public class Start {
    public Start() throws Exception {
        Context namingContext = new InitialContext();

        class ImplPolygonalChain extends UnicastRemoteObject implements PolygonalChain {
            private String polygonalChainProcessorUri;
            private HashMap<String, ArrayList<LineSegment>> receivedLineSegments;
            private HashMap<String, ArrayList<LineSegment>> orderedLineSegments;
            private HashMap<String, ArrayList<Position2D>> polygonalChains;
            private SynchronousQueue<PolygonalChainWithName> queue;
            private ArrayList<String> readyPolygonalChainsNames;
            private HashMap<String, Future<Integer>> sentPolygonalChains;
        
            protected ImplPolygonalChain() throws RemoteException {
                super();
                receivedLineSegments = new HashMap<String, ArrayList<LineSegment>>();
                orderedLineSegments = new HashMap<String, ArrayList<LineSegment>>();
                polygonalChains = new HashMap<String, ArrayList<Position2D>>();
                queue = new SynchronousQueue<PolygonalChainWithName>();
                sentPolygonalChains = new HashMap<String, Future<Integer>>();
                readyPolygonalChainsNames = new ArrayList<String>();
            }
        
            class LineSegment {
                private Position2D firstPoint;
                private Position2D lastPoint;
        
                private LineSegment(Position2D firstPoint, Position2D lastPoint) {
                    this.firstPoint = firstPoint;
                    this.lastPoint = lastPoint;
                }
        
                @Override
                public String toString() {
                    return firstPoint.toString() + " " + lastPoint.toString();
                }
        
                @Override
                public boolean equals(Object obj) {
                    if (this == obj)
                        return true;
                    if (obj == null)
                        return false;
                    if (getClass() != obj.getClass())
                        return false;
                    LineSegment other = (LineSegment) obj;
                    return firstPoint.equals(other.firstPoint) && lastPoint.equals(other.lastPoint);
                }
            }
        
            class PolygonalChainWithName {
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
        
            @Override
            public void newPolygonalChain(String name, Position2D firstPoint, Position2D lastPoint) throws RemoteException {
                polygonalChains.put(name, new ArrayList<Position2D>());
                receivedLineSegments.put(name, new ArrayList<LineSegment>());
                orderedLineSegments.put(name, new ArrayList<LineSegment>());
                polygonalChains.get(name).add(firstPoint);
                polygonalChains.get(name).add(lastPoint);
                orderedLineSegments.get(name).add(new LineSegment(new Position2D(-50000, -50000), firstPoint));
                orderedLineSegments.get(name).add(new LineSegment(lastPoint, new Position2D(50000, 50000)));
            }
        
            @Override
            public synchronized void addLineSegment(String name, Position2D firstPoint, Position2D lastPoint) throws RemoteException {
                receivedLineSegments.get(name).add(0, new LineSegment(firstPoint, lastPoint));
                Runnable runnable1 = () -> {
                    updateOrderedLineSegments(name);
                };
                Runnable runnable2 = () -> {
                    checkIfOrderedLineSegmentsReady(name);
                };
                new Thread(runnable1).start();
                new Thread(runnable2).start();
            }
        
            // private synchronized void updateOrderedLineSegments(String name) {
            //     ArrayList<LineSegment> receivedLineSegments = this.receivedLineSegments.get(name);
            //     ArrayList<LineSegment> orderedLineSegments = this.orderedLineSegments.get(name);
            //     ArrayList<Position2D> polygonalChain = this.polygonalChains.get(name);
            //     boolean search = true;
            //     while (search) {
            //         search = false;
            //         for (int i = 0; i < receivedLineSegments.size(); i++) {
            //             for (int j = 0; j < orderedLineSegments.size(); j++) {
            //                 if (receivedLineSegments.get(i).firstPoint.equals(orderedLineSegments.get(j).lastPoint) &&
            //                     !orderedLineSegments.contains(receivedLineSegments.get(i))) {
            //                     orderedLineSegments.add(j + 1, receivedLineSegments.get(i));
            //                     polygonalChain.add(j + 1, receivedLineSegments.get(i).lastPoint);
            //                     search = true;
            //                 }
            //                 if (receivedLineSegments.get(i).lastPoint.equals(orderedLineSegments.get(j).firstPoint) &&
            //                     !orderedLineSegments.contains(receivedLineSegments.get(i))) {
            //                     orderedLineSegments.add(j, receivedLineSegments.get(i));
            //                     polygonalChain.add(j, receivedLineSegments.get(i).firstPoint);
            //                     search = true;
            //                 }
            //             }
            //         }
            //     }
            // }
        
            // private boolean checkIfReady(String name) {
            //     ArrayList<LineSegment> orderedLineSegments = this.orderedLineSegments.get(name);
            //     for (int i = 1; i < orderedLineSegments.size(); i++) {
            //         if (!orderedLineSegments.get(i).firstPoint.equals(orderedLineSegments.get(i - 1).lastPoint)) {
            //             return false;
            //         }
            //     }
            //     return true;
            // }
        
            // private synchronized void checkIfOrderedLineSegmentsReady(String name) {
            //     if (!readyPolygonalChainsNames.contains(name) && checkIfReady(name)) {
            //         try {
            //             readyPolygonalChainsNames.add(name);
            //             LinkedHashSet<Position2D> set = new LinkedHashSet<Position2D>(polygonalChains.get(name));
            //             polygonalChains.get(name).clear();
            //             polygonalChains.get(name).addAll(set);
            //             queue.put(new PolygonalChainWithName(name, new ArrayList<Position2D>(polygonalChains.get(name))));
            //         } catch (InterruptedException e) {
            //             e.printStackTrace();
            //         }
            //     }
            // }

            // @Override
            // public synchronized void addLineSegment(String name, Position2D firstPoint, Position2D lastPoint) throws RemoteException {
            //     receivedLineSegments.get(name).add(0, new LineSegment(firstPoint, lastPoint));
            //     updateOrderedLineSegments(name);
            //     checkIfOrderedLineSegmentsReady(name);
            // }
        
            private void updateOrderedLineSegments(String name) {
                boolean search = true;
                while (search) {
                    search = false;
                    for (int i = 0; i < receivedLineSegments.get(name).size(); i++) {
                        for (int j = 0; j < orderedLineSegments.get(name).size(); j++) {
                            if (receivedLineSegments.get(name).get(i).firstPoint.equals(orderedLineSegments.get(name).get(j).lastPoint) &&
                                !orderedLineSegments.get(name).contains(receivedLineSegments.get(name).get(i))) {
                                orderedLineSegments.get(name).add(j + 1, receivedLineSegments.get(name).get(i));
                                polygonalChains.get(name).add(j + 1, receivedLineSegments.get(name).get(i).lastPoint);
                                search = true;
                            }
                            if (receivedLineSegments.get(name).get(i).lastPoint.equals(orderedLineSegments.get(name).get(j).firstPoint) &&
                                !orderedLineSegments.get(name).contains(receivedLineSegments.get(name).get(i))) {
                                orderedLineSegments.get(name).add(j, receivedLineSegments.get(name).get(i));
                                polygonalChains.get(name).add(j, receivedLineSegments.get(name).get(i).firstPoint);
                                search = true;
                            }
                        }
                    }
                }
            }
        
            private boolean checkIfReady(String name) {
                for (int i = 1; i < orderedLineSegments.get(name).size(); i++) {
                    if (!orderedLineSegments.get(name).get(i).firstPoint.equals(orderedLineSegments.get(name).get(i - 1).lastPoint)) {
                        return false;
                    }
                }
                return true;
            }
        
            private void checkIfOrderedLineSegmentsReady(String name) {
                if (checkIfReady(name)) {
                    try {
                        readyPolygonalChainsNames.add(name);
                        LinkedHashSet<Position2D> set = new LinkedHashSet<Position2D>(polygonalChains.get(name));
                        polygonalChains.get(name).clear();
                        polygonalChains.get(name).addAll(set);
                        queue.put(new PolygonalChainWithName(name, new ArrayList<Position2D>(polygonalChains.get(name))));
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }
            }
        
            @Override
            public synchronized Integer getResult(String name) throws RemoteException {
                if (sentPolygonalChains.get(name).isDone()) {
                    try {
                        return sentPolygonalChains.get(name).get();
                    } catch (InterruptedException e) {
                        // TODO Auto-generated catch block
                        e.printStackTrace();
                    } catch (ExecutionException e) {
                        // TODO Auto-generated catch block
                        e.printStackTrace();
                    }
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
            
            protected void addSentPolygonalChain(String name, Future<Integer> sentPolygonalChain) {
                this.sentPolygonalChains.put(name, sentPolygonalChain);
            }
        }
        
        ImplPolygonalChain implPolygonalChain = new ImplPolygonalChain();
        
        namingContext.bind( "rmi:POLYGONAL_CHAIN", implPolygonalChain );

        String polygonalChainProcessorName = null;
        while(polygonalChainProcessorName == null) {
            polygonalChainProcessorName = implPolygonalChain.getPolygonalChainProcessorName();
        }
        PolygonalChainProcessor polygonalChainProcessor = (PolygonalChainProcessor) Naming.lookup(polygonalChainProcessorName);

        int maxPolygonalChains = polygonalChainProcessor.getConcurrentTasksLimit();
        ExecutorService executor = Executors.newFixedThreadPool(maxPolygonalChains);

        while (true) {
            final ImplPolygonalChain.PolygonalChainWithName polygonalChainWithName = implPolygonalChain.getFromQueue();
            if (polygonalChainWithName != null) {
                Callable<Integer> callableSend = () -> {
                    return polygonalChainProcessor.process(polygonalChainWithName.getName(), polygonalChainWithName.getPolygonalChain());
                };
                implPolygonalChain.addSentPolygonalChain(polygonalChainWithName.getName(), executor.submit(callableSend));
            }
        }
    }
}