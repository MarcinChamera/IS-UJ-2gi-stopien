import java.rmi.Naming;
import java.rmi.RemoteException;
import java.util.ArrayList;
import java.util.Collections;

import javax.naming.Context;
import javax.naming.InitialContext;
import javax.naming.NamingException;

public class StartProcessor {

    StartProcessor() throws RemoteException, NamingException {
        SerwisProcessor serwisProcessor = new SerwisProcessor();
        Context namingContext = new InitialContext();
        namingContext.unbind("rmi:POLYGONAL_CHAIN_PROCESSOR");
        try {
            Thread.sleep(500);
        } catch (InterruptedException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
        namingContext.bind( "rmi:POLYGONAL_CHAIN_PROCESSOR", serwisProcessor );
    }

    class LineSegment {
        private Position2D firstPoint;
        private Position2D lastPoint;

        LineSegment(Position2D firstPoint, Position2D lastPoint) {
            this.firstPoint = firstPoint;
            this.lastPoint = lastPoint;
        }
    }

    public static void main( String[] argv ) throws Exception
    { 
        StartProcessor startProcessor = new StartProcessor();
        System.out.println("Czekam teraz 5s na serwisMoj...");
        Thread.sleep(5000);
        PolygonalChain polygonalChain = (PolygonalChain) Naming.lookup("rmi://localhost/POLYGONAL_CHAIN");
        polygonalChain.setPolygonalChainProcessorName("rmi://localhost/POLYGONAL_CHAIN_PROCESSOR");

        Position2D p0 = new Position2D(-3, -5);
        Position2D p1 = new Position2D(-4, -2);
        Position2D p2 = new Position2D(-3, -2);
        Position2D p3 = new Position2D(1, 2);
        Position2D p4 = new Position2D(3, 7);
        Position2D p5 = new Position2D(4, 4);
        Position2D p6 = new Position2D(3, -2);
        Position2D p7 = new Position2D(4, -4);

        ArrayList<LineSegment> lineSegments1 = new ArrayList<LineSegment>();
        lineSegments1.add(startProcessor.new LineSegment(p0, p1));
        lineSegments1.add(startProcessor.new LineSegment(p1, p2));
        lineSegments1.add(startProcessor.new LineSegment(p2, p3));
        lineSegments1.add(startProcessor.new LineSegment(p3, p4));
        lineSegments1.add(startProcessor.new LineSegment(p4, p5));
        lineSegments1.add(startProcessor.new LineSegment(p5, p6));
        lineSegments1.add(startProcessor.new LineSegment(p6, p7));
        Collections.shuffle(lineSegments1);
        String lineSegments1Name = "Pierwsza krzywa";

        ArrayList<LineSegment> lineSegments2 = new ArrayList<LineSegment>();
        lineSegments2.add(startProcessor.new LineSegment(p7, p6));
        lineSegments2.add(startProcessor.new LineSegment(p6, p5));
        lineSegments2.add(startProcessor.new LineSegment(p5, p4));
        lineSegments2.add(startProcessor.new LineSegment(p4, p3));
        lineSegments2.add(startProcessor.new LineSegment(p3, p2));
        lineSegments2.add(startProcessor.new LineSegment(p2, p1));
        lineSegments2.add(startProcessor.new LineSegment(p1, p0));
        Collections.shuffle(lineSegments2);
        String lineSegments2Name = "Druga krzywa";

        System.out.println("Czekam 5 sekund az zaczne wysylac dane");
        Thread.sleep(5000);

        System.out.println("Wywoluje metode newPolygonalChain dla krzywej " + lineSegments1Name);
        polygonalChain.newPolygonalChain(lineSegments1Name, p0, p7);
        for (LineSegment lineSegment1 : lineSegments1) {
            new Thread(() -> {
                try {
                    System.out.println("Wywoluje metode newLineSegment dla krzywej " + lineSegments1Name + 
                    ". " + lineSegment1.firstPoint.toString() + " " + lineSegment1.lastPoint.toString());
                    polygonalChain.addLineSegment(lineSegments1Name, lineSegment1.firstPoint, lineSegment1.lastPoint);
                } catch (RemoteException e) {
                    e.printStackTrace();
                }
            }).start();
        }
        Thread.sleep(100);
        System.out.println("Wyslalem wszystkie odcinki krzywej " + lineSegments1Name + " i czekam na odpowiedz...");

        System.out.println("Wywoluje metode newPolygonalChain dla krzywej " + lineSegments2Name);
        polygonalChain.newPolygonalChain(lineSegments2Name, p7, p0);
        for (LineSegment lineSegment2 : lineSegments2) {
            new Thread(() -> {
                try {
                    System.out.println("Wywoluje metode newLineSegment dla krzywej " + lineSegments2Name + 
                    ". " + lineSegment2.firstPoint.toString() + " " + lineSegment2.lastPoint.toString());
                    polygonalChain.addLineSegment(lineSegments2Name, lineSegment2.firstPoint, lineSegment2.lastPoint);
                } catch (RemoteException e) {
                    e.printStackTrace();
                }
            }).start();
        }
        Thread.sleep(100);
        System.out.println("Wyslalem wszystkie odcinki krzywej " + lineSegments2Name + " i czekam na odpowiedz...");

        for(int i = 0; i < 10; i++) {
            System.out.println("Otrzymalem wynik metody getResult dla " + lineSegments1Name + ": " + polygonalChain.getResult(lineSegments1Name));
            System.out.println("Otrzymalem wynik metody getResult dla " + lineSegments2Name + ": " + polygonalChain.getResult(lineSegments2Name));
            Thread.sleep(500);
        }
    }
}
