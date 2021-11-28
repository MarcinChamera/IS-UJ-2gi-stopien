import java.util.ArrayList;

class RAID implements RAIDInterface {
    private ArrayList<DiskInterface> logicalDisk;
    private ArrayList<Boolean> arePhysicalDisksDamaged;
    private RAIDState state;
    private int logicalDiskSize;
    private int onePhysicalDiskSize;
    private boolean isShutdown;

    RAID() {
        logicalDisk = new ArrayList<DiskInterface>();
        arePhysicalDisksDamaged = new ArrayList<Boolean>();
        state = RAIDState.UNKNOWN;
        isShutdown = false;
    }

    // aby prawidlowo chronic zmienna stanowa za pomocy blokady trzeba wszystkie operacje
    // na niej zabezpieczac za pomoca tej samej blokady
    public synchronized RAIDState getState() {
        return state;
    }

    public void addDisk(DiskInterface disk) {
        if (isShutdown == false) {
            if (logicalDisk.isEmpty()) {
                onePhysicalDiskSize = disk.size();
            }
            // rozmiar dysku logicznego = rozmiar dysku fizycznego * (N - 1) dyskow fizycznych
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

            // zamiana dysku na nowy
            logicalDisk.set(damagedDiskNumber, disk);
            arePhysicalDisksDamaged.set(damagedDiskNumber, false);

            state = RAIDState.REBUILD;

            // POCZATEK ODBUDOWY MACIERZY
        
            // 1. jesli awarii ulegl ostatni dysk (kontrolny) - trzeba przeiterowac przez dyski od 0 do N - 1, dodajac wartosci z kazdego z sektorow, tak zeby
            // utworzyc nowe sumy kontrolne
            if (damagedDiskNumber == logicalDisk.size() - 1) {
                new Thread(() -> rebuildCheckSum()).start();
            }
            // 2. jesli awarii ulegl inny niz ostatni dysk - od sumy kontrolnej z ostatniego dysku trzeba odjac wartosci z sektorow innych niz ostatni i ten uszkodzony
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
            // zezwalaj na dostep do dysku, tylko jesli nie jest uszkodzony
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
                    state = RAIDState.DEGRADED;
                    arePhysicalDisksDamaged.set(physicalDiskNumber, true);
                    // obsluga awarii wadliwego dysku - zmiana stanu dysku z suma kontrolna (wez pod uwage zmiane pomiedzy obecna wartoscia z sektora uszkodzonego
                    // dysku, a nowa, dodawana wartoscia)
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

    public int read(int sector) {
        if (isShutdown == false) {
            int physicalDiskNumber = sector / onePhysicalDiskSize;
            // zezwalaj na dostep do dysku, tylko jesli nie jest uszkodzony
            if (arePhysicalDisksDamaged.get(physicalDiskNumber) == false) {
                int physicalDiskSector = sector - physicalDiskNumber * onePhysicalDiskSize;
                try {
                    synchronized (logicalDisk.get(physicalDiskNumber)) {
                        return logicalDisk.get(physicalDiskNumber).read(physicalDiskSector);
                    }
                } catch (DiskInterface.DiskError e) {
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
}