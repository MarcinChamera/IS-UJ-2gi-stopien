import java.util.ArrayList;
import java.util.Random;
// import java.util.concurrent.Callable;
// import java.util.concurrent.Executors;

// TODO:
// 1. Kontroler ma obsługiwać współbieżną z aktualnymi operacjami odbudowę macierzy dyskowej.
//    new Thread(() -> odbudowa macierzy dyskowej).start();

class RAID implements RAIDInterface {
    private ArrayList<DiskInterface> logicalDisk;
    private ArrayList<Boolean> arePhysicalDisksDamaged;
    private RAIDState state;
    private int logicalDiskSize;
    private int onePhysicalDiskSize;
    private boolean isShutdown;

    class RAIDError extends Exception {
	};

    RAID() {
        logicalDisk = new ArrayList<DiskInterface>();
        arePhysicalDisksDamaged = new ArrayList<Boolean>();
        state = RAIDState.UNKNOWN;
        isShutdown = false;
    }

    // aby prawidłowo chronić zmienną stanową za pomocą blokady trzeba wszystkie operacje
    // na niej zabezpieczac za pomoca tej samej blokady
    public synchronized RAIDState getState() {
        return state;
    }

    public void addDisk(DiskInterface disk) {
        if (isShutdown == false) {
            if (logicalDisk.isEmpty()) {
                onePhysicalDiskSize = disk.size();
            }
            // rozmiar dysku logicznego = rozmiar dysku fizycznego * (N - 1) dysków fizycznych
            else {
                logicalDiskSize += disk.size();
            }
            logicalDisk.add(disk);
            arePhysicalDisksDamaged.add(false);
        }
    }

    private void rebuildCheckSum() {
        if (isShutdown == false) {
            for (int physicalSector = 0; physicalSector < onePhysicalDiskSize; physicalSector++) {
                int checkSum = 0;
                for (int physicalDiskNumber = 0; physicalDiskNumber < logicalDisk.size() - 1; physicalDiskNumber++) {
                    checkSum += read(getSectorInLogicalSpace(physicalDiskNumber, physicalSector));
                }
                write(getSectorInLogicalSpace(logicalDisk.size() - 1, physicalSector), checkSum);
            }
            state = RAIDState.NORMAL;
        }
    }

    public void startRAID() {
        state = RAIDState.INITIALIZATION;
        new Thread(() -> rebuildCheckSum()).start();
    }

    private int whichDiskIsDamaged() {
        if (isShutdown == false) {
            for (int i = 0; i < arePhysicalDisksDamaged.size(); i++) {
                if (arePhysicalDisksDamaged.get(i))
                    return i;
            }
        }
        return -1;
    }

    private int getSectorInLogicalSpace(int physicalDiskNumber, int physicalSector) {
        if (isShutdown == false) {
            return physicalDiskNumber * onePhysicalDiskSize + physicalSector;
        }
        return -1;
    }

    public void replaceDisk(DiskInterface disk) {
        if (isShutdown == false) {
            // znajdz zepsuty dysk
            final int damagedDiskNumber = whichDiskIsDamaged();
            if (damagedDiskNumber == -1)
                System.out.println("Niepoprawna proba zamiany, zaden dysk nie jest uszkodzony!");

            // zamiana dysku na nowy
            logicalDisk.set(damagedDiskNumber, disk);
            arePhysicalDisksDamaged.set(damagedDiskNumber, false);

            state = RAIDState.REBUILD;

            // POCZĄTEK ODBUDOWY MACIERZY
        
            // 1. jesli awarii ulegl ostatni dysk (kontrolny) - trzeba przeiterowac przez dyski od 0 do N - 1, dodajac wartosci z kazdego z sektorow, tak zeby
            // utworzyc nowe sumy kontrolne
            if (damagedDiskNumber == logicalDisk.size() - 1) {
                new Thread(() -> rebuildCheckSum()).start();
            }
            // 2. jesli awarii ulegl inny niż ostatni dysk - od sumy kontrolnej z ostatniego dysku trzeba odjac wartosci z sektorow innych niz ostatni i ten uszkodzony
            else {
                new Thread(() -> restoreDiskData(damagedDiskNumber)).start();
            }

            // KONIEC ODBUDOWY MACIERZY
        }
    }

    private void restoreDiskData(int damagedDiskNumber) {
        if (isShutdown == false) {
            for (int physicalSector = 0; physicalSector < onePhysicalDiskSize; physicalSector++) {
                int checkSum = read(getSectorInLogicalSpace(logicalDisk.size() - 1, physicalSector));
                for (int physicalDiskNumber = 0; physicalDiskNumber < logicalDisk.size() - 1; physicalDiskNumber++) {
                    if (physicalDiskNumber != damagedDiskNumber) {
                        checkSum -= read(getSectorInLogicalSpace(physicalDiskNumber, physicalSector));
                    }
                }
                write(getSectorInLogicalSpace(damagedDiskNumber, physicalSector), checkSum);
            }
            state = RAIDState.NORMAL;
        }
    }

    public void write(int sector, int value) {
        if (isShutdown == false) {
            int physicalDiskNumber = sector / onePhysicalDiskSize;
            // zezwalaj na dostęp do dysku, tylko jeśli nie jest uszkodzony
            if (arePhysicalDisksDamaged.get(physicalDiskNumber) == false) {
                int physicalDiskSector = sector - physicalDiskNumber * onePhysicalDiskSize;
                try {
                    synchronized (logicalDisk.get(physicalDiskNumber)) {
                        logicalDisk.get(physicalDiskNumber).write(physicalDiskSector, value);
                    }
                    if (physicalDiskNumber != logicalDisk.size() - 1) {
                        new Thread(() -> rebuildCheckSum()).start();
                    }
                } catch (DiskInterface.DiskError e) {
                    System.out.println("DiskError dla dysku: " + physicalDiskNumber + ", sektora " + physicalDiskSector + ", proba zapisania wartosci " + value);
                    state = RAIDState.DEGRADED;
                    arePhysicalDisksDamaged.set(physicalDiskNumber, true);
                    // obsługa awarii wadliwego dysku - zmiana stanu dysku z sumą kontrolną (weź pod uwagę zmianę pomiędzy obecną wartością z sektora uszkodzonego
                    // dysku, a nową, dodawaną wartością)
                    int damagedDiskNumber = physicalDiskNumber;
                    int checkSum = read(getSectorInLogicalSpace(logicalDisk.size() - 1, physicalDiskSector));
                    int damagedDiskValueFromSector = checkSum;
                    for (physicalDiskNumber = 0; physicalDiskNumber < logicalDisk.size() - 1; physicalDiskNumber++) {
                        if (physicalDiskNumber != damagedDiskNumber) {
                            damagedDiskValueFromSector -= read(getSectorInLogicalSpace(physicalDiskNumber, physicalDiskSector));
                        }
                    }
                    int valueChange = value - damagedDiskValueFromSector;
                    write(getSectorInLogicalSpace(logicalDisk.size() - 1, physicalDiskSector), checkSum + valueChange);
                }
            }
        }
    }

    // class DiskDataSectorRestorer implements Callable<Integer> {
    //     private int damagedDiskNumber;
    //     private int physicalDiskSector;

    //     DiskDataSectorRestorer(int damagedDiskNumber, int physicalDiskSector) {
    //         this.damagedDiskNumber = damagedDiskNumber;
    //         this.physicalDiskSector = physicalDiskSector;
    //     }

        // private int restoreDiskDataSector(int damagedDiskNumber, int physicalDiskSector) {
    //     int checkSum = read(getSectorInLogicalSpace(logicalDisk.size() - 1, physicalDiskSector));
    //     for (int physicalDiskNumber = 0; physicalDiskNumber < logicalDisk.size(); physicalDiskNumber++) {
    //         if (physicalDiskNumber != damagedDiskNumber) {
    //             checkSum -= read(getSectorInLogicalSpace(physicalDiskNumber, physicalDiskSector));
    //         }
    //     }
    //     return checkSum;
    // }

    //     public Integer call() {
    //         return restoreDiskDataSector(this.damagedDiskNumber, this.physicalDiskSector);
    //     }
    // }

    public int read(int sector) {
        if (isShutdown == false) {
            int physicalDiskNumber = sector / onePhysicalDiskSize;
            // zezwalaj na dostęp do dysku, tylko jeśli nie jest uszkodzony
            if (arePhysicalDisksDamaged.get(physicalDiskNumber) == false) {
                int physicalDiskSector = sector - physicalDiskNumber * onePhysicalDiskSize;
                try {
                    synchronized (logicalDisk.get(physicalDiskNumber)) {
                        return logicalDisk.get(physicalDiskNumber).read(physicalDiskSector);
                    }
                } catch (DiskInterface.DiskError e) {
                    System.out.println("DiskError dla dysku: " + physicalDiskNumber + ", sektora " + physicalDiskSector + ", proba odczytania wartosci ");
                    state = RAIDState.DEGRADED;
                    arePhysicalDisksDamaged.set(physicalDiskNumber, true);
                    final int damagedDiskNumber = physicalDiskNumber;
                    // odczytaj przy pomocy sumy kontrolnej
                    int checkSum = read(getSectorInLogicalSpace(logicalDisk.size() - 1, physicalDiskSector));
                    for (physicalDiskNumber = 0; physicalDiskNumber < logicalDisk.size() - 1; physicalDiskNumber++) {
                        if (physicalDiskNumber != damagedDiskNumber) {
                            checkSum -= read(getSectorInLogicalSpace(physicalDiskNumber, physicalDiskSector));
                        }
                    }
                    return checkSum;
                    // ExecutorService executor = Executors.newSingleThreadExecutor();
                    // Thread thread = new Thread(() -> restoreDiskDataSector(damagedDiskNumber, physicalDiskSector));
                }
            }
        }
        return -1;
    }

    public int size() {
        if (isShutdown == false) {
            return logicalDiskSize;
        }
        return -1;
    }

    public void shutdown() {
        isShutdown = true;
    }

    private void showSectors(ArrayList<Disk> physicalDisks) {
        // for (Disk physcialDisk : physicalDisks)
        //     physcialDisk.showValuesInDisk();
        System.out.println(
            physicalDisks.get(0).showValuesInDisk() + 
            physicalDisks.get(1).showValuesInDisk() + 
            physicalDisks.get(2).showValuesInDisk() + 
            physicalDisks.get(3).showValuesInDisk() + 
            physicalDisks.get(4).showValuesInDisk()
        );
    }

    public static void main(String[] args) {
        ArrayList<Disk> physicalDisks = new ArrayList<Disk>();
        for (int i = 0; i < 5; i++) {
            physicalDisks.add(new Disk());
        }
        RAID raid = new RAID();
        System.out.println("Po dodaniu dyskow do RAID:");
        raid.showSectors(physicalDisks);
        System.out.println("Stan dysku: " + raid.getState());
        for (Disk physicalDisk : physicalDisks) {
            raid.addDisk(physicalDisk);
        }
        raid.startRAID();
        System.out.println("Uruchomiono RAID, stan: " + raid.getState());
        System.out.println("Po uzupelnieniu sum kontrolnych:");
        raid.showSectors(physicalDisks);
        System.out.println("Stan: " + raid.getState());
        System.out.println("Rozmiar dysku logicznego: " + raid.size());
        System.out.println("Stan: " + raid.getState());
        Random random = new Random();

        class Reader implements Runnable {
            public void run() {
                // int sector = random.nextInt(20);
                int sector = 4;
                int readValue = raid.read(sector);
                System.out.println("Watek " + Thread.currentThread().getName() + " przeczytal wartosc z sektora "+ sector + ": " + readValue);
                raid.showSectors(physicalDisks);
            }
        }

        class Writer implements Runnable {
            public void run() {
                // int sector = random.nextInt(20);
                int value = random.nextInt(10);
                int sector = 4;
                raid.write(sector, value);
                System.out.println("Watek " + Thread.currentThread().getName() + " zapisal wartosc " + value + " do sektora "+ sector);
                raid.showSectors(physicalDisks);
            }
        }

        // new Reader().run();
        // for (int i = 0; i < 5; i++) {
        //     new Thread(new Reader()).start();;
        // }
        // new Writer().run();
        // for (int i = 0; i < 5; i++) {
        //     new Thread(new Writer()).start();;
        // }

        new Reader().run();
        new Writer().run();
        for (int i = 0; i < 5; i++) {
            new Thread(new Reader()).start();
            new Thread(new Writer()).start();
        }
    }
}

