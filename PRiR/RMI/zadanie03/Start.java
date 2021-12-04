// 1. Kolejka bezpieczna watkowo
// 2. Metody niech beda synchronized
// 3. Mozna uzyc egzekutorow

import java.rmi.*;
import java.util.ArrayList;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import javax.naming.*;

public class Start {
    private SerwisMoj serwisMoj;

    Start() throws Exception {
        // uruchomienie serwisu
        serwisMoj = new SerwisMoj();
        Context namingContext = new InitialContext();
        namingContext.bind( "rmi:POLYGONAL_CHAIN", serwisMoj );
    }

    private class NameWithFuture {
        private String name;
        private Future<Integer> future;

        NameWithFuture(String name, Future<Integer> future) {
            this.name = name;
            this.future = future;
        }

        protected String getName() {
            return name;
        }

        protected Future<Integer> getFuture() {
            return future;
        }
    }

    public static void main( String[] argv ) throws Exception {
        Start start = new Start();
        // czekaj na zdalne wywołanie metody setPolygonalChainProcessorName zeby zapisalo sie uri
        for (int i = 0; i < 5; i++) {
            System.out.println("Czekam na odebranie URI serwisu PolygonalChainProcessor...");
            Thread.sleep(1000);
        }
        String polygonalChainProcessorName = start.serwisMoj.getPolygonalChainProcessorName();
        PolygonalChainProcessor polygonalChainProcessor = (PolygonalChainProcessor) Naming.lookup(polygonalChainProcessorName);
        int maxPolygonalChains = polygonalChainProcessor.getConcurrentTasksLimit();
        ExecutorService executor = Executors.newFixedThreadPool(maxPolygonalChains);
        // metody newPolygonalChain i addLineSegment beda wywolywane zdalnie przez 
        // polygonalChainProcessor

        ArrayList<NameWithFuture> results = new ArrayList<NameWithFuture>();
        // sprawdzaj co 20 milisekund czy jakis rezultat z metody process nie zostal zwrocony
        Runnable runnable = () -> {
            while(true) {
                checkIfAnyPolygonalChainProcessed(results, start.serwisMoj);
                try {
                    Thread.sleep(20);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        };
        new Thread(runnable).start();

        // sciagaj jak najszybciej z kolejki zlozone polygonalChainy
        while (true) {
            final SerwisMoj.PolygonalChainWithName polygonalChainWithName = start.serwisMoj.getFromQueue();
            // jesli serwisMoj ma juz przygotowana linie lamana, to wyslij ja do polygonalChainProcessor
            if (polygonalChainWithName != null) {
                Callable<Integer> callableSend = () -> {
                    return polygonalChainProcessor.process(polygonalChainWithName.getName(), polygonalChainWithName.getPolygonalChain());
                };
                Future<Integer> result = executor.submit(callableSend);
                results.add(start.new NameWithFuture(polygonalChainWithName.getName(), result));
            }
        }
    }

    private static void checkIfAnyPolygonalChainProcessed(ArrayList<NameWithFuture> results, SerwisMoj serwisMoj) {
        ArrayList<Integer> indicesOfProcessedPolygonalChains = new ArrayList<Integer>();
        for (int i = 0; i < results.size(); i++) {
            if (results.get(i).getFuture().isDone()) {
                try {
                    serwisMoj.addProcessedPolygonalChain(results.get(i).getName(), results.get(i).getFuture().get());
                    indicesOfProcessedPolygonalChains.add(i);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                } catch (ExecutionException e) {
                    e.printStackTrace();
                }
            }
        }
        for (int i : indicesOfProcessedPolygonalChains) {
            results.remove(i);
        }
    }
}