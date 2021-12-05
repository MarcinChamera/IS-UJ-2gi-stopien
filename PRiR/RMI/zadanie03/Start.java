import java.rmi.*;
import java.util.ArrayList;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import javax.naming.*;

public class Start {
    private ImplPolygonalChain implPolygonalChain;

    Start() throws Exception {
        implPolygonalChain = new ImplPolygonalChain();
        Context namingContext = new InitialContext();
        namingContext.unbind("rmi:POLYGONAL_CHAIN");
        try {
            Thread.sleep(500);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        namingContext.bind( "rmi:POLYGONAL_CHAIN", implPolygonalChain );
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

        String polygonalChainProcessorName = null;
        while(polygonalChainProcessorName == null) {
            polygonalChainProcessorName = start.implPolygonalChain.getPolygonalChainProcessorName();
            Thread.sleep(100);
        }
        PolygonalChainProcessor polygonalChainProcessor = (PolygonalChainProcessor) Naming.lookup(polygonalChainProcessorName);

        int maxPolygonalChains = polygonalChainProcessor.getConcurrentTasksLimit();
        ExecutorService executor = Executors.newFixedThreadPool(maxPolygonalChains);

        ArrayList<NameWithFuture> results = new ArrayList<NameWithFuture>();
        Runnable runnable = () -> {
            while(true) {
                checkIfAnyPolygonalChainProcessed(results, start.implPolygonalChain);
                try {
                    Thread.sleep(20);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        };
        new Thread(runnable).start();

        while (true) {
            final ImplPolygonalChain.PolygonalChainWithName polygonalChainWithName = start.implPolygonalChain.getFromQueue();
            if (polygonalChainWithName != null) {
                Callable<Integer> callableSend = () -> {
                    return polygonalChainProcessor.process(polygonalChainWithName.getName(), polygonalChainWithName.getPolygonalChain());
                };
                Future<Integer> result = executor.submit(callableSend);
                results.add(start.new NameWithFuture(polygonalChainWithName.getName(), result));
            }
        }
    }

    private static void checkIfAnyPolygonalChainProcessed(ArrayList<NameWithFuture> results, ImplPolygonalChain implPolygonalChain) {
        ArrayList<Integer> indicesOfProcessedPolygonalChains = new ArrayList<Integer>();
        for (int i = 0; i < results.size(); i++) {
            if (results.get(i).getFuture().isDone()) {
                try {
                    implPolygonalChain.addProcessedPolygonalChain(results.get(i).getName(), results.get(i).getFuture().get());
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